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

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

struct DateCaseSpec {
    const char* name;
    int32_t shiftHours;
    int32_t bucket;
};

static const DateCaseSpec g_dateCaseSpecs[] = {
    {
        "dateCase01",
        3,
        1,
    },
    {
        "dateCase02",
        6,
        2,
    },
    {
        "dateCase03",
        9,
        3,
    },
    {
        "dateCase04",
        12,
        4,
    },
    {
        "dateCase05",
        15,
        5,
    },
    {
        "dateCase06",
        18,
        0,
    },
    {
        "dateCase07",
        21,
        1,
    },
    {
        "dateCase08",
        24,
        2,
    },
    {
        "dateCase09",
        27,
        3,
    },
    {
        "dateCase10",
        30,
        4,
    },
    {
        "dateCase11",
        33,
        5,
    },
    {
        "dateCase12",
        36,
        0,
    },
    {
        "dateCase13",
        39,
        1,
    },
    {
        "dateCase14",
        42,
        2,
    },
    {
        "dateCase15",
        45,
        3,
    },
    {
        "dateCase16",
        48,
        4,
    },
    {
        "dateCase17",
        51,
        5,
    },
    {
        "dateCase18",
        54,
        0,
    },
    {
        "dateCase19",
        57,
        1,
    },
    {
        "dateCase20",
        60,
        2,
    },
    {
        "dateCase21",
        63,
        3,
    },
    {
        "dateCase22",
        66,
        4,
    },
    {
        "dateCase23",
        69,
        5,
    },
    {
        "dateCase24",
        72,
        0,
    },
    {
        "dateCase25",
        75,
        1,
    },
    {
        "dateCase26",
        78,
        2,
    },
    {
        "dateCase27",
        81,
        3,
    },
    {
        "dateCase28",
        84,
        4,
    },
};

bool ReadDate(napi_env env, napi_value value, double* result)
{
    if (napi_get_date_value(env, value, result) != napi_ok) {
        napi_throw_type_error(env, nullptr, "value must be a Date");
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

bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value)
{
    napi_value napiValue = nullptr;
    if (napi_create_double(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateDateSummary(
    napi_env env, const char* name, double originalMs, double shiftedMs, int32_t bucket, bool matchesBucket)
{
    napi_value result = nullptr;
    napi_value shiftedDate = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_date(env, shiftedMs, &shiftedDate));
    SetNamedString(env, result, "name", name);
    SetNamedDouble(env, result, "originalMs", originalMs);
    SetNamedDouble(env, result, "shiftedMs", shiftedMs);
    SetNamedInt32(env, result, "bucket", bucket);
    SetNamedBool(env, result, "matchesBucket", matchesBucket);
    NAPI_CALL(env, napi_set_named_property(env, result, "shiftedDate", shiftedDate));
    return result;
}

static napi_value TestDateCase01(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[0];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase02(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[1];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase03(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[2];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase04(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[3];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase05(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[4];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase06(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[5];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase07(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[6];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase08(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[7];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase09(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[8];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase10(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[9];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase11(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[10];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase12(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[11];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase13(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[12];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase14(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[13];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase15(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[14];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase16(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[15];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase17(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[16];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase18(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[17];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase19(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[18];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase20(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[19];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase21(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[20];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase22(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[21];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase23(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[22];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase24(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[23];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase25(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[24];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase26(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[25];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase27(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[26];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

static napi_value TestDateCase28(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    double originalMs = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "date is required");
        return nullptr;
    }
    if (!ReadDate(env, args[0], &originalMs)) {
        return nullptr;
    }

    const auto& spec = g_dateCaseSpecs[27];
    double shiftedMs = originalMs + static_cast<double>(spec.shiftHours) * 60.0 * 60.0 * 1000.0;
    int32_t bucket = static_cast<int32_t>(shiftedMs / (60.0 * 60.0 * 1000.0)) % 24;
    bool matchesBucket = bucket % 6 == spec.bucket;

    return CreateDateSummary(env, spec.name, originalMs, shiftedMs, bucket, matchesBucket);
}

}  // namespace

static napi_value InitBranch06Date(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("testDateCase01", TestDateCase01),
        DECLARE_NAPI_FUNCTION("testDateCase02", TestDateCase02),
        DECLARE_NAPI_FUNCTION("testDateCase03", TestDateCase03),
        DECLARE_NAPI_FUNCTION("testDateCase04", TestDateCase04),
        DECLARE_NAPI_FUNCTION("testDateCase05", TestDateCase05),
        DECLARE_NAPI_FUNCTION("testDateCase06", TestDateCase06),
        DECLARE_NAPI_FUNCTION("testDateCase07", TestDateCase07),
        DECLARE_NAPI_FUNCTION("testDateCase08", TestDateCase08),
        DECLARE_NAPI_FUNCTION("testDateCase09", TestDateCase09),
        DECLARE_NAPI_FUNCTION("testDateCase10", TestDateCase10),
        DECLARE_NAPI_FUNCTION("testDateCase11", TestDateCase11),
        DECLARE_NAPI_FUNCTION("testDateCase12", TestDateCase12),
        DECLARE_NAPI_FUNCTION("testDateCase13", TestDateCase13),
        DECLARE_NAPI_FUNCTION("testDateCase14", TestDateCase14),
        DECLARE_NAPI_FUNCTION("testDateCase15", TestDateCase15),
        DECLARE_NAPI_FUNCTION("testDateCase16", TestDateCase16),
        DECLARE_NAPI_FUNCTION("testDateCase17", TestDateCase17),
        DECLARE_NAPI_FUNCTION("testDateCase18", TestDateCase18),
        DECLARE_NAPI_FUNCTION("testDateCase19", TestDateCase19),
        DECLARE_NAPI_FUNCTION("testDateCase20", TestDateCase20),
        DECLARE_NAPI_FUNCTION("testDateCase21", TestDateCase21),
        DECLARE_NAPI_FUNCTION("testDateCase22", TestDateCase22),
        DECLARE_NAPI_FUNCTION("testDateCase23", TestDateCase23),
        DECLARE_NAPI_FUNCTION("testDateCase24", TestDateCase24),
        DECLARE_NAPI_FUNCTION("testDateCase25", TestDateCase25),
        DECLARE_NAPI_FUNCTION("testDateCase26", TestDateCase26),
        DECLARE_NAPI_FUNCTION("testDateCase27", TestDateCase27),
        DECLARE_NAPI_FUNCTION("testDateCase28", TestDateCase28),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch06DateModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch06Date,
    .nm_modname = "date_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch06DateModule(void)
{
    napi_module_register(&g_branch06DateModule);
}
