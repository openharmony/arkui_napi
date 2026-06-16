/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstddef>
#include <cstdint>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

// ============================================================
// Named constants — no magic numbers
// ============================================================

// Type tag UUID halves for distinct test identities
static constexpr uint64_t TAG_A_LOWER = 0xABCD1234ABCD1234ULL;
static constexpr uint64_t TAG_A_UPPER = 0x5678FEDC5678FEDCULL;
static constexpr uint64_t TAG_B_LOWER = 0x1111222233334444ULL;
static constexpr uint64_t TAG_B_UPPER = 0x5555666677778888ULL;
static constexpr uint64_t TAG_ZERO = 0ULL;
static constexpr uint64_t TAG_ALL_BITS = 0xFFFFFFFFFFFFFFFFULL;

// Shared-lower / unique-upper constants for partial-overlap tests
static constexpr uint64_t TAG_SHARED_LOW = 0xDEADBEEFDEADBEEFULL;
static constexpr uint64_t TAG_UNIQUE_UP_C = 0xCAFECAFECAFECAFEULL;
static constexpr uint64_t TAG_UNIQUE_UP_D = 0xBEEFBEEFBEEFBEEFULL;

// Shared-upper / unique-lower constants for partial-overlap tests
static constexpr uint64_t TAG_SHARED_UP = 0x8BADF00D8BADF00DULL;
static constexpr uint64_t TAG_UNIQUE_LOW_E = 0xF00DF00DF00DF00DULL;
static constexpr uint64_t TAG_UNIQUE_LOW_F = 0xFEEDFEEDFEEDFEEDULL;

// Module registration
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t MODULE_FLAGS_NONE = 0;

// ============================================================
// Helper functions
// ============================================================

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// ============================================================
// Test 1: tag object and verify the same tag matches
// ============================================================
static napi_value TestTypeTagBasicRoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_type_tag tag = {TAG_A_LOWER, TAG_A_UPPER};
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tag));

    bool hasTag = false;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tag, &hasTag));
    SetNamedBool(env, result, "basicRoundTrip", hasTag);

    return result;
}

// ============================================================
// Test 2: tag with UUID-A, verify UUID-B does not match
// ============================================================
static napi_value TestTypeTagMismatch(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_type_tag tagA = {TAG_A_LOWER, TAG_A_UPPER};
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tagA));

    napi_type_tag tagB = {TAG_B_LOWER, TAG_B_UPPER};
    bool hasTagB = true;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tagB, &hasTagB));
    SetNamedBool(env, result, "mismatchRejected", !hasTagB);

    return result;
}

// ============================================================
// Test 3: two objects with different tags — verify isolation
// ============================================================
static napi_value TestTypeTagTwoObjects(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value objA = nullptr;
    napi_value objB = nullptr;
    NAPI_CALL(env, napi_create_object(env, &objA));
    NAPI_CALL(env, napi_create_object(env, &objB));

    napi_type_tag tagA = {TAG_A_LOWER, TAG_A_UPPER};
    napi_type_tag tagB = {TAG_B_LOWER, TAG_B_UPPER};
    NAPI_CALL(env, napi_type_tag_object(env, objA, &tagA));
    NAPI_CALL(env, napi_type_tag_object(env, objB, &tagB));

    bool aHasA = false;
    bool aHasB = true;
    bool bHasB = false;
    bool bHasA = true;
    NAPI_CALL(env, napi_check_object_type_tag(env, objA, &tagA, &aHasA));
    NAPI_CALL(env, napi_check_object_type_tag(env, objA, &tagB, &aHasB));
    NAPI_CALL(env, napi_check_object_type_tag(env, objB, &tagB, &bHasB));
    NAPI_CALL(env, napi_check_object_type_tag(env, objB, &tagA, &bHasA));

    SetNamedBool(env, result, "objAMatchesTagA", aHasA);
    SetNamedBool(env, result, "objARejectsTagB", !aHasB);
    SetNamedBool(env, result, "objBMatchesTagB", bHasB);
    SetNamedBool(env, result, "objBRejectsTagA", !bHasA);

    return result;
}

// ============================================================
// Test 4: tag object with all-zero UUID and verify match
// ============================================================
static napi_value TestTypeTagZeroTag(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_type_tag zeroTag = {TAG_ZERO, TAG_ZERO};
    NAPI_CALL(env, napi_type_tag_object(env, obj, &zeroTag));

    bool hasTag = false;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &zeroTag, &hasTag));
    SetNamedBool(env, result, "zeroTagMatch", hasTag);

    return result;
}

