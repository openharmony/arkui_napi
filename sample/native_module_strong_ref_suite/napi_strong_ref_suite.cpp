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
#include <cstring>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

// ============================================================
// Named constants — no magic numbers
// ============================================================

// Integer test values
static constexpr int32_t INT32_VALUE = 42;
static constexpr int32_t INT32_NEGATIVE = -99;
static constexpr int32_t INT32_ZERO = 0;

// String test values
static constexpr char STRING_HELLO[] = "hello";
static constexpr char STRING_EMPTY[] = "";
static constexpr char STRING_WORLD[] = "world";
static constexpr size_t STRING_BUFFER_SIZE = 256;

// Reference count constants
static constexpr uint32_t REF_COUNT_ONE = 1;
static constexpr uint32_t REF_COUNT_ZERO = 0;
static constexpr uint32_t REF_COUNT_TWO = 2;

// Property names for object round-trip tests
static constexpr char PROP_NAME[] = "value";
static constexpr char PROP_KEY[] = "key";

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
// Test 1: strong reference to int32 — create, get, verify
// ============================================================
static napi_value TestStrongRefInt32RoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value original = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_VALUE, &original));

    napi_strong_ref ref = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, original, &ref));

    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_strong_reference_value(env, ref, &retrieved));

    int32_t got = INT32_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, retrieved, &got));
    if (!SetNamedBool(env, result, "int32Match", got == INT32_VALUE)) {
        return nullptr;
    }

    NAPI_CALL(env, napi_delete_strong_reference(env, ref));
    return result;
}

// ============================================================
// Test 2: strong reference to utf8 string — create, get, verify
// ============================================================
static napi_value TestStrongRefStringRoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value original = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, STRING_HELLO, NAPI_AUTO_LENGTH, &original));

    napi_strong_ref ref = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, original, &ref));

    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_strong_reference_value(env, ref, &retrieved));

    char buf[STRING_BUFFER_SIZE] = {0};
    size_t len = REF_COUNT_ZERO;
    NAPI_CALL(env, napi_get_value_string_utf8(env, retrieved, buf, sizeof(buf), &len));
    if (!SetNamedBool(env, result, "stringMatch", strcmp(buf, STRING_HELLO) == INT32_ZERO)) {
        return nullptr;
    }

    NAPI_CALL(env, napi_delete_strong_reference(env, ref));
    return result;
}

// ============================================================
// Test 3: strong reference to boolean — create, get, verify
// ============================================================
static napi_value TestStrongRefBooleanRoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value original = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &original));

    napi_strong_ref ref = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, original, &ref));

    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_strong_reference_value(env, ref, &retrieved));

    bool got = false;
    NAPI_CALL(env, napi_get_value_bool(env, retrieved, &got));
    if (!SetNamedBool(env, result, "booleanMatch", got)) {
        return nullptr;
    }

    NAPI_CALL(env, napi_delete_strong_reference(env, ref));
    return result;
}

// ============================================================
// Test 4: strong reference to object — verify property access through ref
// ============================================================
static napi_value TestStrongRefObjectRoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value original = nullptr;
    NAPI_CALL(env, napi_create_object(env, &original));
    napi_value propVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, STRING_WORLD, NAPI_AUTO_LENGTH, &propVal));
    NAPI_CALL(env, napi_set_named_property(env, original, PROP_NAME, propVal));

    napi_strong_ref ref = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, original, &ref));

    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_strong_reference_value(env, ref, &retrieved));
    napi_value gotProp = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, retrieved, PROP_NAME, &gotProp));

    char buf[STRING_BUFFER_SIZE] = {0};
    size_t len = REF_COUNT_ZERO;
    NAPI_CALL(env, napi_get_value_string_utf8(env, gotProp, buf, sizeof(buf), &len));
    if (!SetNamedBool(env, result, "propertyMatch", strcmp(buf, STRING_WORLD) == INT32_ZERO)) {
        return nullptr;
    }

    NAPI_CALL(env, napi_delete_strong_reference(env, ref));
    return result;
}

// ============================================================
// Test 5: two strong references to the same object — both resolve correctly
// ============================================================
static napi_value TestStrongRefMultipleRefs(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value original = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_NEGATIVE, &original));

    napi_strong_ref refA = nullptr;
    napi_strong_ref refB = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, original, &refA));
    NAPI_CALL(env, napi_create_strong_reference(env, original, &refB));

    napi_value valA = nullptr;
    napi_value valB = nullptr;
    NAPI_CALL(env, napi_get_strong_reference_value(env, refA, &valA));
    NAPI_CALL(env, napi_get_strong_reference_value(env, refB, &valB));

    int32_t gotA = INT32_ZERO;
    int32_t gotB = INT32_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, valA, &gotA));
    NAPI_CALL(env, napi_get_value_int32(env, valB, &gotB));

    if (!SetNamedBool(env, result, "refAMatch", gotA == INT32_NEGATIVE)) {
        return nullptr;
    }
    if (!SetNamedBool(env, result, "refBMatch", gotB == INT32_NEGATIVE)) {
        return nullptr;
    }

    NAPI_CALL(env, napi_delete_strong_reference(env, refA));
    NAPI_CALL(env, napi_delete_strong_reference(env, refB));
    return result;
}

