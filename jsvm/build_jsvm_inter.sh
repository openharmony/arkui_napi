#!/bin/bash
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

do_fetch() {
    echo "skip."
}

do_patch() {
    echo "skip."
}

do_configure() {
    pushd ${workdir}
    if [[ "${TARGET_CPU}" = "x86_64" ]]; then
        ./configure --shared; return
    fi
    ./configure \
        --prefix=${workdir} \
        --dest-cpu=${TARGET_CPU} --dest-os=linux \
        --cross-compiling \
        --shared \
        --with-arm-float-abi=hard \
        --without-corepack \
        --without-npm \
        --without-intl
    popd
}

do_unstripped_copy() {
    mkdir -p ${TARGET_GEN_DIR}/../../../../../lib.unstripped/jsvm/
    cp -u ${workdir}/out/Release/libjsvm.so ${TARGET_GEN_DIR}/../../../../../lib.unstripped/jsvm/
    cp -u ${workdir}/deps/v8/lib.unstripped/libv8_shared.so ${TARGET_GEN_DIR}/../../../../../lib.unstripped/jsvm/
    pushd ${out_dir}
    rm -rf *
    popd
}

get_thread_num() {
    quota_us_file="/sys/fs/cgroup/cpu/cpu.cfs_quota_us"
    period_us_file="/sys/fs/cgroup/cpu/cpu.cfs_period_us"
    if [ -f "${quota_us_file}" ]; then
        cfs_quota_us=$(cat ${quota_us_file})
    fi
    if [ -f "${period_us_file}" ]; then
        cfs_period_us=$(cat ${period_us_file})
    fi
    # Set the default value when the variable is empty.
    cfs_quota_us=${cfs_quota_us:=-1}
    cfs_period_us=${cfs_period_us:=0}
    if [ "${cfs_quota_us}" != -1 -a "${cfs_period_us}" != 0 ]; then
        PROCESSORS=$(expr ${cfs_quota_us} / ${cfs_period_us})
        echo "cpu.cfs_quota_us: "$PROCESSORS
    else
        PROCESSORS=$(cat /proc/cpuinfo | grep "processor" | wc -l)
        echo "cpuinfo: "$PROCESSORS
    fi
}

do_compile() {
    pushd ${workdir}
    get_thread_num
    cpu_num=$[PROCESSORS*2]
    make -j${cpu_num}
    popd
}

do_strip() {
    stripped_binary_path=${TARGET_GEN_DIR}/libjsvm.so
    binary=${stripped_binary_path}
    echo "${binary}"
    dynsyms_path="${stripped_binary_path}.dynsyms"
    funcsysms_path="${stripped_binary_path}.funcsyms"
    keep_path="${stripped_binary_path}.keep"
    debug_path="${stripped_binary_path}.debug"
    mini_debug_path="${stripped_binary_path}.minidebug"

    ${NM} -D ${binary} --format=posix --defined-only \
            | awk '{ print $1 }' | sort > ${dynsyms_path}
    ${NM} ${binary} --format=posix --defined-only \
            | awk '{ if ($2 == "T" || $2 == "t" || $2 == "D") print $1 }' \
            | sort > ${funcsysms_path}
    comm -13 ${dynsyms_path} ${funcsysms_path} > ${keep_path}

    ${OBJCOPY} --only-keep-debug ${binary} ${debug_path}
    ${OBJCOPY} -S --remove-section .gdb_index --remove-section .comment \
            --keep-symbols=${keep_path} ${debug_path} ${mini_debug_path}

    ${STRIP} --strip-all --keep-section=.comment ${binary}

    xz ${mini_debug_path}
    ${OBJCOPY} --add-section .gnu_debugdata=${mini_debug_path}.xz ${binary}

    rm -f ${dynsyms_path}
    rm -f ${funcsysms_path}
    rm -f ${keep_path}
    rm -f ${debug_path}
    rm -f ${mini_debug_path}
    rm -f ${mini_debug_path}.xz
}

do_install () {
    cp -u ${workdir}/out/Release/libjsvm.so ${TARGET_GEN_DIR}
}

do_env() {
    # init workspace
    out_dir=${TARGET_GEN_DIR}/out
    workdir=${NODE_PATH}
    [ -d "${out_dir}" ] || mkdir -p ${out_dir}
    [ -L "${workdir}/out" ] || ln -s ${out_dir} ${workdir}/out

    argurment+=" -fstack-protector-strong"
    argurment+=" -Wl,-z,noexecstack"
    argurment+=" -Wl,-z,relro"
    argurment+=" -Wl,-z,now"
    argurment+=" -pie"
    if [ $1 -eq 1 ]; then
        argurment+=" -ggdb3"
    fi
    if [[ "${TARGET_CPU}" = "arm" ]]; then
        cflags="  --target=arm-linux-ohos"
        cflags+=" --sysroot=${SYSROOT}"
        cflags+=" -march=armv7-a"
        cflags+=" -mfpu=neon"
        cflags_host="-m32"
        ARCH="arm"
    elif [[ "${TARGET_CPU}" = "arm64" ]]; then
        cflags="  --target=aarch64-linux-ohos"
        cflags+=" --sysroot=${SYSROOT}"
        cflags+=" -march=armv8-a"
        cflags+=" -DV8_OS_OH=1"
        cflags+=" -mfpu=neon"
        cflags_host="-m64"
        ARCH="aarch64"
    elif [[ "${TARGET_CPU}" = "x86_64" ]]; then
        export CC="${CCACHE_EXEC} gcc"
        export CXX="${CCACHE_EXEC} g++"
        return
    else
        die "not support target cpu"
    fi

    if [[ "${TARGET_CLANG_COVERAGE}" = "true" ]]; then
        cflags+=" --coverage"
    fi

    cflags+=" ${argurment}"

    # linux host env
    HOST_OS="linux"
    HOST_ARCH="x86_64"
    export LINK_host="${CCACHE_EXEC} ${PREFIX}/clang++ ${cflags_host}"
    export CXX_host="${CCACHE_EXEC} ${PREFIX}/clang++ ${cflags_host}"
    export CC_host="${CCACHE_EXEC} ${PREFIX}/clang ${cflags_host}"
    export AR_host=${PREFIX}/llvm-ar

    # target env
    export CC="${CCACHE_EXEC} ${PREFIX}/clang ${cflags}"
    export CXX="${CCACHE_EXEC} ${PREFIX}/clang++ ${cflags}"
    export LD="${PREFIX}/ld.lld"
    export AS="${PREFIX}/llvm-as"
    export AR="${PREFIX}/llvm-ar"
    export STRIP="${PREFIX}/llvm-strip"
    export OBJCOPY="${PREFIX}/llvm-objcopy"
    export OBJDUMP="${PREFIX}/llvm-obidump"
    export RANLIB="${PREFIX}/llvm-ranlib"
    export NM="${PREFIX}/llvm-nm"
    export STRINGS="${PREFIX}/llvm-strings"
    export READELF="${PREFIX}/llvm-readelf"
    env > ${out_dir}/log.do_env
}
