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
    if [[ "${TARGET_CPU}" = "x64" ]]; then
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
        --without-ssl \
        --without-intl
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

do_install () {
    # default install path
    cp -u ${workdir}/out/Release/libjsvm.so ${out_dir}/lib/libjsvm.so
    # target install path
    if [ "x${TARGET_GEN_DIR}" != "x" ]; then
        cp -u ${out_dir}/lib/* ${TARGET_GEN_DIR}
    fi
}

do_env() {
    # init workspace
    out_dir=${SCRIPT_PATCH}/out
    workdir=${NODE_PATH}
    mkdir -p ${out_dir}
    mkdir -p ${out_dir}/include
    mkdir -p ${out_dir}/lib

    if [[ "${TARGET_CPU}" = "arm" ]]; then
        target_name="arm-linux-ohos"
        target_arch="arm"
        target_march="armv7-a"
        host_args="-m32 -I${workdir}"
    elif [[ "${TARGET_CPU}" = "arm64" ]]; then
        target_name="aarch64-linux-ohos"
        target_arch="aarch64"
        target_march="armv8-a"
        host_args="-m64"
    else
        return # do nothing
    fi

    argurment+=" -s"
    argurment+=" -fstack-protector-strong"
    argurment+=" -Wl,-z,noexecstack"
    argurment+=" -Wl,-z,relro"
    argurment+=" -Wl,-z,now"
    argurment+=" -pie"

    cflags_host=${host_args}
    cflags="  --target=${target_name}"
    cflags+=" --sysroot=${SYSROOT}"
    cflags+=" -mfpu=neon"
    cflags+=" -march=${target_march}"
    cflags+=" ${argurment}"

    # linux host env
    HOST_OS="linux"
    HOST_ARCH="x86_64"
    export LINK_host="${CCACHE_EXEC} ${PREFIX}/clang++ ${cflags_host}"
    export CXX_host="${CCACHE_EXEC} ${PREFIX}/clang++ ${cflags_host}"
    export CC_host="${CCACHE_EXEC} ${PREFIX}/clang ${cflags_host}"
    export AR_host=${PREFIX}/llvm-ar

    # target env
    ARCH=${target_arch}
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
    env | tee ${out_dir}/log.do_env
}