// ============================================================
// Test 5: tag object with all-bits-set UUID and verify match
// ============================================================
static napi_value TestTypeTagMaxBitsTag(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_type_tag maxTag = {TAG_ALL_BITS, TAG_ALL_BITS};
    NAPI_CALL(env, napi_type_tag_object(env, obj, &maxTag));

    bool hasTag = false;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &maxTag, &hasTag));
    SetNamedBool(env, result, "maxBitsTagMatch", hasTag);

    return result;
}

// ============================================================
// Test 6: overwrite an existing tag — old tag must not match
// ============================================================
static napi_value TestTypeTagOverwrite(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_type_tag tagA = {TAG_A_LOWER, TAG_A_UPPER};
    napi_type_tag tagB = {TAG_B_LOWER, TAG_B_UPPER};
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tagA));
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tagB));

    bool hasTagA = true;
    bool hasTagB = false;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tagA, &hasTagA));
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tagB, &hasTagB));

    SetNamedBool(env, result, "oldTagCleared", !hasTagA);
    SetNamedBool(env, result, "newTagActive", hasTagB);

    return result;
}

// ============================================================
// Test 7: check type tag on an object that was never tagged
// ============================================================
static napi_value TestTypeTagUntaggedObject(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_type_tag tag = {TAG_A_LOWER, TAG_A_UPPER};
    bool hasTag = true;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tag, &hasTag));
    SetNamedBool(env, result, "untaggedReturnsFalse", !hasTag);

    return result;
}

// ============================================================
// Test 8: tags with same lower, different upper are distinct
// ============================================================
static napi_value TestTypeTagSameLowerDiffUpper(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_type_tag tagC = {TAG_SHARED_LOW, TAG_UNIQUE_UP_C};
    napi_type_tag tagD = {TAG_SHARED_LOW, TAG_UNIQUE_UP_D};
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tagC));

    bool hasTagC = false;
    bool hasTagD = true;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tagC, &hasTagC));
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tagD, &hasTagD));

    SetNamedBool(env, result, "matchesOwnTag", hasTagC);
    SetNamedBool(env, result, "rejectsSimilarTag", !hasTagD);

    return result;
}

// ============================================================
// Test 9: tags with same upper, different lower are distinct
// ============================================================
static napi_value TestTypeTagSameUpperDiffLower(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_type_tag tagE = {TAG_UNIQUE_LOW_E, TAG_SHARED_UP};
    napi_type_tag tagF = {TAG_UNIQUE_LOW_F, TAG_SHARED_UP};
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tagE));

    bool hasTagE = false;
    bool hasTagF = true;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tagE, &hasTagE));
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tagF, &hasTagF));

    SetNamedBool(env, result, "matchesOwnTag", hasTagE);
    SetNamedBool(env, result, "rejectsSimilarTag", !hasTagF);

    return result;
}

// ============================================================
// Module export descriptors table
// ============================================================
static constexpr size_t TYPE_TAG_TEST_COUNT = 9;

struct TypeTagTestEntry {
    const char* name;
    napi_callback callback;
};

static const TypeTagTestEntry TYPE_TAG_TESTS[] = {
    {"testTypeTagBasicRoundTrip", TestTypeTagBasicRoundTrip},
    {"testTypeTagMismatch", TestTypeTagMismatch},
    {"testTypeTagTwoObjects", TestTypeTagTwoObjects},
    {"testTypeTagZeroTag", TestTypeTagZeroTag},
    {"testTypeTagMaxBitsTag", TestTypeTagMaxBitsTag},
    {"testTypeTagOverwrite", TestTypeTagOverwrite},
    {"testTypeTagUntaggedObject", TestTypeTagUntaggedObject},
    {"testTypeTagSameLowerDiffUpper", TestTypeTagSameLowerDiffUpper},
    {"testTypeTagSameUpperDiffLower", TestTypeTagSameUpperDiffLower},
};
}  // namespace

static napi_value InitTypeTagSuite(napi_env env, napi_value exports)
{
    std::vector<napi_property_descriptor> descriptors(TYPE_TAG_TEST_COUNT);
    for (size_t i = 0; i < TYPE_TAG_TEST_COUNT; i++) {
        descriptors[i] = napi_property_descriptor{
            TYPE_TAG_TESTS[i].name,
            nullptr,
            TYPE_TAG_TESTS[i].callback,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            nullptr,
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_typeTagSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = MODULE_FLAGS_NONE,
    .nm_filename = nullptr,
    .nm_register_func = InitTypeTagSuite,
    .nm_modname = "type_tag_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterTypeTagSuiteModule(void)
{
    napi_module_register(&g_typeTagSuiteModule);
}
