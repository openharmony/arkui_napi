#!/usr/bin/env python3
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

"""
Runs checkapi on the host: sets up LD_LIBRARY_PATH from the out dir, lays .so files out
under <workdir>/module/ following the module manager search rules, generates the checklist
json and invokes checkapi in multi-file mode, forwarding its exit code. <so module name>
accepts a comma-separated list for cross-module regression cases.
"""

import json
import os
import shutil
import subprocess
import sys


def print_usage():
    print("Usage: run_check.py <checkapi> <root_out_dir> <kit> "
          "<so module name[,more]> <legal json> <result json>", file=sys.stderr)


def module_dir_and_so(module_name):
    # Mirrors NativeModuleManager::GetNativeModulePath (LINUX): dots become directories and
    # the last segment names the .so ("a.b" -> module/a/libb.so).
    m = module_name.lower()
    parts = m.split(".")
    if len(parts) == 1:
        return "module", "lib%s.so" % m
    return os.path.join("module", *parts[:-1]), "lib%s.so" % parts[-1]


def module_require_name(module_name):
    # Mirrors the C++ ModuleNameForRequireNapi mapping. Known gap: the NATIVE_MODULE special
    # cases are not mirrored (host samples and regular @ohos.<name> modules are unaffected).
    if module_name.startswith("@ohos.") or module_name.startswith("@system."):
        stripped = module_name[1:]
        if "." in stripped:
            return stripped.split(".", 1)[1]
        return stripped
    if module_name.startswith("@"):
        return module_name[1:]
    return module_name


def find_built_so(root, module_name):
    m = module_name.lower()
    last = m.split(".")[-1]
    candidates = ["lib%s.so" % last, "lib%s_napi.so" % last]
    for base in ["arkui/napi", "."]:
        for cand in candidates:
            path = os.path.join(root, base, cand)
            if os.path.isfile(path):
                return path
    return None


def build_run_env(root):
    """Builds the run environment with LD_LIBRARY_PATH matching the out dir layout."""
    env = os.environ.copy()
    lib_parts = [
        "arkui/napi",
        "arkcompiler/ets_runtime",
        "thirdparty/cJSON",
        "thirdparty/libuv",
        "thirdparty/bounds_checking_function",
        "thirdparty/icu",
    ]
    lib_paths = [os.path.join(root, p) for p in lib_parts]
    src_root = os.path.dirname(os.path.dirname(root))
    clang_lib = os.path.join(src_root, "prebuilts", "clang", "ohos", "linux-x86_64", "llvm", "lib")
    if os.path.isdir(clang_lib):
        lib_paths.append(clang_lib)
    env["LD_LIBRARY_PATH"] = ":".join(lib_paths)
    return env


def main():
    if len(sys.argv) != 7:
        print_usage()
        return 2

    api_check, root, kit, check_file, legal_json, result_json = sys.argv[1:]
    api_check = os.path.abspath(api_check)
    root = os.path.abspath(root)
    legal_json = os.path.abspath(legal_json)
    result_json = os.path.abspath(result_json)

    print("run start...")

    env = build_run_env(root)
    print("LD_LIBRARY_PATH=" + env["LD_LIBRARY_PATH"])

    # The .so must be reachable as ./module/... relative to cwd (LINUX_PLATFORM search prefix);
    # supports a comma-separated module list for cross-module regression cases.
    check_files = [f for f in check_file.split(",") if f]
    workdir = os.path.join(root, "checkapi", "run", "so",
                           "_".join(module_require_name(f).replace(".", "_") for f in check_files))
    for check in check_files:
        module_name = module_require_name(check)
        so_dir, so_name = module_dir_and_so(module_name)
        target_so_dir = os.path.join(workdir, so_dir)
        os.makedirs(target_so_dir, exist_ok=True)
        built_so = find_built_so(root, module_name)
        if built_so is None:
            print("run_check.py: can not find built so for module %s" % module_name, file=sys.stderr)
            return 2
        shutil.copy2(built_so, os.path.join(target_so_dir, so_name))
        print("copied %s -> %s" % (built_so, os.path.join(target_so_dir, so_name)))
    os.makedirs(workdir, exist_ok=True)

    checklist = os.path.join(workdir, "checklist.json")
    with open(checklist, "w") as f:
        json.dump({kit: check_files}, f)

    cmd = [api_check, checklist, os.path.dirname(legal_json), result_json]
    print("run: " + " ".join(cmd) + " (cwd: %s)" % workdir)
    proc = subprocess.run(cmd, env=env, cwd=workdir, check=False)
    print("run end. exit code: %d" % proc.returncode)
    return proc.returncode


if __name__ == "__main__":
    sys.exit(main())