// ============================================================
// Test 6: delete strong reference returns napi_ok
// ============================================================
static napi_value TestStrongRefDelete(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value original = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_VALUE, &original));

    napi_strong_ref ref = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, original, &ref));

    napi_status status = napi_delete_strong_reference(env, ref);
    if (!SetNamedBool(env, result, "deleteOk", status == napi_ok)) {
        return nullptr;
    }

    return result;
}

// ============================================================
// Test 7: strong references to different types — verify independence
// ============================================================
static napi_value TestStrongRefDifferentTypes(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Integer ref
    napi_value intVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_VALUE, &intVal));
    napi_strong_ref intRef = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, intVal, &intRef));

    // String ref
    napi_value strVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, STRING_HELLO, NAPI_AUTO_LENGTH, &strVal));
    napi_strong_ref strRef = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, strVal, &strRef));

    // Verify int ref
    napi_value gotInt = nullptr;
    NAPI_CALL(env, napi_get_strong_reference_value(env, intRef, &gotInt));
    int32_t intResult = INT32_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, gotInt, &intResult));
    if (!SetNamedBool(env, result, "intRefMatch", intResult == INT32_VALUE)) {
        return nullptr;
    }

    // Verify string ref
    napi_value gotStr = nullptr;
    NAPI_CALL(env, napi_get_strong_reference_value(env, strRef, &gotStr));
    char buf[STRING_BUFFER_SIZE] = {0};
    size_t len = REF_COUNT_ZERO;
    NAPI_CALL(env, napi_get_value_string_utf8(env, gotStr, buf, sizeof(buf), &len));
    if (!SetNamedBool(env, result, "strRefMatch", strcmp(buf, STRING_HELLO) == INT32_ZERO)) {
        return nullptr;
    }

    NAPI_CALL(env, napi_delete_strong_reference(env, intRef));
    NAPI_CALL(env, napi_delete_strong_reference(env, strRef));
    return result;
}

// ============================================================
// Test 8: delete and recreate — reuse variable for a new ref
// ============================================================
static napi_value TestStrongRefDeleteAndRecreate(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value valA = nullptr;
    napi_value valB = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, STRING_HELLO, NAPI_AUTO_LENGTH, &valA));
    NAPI_CALL(env, napi_create_string_utf8(env, STRING_WORLD, NAPI_AUTO_LENGTH, &valB));

    napi_strong_ref ref = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, valA, &ref));

    napi_value first = nullptr;
    NAPI_CALL(env, napi_get_strong_reference_value(env, ref, &first));
    char bufFirst[STRING_BUFFER_SIZE] = {0};
    size_t lenFirst = REF_COUNT_ZERO;
    NAPI_CALL(env, napi_get_value_string_utf8(env, first, bufFirst, sizeof(bufFirst), &lenFirst));
    if (!SetNamedBool(env, result, "firstMatch", strcmp(bufFirst, STRING_HELLO) == INT32_ZERO)) {
        return nullptr;
    }

    NAPI_CALL(env, napi_delete_strong_reference(env, ref));

    // Reuse the same variable for a new ref to a different value
    ref = nullptr;
    NAPI_CALL(env, napi_create_strong_reference(env, valB, &ref));

    napi_value second = nullptr;
    NAPI_CALL(env, napi_get_strong_reference_value(env, ref, &second));
    char bufSecond[STRING_BUFFER_SIZE] = {0};
    size_t lenSecond = REF_COUNT_ZERO;
    NAPI_CALL(env, napi_get_value_string_utf8(env, second, bufSecond, sizeof(bufSecond), &lenSecond));
    if (!SetNamedBool(env, result, "secondMatch", strcmp(bufSecond, STRING_WORLD) == INT32_ZERO)) {
        return nullptr;
    }

    NAPI_CALL(env, napi_delete_strong_reference(env, ref));
    return result;
}

// ============================================================
// Module export descriptors table
// ============================================================
static constexpr size_t STRONG_REF_TEST_COUNT = 8;

struct StrongRefTestEntry {
    const char* name;
    napi_callback callback;
};

static const StrongRefTestEntry STRONG_REF_TESTS[] = {
    {"testStrongRefInt32RoundTrip", TestStrongRefInt32RoundTrip},
    {"testStrongRefStringRoundTrip", TestStrongRefStringRoundTrip},
    {"testStrongRefBooleanRoundTrip", TestStrongRefBooleanRoundTrip},
    {"testStrongRefObjectRoundTrip", TestStrongRefObjectRoundTrip},
    {"testStrongRefMultipleRefs", TestStrongRefMultipleRefs},
    {"testStrongRefDelete", TestStrongRefDelete},
    {"testStrongRefDifferentTypes", TestStrongRefDifferentTypes},
    {"testStrongRefDeleteAndRecreate", TestStrongRefDeleteAndRecreate},
};
}  // namespace

static napi_value InitStrongRefSuite(napi_env env, napi_value exports)
{
    std::vector<napi_property_descriptor> descriptors(STRONG_REF_TEST_COUNT);
    for (size_t i = 0; i < STRONG_REF_TEST_COUNT; i++) {
        descriptors[i] = napi_property_descriptor{
            STRONG_REF_TESTS[i].name,
            nullptr,
            STRONG_REF_TESTS[i].callback,
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

static napi_module g_strongRefSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = MODULE_FLAGS_NONE,
    .nm_filename = nullptr,
    .nm_register_func = InitStrongRefSuite,
    .nm_modname = "strong_ref_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterStrongRefSuiteModule(void)
{
    napi_module_register(&g_strongRefSuiteModule);
}
