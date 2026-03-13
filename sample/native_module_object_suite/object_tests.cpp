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

#include <cstdint>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

struct ObjectCaseSpec {
    const char* name;
    int32_t delta;
    int32_t multiplier;
    int32_t threshold;
};

static const ObjectCaseSpec g_objectCaseSpecs[] = {
    {
        "objectCase01",
        -17,
        2,
        41,
    },
    {
        "objectCase02",
        -14,
        3,
        42,
    },
    {
        "objectCase03",
        -11,
        4,
        43,
    },
    {
        "objectCase04",
        -8,
        1,
        44,
    },
    {
        "objectCase05",
        -5,
        2,
        45,
    },
    {
        "objectCase06",
        -2,
        3,
        46,
    },
    {
        "objectCase07",
        1,
        4,
        47,
    },
    {
        "objectCase08",
        4,
        1,
        48,
    },
    {
        "objectCase09",
        7,
        2,
        49,
    },
    {
        "objectCase10",
        10,
        3,
        50,
    },
    {
        "objectCase11",
        13,
        4,
        51,
    },
    {
        "objectCase12",
        16,
        1,
        52,
    },
    {
        "objectCase13",
        19,
        2,
        53,
    },
    {
        "objectCase14",
        22,
        3,
        54,
    },
    {
        "objectCase15",
        25,
        4,
        55,
    },
    {
        "objectCase16",
        28,
        1,
        56,
    },
    {
        "objectCase17",
        31,
        2,
        57,
    },
    {
        "objectCase18",
        34,
        3,
        58,
    },
    {
        "objectCase19",
        37,
        4,
        59,
    },
};

bool ReadUtf8(napi_env env, napi_value value, const char* message, std::string* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_string) {
        napi_throw_type_error(env, nullptr, message);
        return false;
    }

    size_t length = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &length) != napi_ok) {
        return false;
    }
    std::string buffer(length + 1, '\0');
    if (napi_get_value_string_utf8(env, value, buffer.data(), buffer.size(), &length) != napi_ok) {
        return false;
    }
    buffer.resize(length);
    *result = buffer;
    return true;
}

bool ReadNamedString(napi_env env, napi_value object, const char* name, std::string* result)
{
    napi_value property = nullptr;
    bool hasProperty = false;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok || !hasProperty) {
        napi_throw_error(env, nullptr, "required string property is missing");
        return false;
    }
    if (napi_get_named_property(env, object, name, &property) != napi_ok) {
        return false;
    }
    return ReadUtf8(env, property, "property must be a string", result);
}

bool ReadNamedInt32(napi_env env, napi_value object, const char* name, int32_t* result)
{
    napi_value property = nullptr;
    bool hasProperty = false;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok || !hasProperty) {
        napi_throw_error(env, nullptr, "required number property is missing");
        return false;
    }
    if (napi_get_named_property(env, object, name, &property) != napi_ok) {
        return false;
    }
    if (napi_get_value_int32(env, property, result) != napi_ok) {
        napi_throw_type_error(env, nullptr, "property must be a number");
        return false;
    }
    return true;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(), &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateObjectSummary(
    napi_env env, const char* name, const std::string& label, int32_t actual, int32_t expected, bool elevated)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", std::string(name));
    SetNamedString(env, result, "label", label);
    SetNamedInt32(env, result, "actual", actual);
    SetNamedInt32(env, result, "expected", expected);
    SetNamedBool(env, result, "elevated", elevated);
    SetNamedBool(env, result, "passed", actual == expected);
    return result;
}

static napi_value TestObjectCase01(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[0];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase02(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[1];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase03(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[2];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase04(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[3];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase05(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[4];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase06(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[5];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase07(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[6];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase08(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[7];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase09(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[8];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase10(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[9];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase11(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[10];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase12(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[11];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase13(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[12];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase14(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[13];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase15(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[14];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase16(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[15];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase17(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[16];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase18(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[17];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

static napi_value TestObjectCase19(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string label;
    int32_t score = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto& spec = g_objectCaseSpecs[18];
    int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

}  // namespace

static napi_value InitBranch04Object(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("testObjectCase01", TestObjectCase01),
        DECLARE_NAPI_FUNCTION("testObjectCase02", TestObjectCase02),
        DECLARE_NAPI_FUNCTION("testObjectCase03", TestObjectCase03),
        DECLARE_NAPI_FUNCTION("testObjectCase04", TestObjectCase04),
        DECLARE_NAPI_FUNCTION("testObjectCase05", TestObjectCase05),
        DECLARE_NAPI_FUNCTION("testObjectCase06", TestObjectCase06),
        DECLARE_NAPI_FUNCTION("testObjectCase07", TestObjectCase07),
        DECLARE_NAPI_FUNCTION("testObjectCase08", TestObjectCase08),
        DECLARE_NAPI_FUNCTION("testObjectCase09", TestObjectCase09),
        DECLARE_NAPI_FUNCTION("testObjectCase10", TestObjectCase10),
        DECLARE_NAPI_FUNCTION("testObjectCase11", TestObjectCase11),
        DECLARE_NAPI_FUNCTION("testObjectCase12", TestObjectCase12),
        DECLARE_NAPI_FUNCTION("testObjectCase13", TestObjectCase13),
        DECLARE_NAPI_FUNCTION("testObjectCase14", TestObjectCase14),
        DECLARE_NAPI_FUNCTION("testObjectCase15", TestObjectCase15),
        DECLARE_NAPI_FUNCTION("testObjectCase16", TestObjectCase16),
        DECLARE_NAPI_FUNCTION("testObjectCase17", TestObjectCase17),
        DECLARE_NAPI_FUNCTION("testObjectCase18", TestObjectCase18),
        DECLARE_NAPI_FUNCTION("testObjectCase19", TestObjectCase19),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch04ObjectModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch04Object,
    .nm_modname = "object_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch04ObjectModule(void)
{
    napi_module_register(&g_branch04ObjectModule);
}
