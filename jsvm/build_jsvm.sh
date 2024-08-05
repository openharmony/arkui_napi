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

declare SCRIPT_PATCH
declare SYSROOT
declare PREFIX
declare TARGET_CPU
declare TARGET_GEN_DIR

export SCRIPT_PATCH=$(dirname $(readlink -f "$0"))

source ${SCRIPT_PATCH}/build_jsvm_inter.sh

options="$(getopt -o h "help,sysroot:,node_path:,prefix:,target_cpu:,base_path:" -- "$@")" || usage

eval set -- "$options"

usage() {
    echo "Tool $(basename "$0") Usage"
    echo "Options:"
    echo "-h|--help"
    echo "--sysroot <path>            Sysroot path."
    echo "--prefix <perfix>           Cross-compiler prefix."
    echo "--target_cpu <arm/arm64>    Cross-compile CPU types."
    echo "--target_gen_dir <output path>"
    echo "          if target_gen_dir not set, will install only to the default path."
    exit 0
}

die() {
    echo $@
    exit 0
}

do_man_process() {
    do_opt_process $@
    do_env 0
    do_fetch     > ${out_dir}/log.do_fetch
    do_patch     > ${out_dir}/log.do_patch
    do_configure > ${out_dir}/log.do_configure
    do_compile   > ${out_dir}/log.do_compile
    do_install   > ${out_dir}/log.do_install
    do_strip
    do_env 1
    do_fetch     > ${out_dir}/log.do_fetch_unstripped
    do_patch     > ${out_dir}/log.do_patch_unstripped
    do_configure > ${out_dir}/log.do_configure_unstripped
    do_compile   > ${out_dir}/log.do_compile_unstripped
    do_unstripped_copy
}

do_opt_process() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
        -h|--help)
            usage
            ;;
        --sysroot)
            export SYSROOT=$2
            shift
            ;;
        --node_path)
            export NODE_PATH=$2
            shift
            ;;
        --prefix)
            export PREFIX=$2
            shift
            ;;
        --target_cpu)
            export TARGET_CPU=$2
            shift
            ;;
        --target_gen_dir)
            export TARGET_GEN_DIR=$2
            shift
            ;;
        --target_clang_coverage)
            export TARGET_CLANG_COVERAGE=$2
            shift
            ;;
        *)
            ;;
        esac
        shift
    done
}

do_man_process $@
