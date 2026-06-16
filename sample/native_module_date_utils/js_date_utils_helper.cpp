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

#include "js_date_utils_helper.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr size_t K_DATE_STR_MAX_LEN = 64;
constexpr size_t K_DATE_FORMAT_MAX_LEN = 128;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;
constexpr int K_INVALID_VALUE = -1;
constexpr int K_MIN_VALID_YEAR = 1;
constexpr int K_MIN_VALID_MONTH = 1;
constexpr int K_MIN_VALID_DAY = 1;
constexpr int K_MONTHS_PER_YEAR = 12;
constexpr int K_DAYS_PER_WEEK = 7;
constexpr int K_FEB_MONTH = 2;
constexpr int K_FEB_DAYS_LEAP = 29;
constexpr int K_LEAP_YEAR_DIVISOR = 4;
constexpr int K_CENTURY_DIVISOR = 100;
constexpr int K_QUAD_CENTURY_DIVISOR = 400;
constexpr int K_MONTH_INDEX_OFFSET = 1;
constexpr int K_MONTH_THRESHOLD = 2;
constexpr int K_MONTHS_PER_SEASON = 3;
constexpr int K_MAX_WEEKS_IN_YEAR = 52;
constexpr int K_JANUARY = 1;
constexpr int K_DAYS_PER_YEAR_COMMON = 365;
constexpr int K_DAYS_PER_YEAR_LEAP = 366;
constexpr int K_WEEKDAY_SUNDAY = 0;
constexpr int K_WEEKDAY_SATURDAY = 6;
constexpr int K_WEEKDAY_FORMULA_MONTH_OFFSET = 26;
constexpr int K_WEEKDAY_FORMULA_DIVISOR = 10;
constexpr int K_WEEKDAY_FORMULA_QUARTER = 4;
constexpr int K_WEEKDAY_FORMULA_MULTIPLIER = 5;
constexpr int K_WEEKDAY_FORMULA_MOD = 7;
constexpr int K_WEEKDAY_OFFSET = 6;
constexpr int K_REFERENCE_YEAR = 2024;
constexpr int K_REFERENCE_MONTH = 6;
constexpr int K_REFERENCE_DAY = 15;

constexpr std::array<int, K_MONTHS_PER_YEAR + 1> K_DAYS_IN_MONTHS = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

constexpr std::array<const char*, K_DAYS_PER_WEEK> K_WEEKDAY_NAMES = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};

constexpr std::array<const char*, K_MONTHS_PER_YEAR + 1> K_MONTH_NAMES = {
    "", "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

constexpr std::array<const char*, 4> K_SEASON_NAMES = {
    "Winter", "Spring", "Summer", "Autumn"
};

enum class DateArgIndex {
    FIRST = 0,
    SECOND = 1,
    THIRD = 2,
};

namespace DateArgCount {
    constexpr size_t ONE = 1;
    constexpr size_t TWO = 2;
    constexpr size_t THREE = 3;
    constexpr size_t FOUR = 4;
    constexpr size_t SIX = 6;
};

bool ExtractDateFromArgs(
    napi_env env, napi_value* args, int& year, int& month, int& day)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[static_cast<size_t>(
        DateArgIndex::FIRST)], &type));
    if (type != napi_number) {
        return false;
    }
    NAPI_CALL(env, napi_typeof(env, args[static_cast<size_t>(
        DateArgIndex::SECOND)], &type));
    if (type != napi_number) {
        return false;
    }
    NAPI_CALL(env, napi_typeof(env, args[static_cast<size_t>(
        DateArgIndex::THIRD)], &type));
    if (type != napi_number) {
        return false;
    }
    int32_t y;
    int32_t m;
    int32_t d;
    NAPI_CALL(env, napi_get_value_int32(env, args[static_cast<size_t>(
        DateArgIndex::FIRST)], &y));
    NAPI_CALL(env, napi_get_value_int32(env, args[static_cast<size_t>(
        DateArgIndex::SECOND)], &m));
    NAPI_CALL(env, napi_get_value_int32(env, args[static_cast<size_t>(
        DateArgIndex::THIRD)], &d));
    year = y;
    month = m;
    day = d;
    return true;
}

/***********************************************
 * NAPI Functions - Date Validation
 ***********************************************/
static napi_value JSIsLeapYear(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::ONE;
    napi_value argv[DateArgCount::ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::ONE) {
        napi_throw_type_error(env, nullptr, "requires 1 parameter: year");
        return nullptr;
    }
    int32_t year;
    NAPI_CALL(env, napi_get_value_int32(env,
        argv[static_cast<size_t>(DateArgIndex::FIRST)], &year));
    bool result = DateUtils::IsLeapYear(year);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &jsResult));
    return jsResult;
}

static napi_value JSGetDaysInMonth(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::TWO;
    napi_value argv[DateArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::TWO) {
        napi_throw_type_error(env, nullptr, "requires 2 parameters: year, month");
        return nullptr;
    }
    int32_t year;
    int32_t month;
    NAPI_CALL(env, napi_get_value_int32(env,
        argv[static_cast<size_t>(DateArgIndex::FIRST)], &year));
    NAPI_CALL(env, napi_get_value_int32(env,
        argv[static_cast<size_t>(DateArgIndex::SECOND)], &month));
    int result = DateUtils::GetDaysInMonth(year, month);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_int32(env, result, &jsResult));
    return jsResult;
}

static napi_value JSIsValidDate(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::THREE;
    napi_value argv[DateArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::THREE) {
        napi_throw_type_error(env, nullptr,
            "requires 3 parameters: year, month, day");
        return nullptr;
    }
    int year;
    int month;
    int day;
    if (!ExtractDateFromArgs(env, argv, year, month, day)) {
        napi_value jsFalse = nullptr;
        NAPI_CALL(env, napi_get_boolean(env, false, &jsFalse));
        return jsFalse;
    }
    bool result = DateUtils::IsValidDate(year, month, day);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &jsResult));
    return jsResult;
}

/***********************************************
 * NAPI Functions - Date Calculation
 ***********************************************/
static napi_value JSGetDayOfYear(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::THREE;
    napi_value argv[DateArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::THREE) {
        napi_throw_type_error(env, nullptr, "requires 3 parameters");
        return nullptr;
    }
    int year;
    int month;
    int day;
    if (!ExtractDateFromArgs(env, argv, year, month, day)) {
        return nullptr;
    }
    int result = DateUtils::GetDayOfYear(year, month, day);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_int32(env, result, &jsResult));
    return jsResult;
}

static napi_value JSGetWeekday(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::THREE;
    napi_value argv[DateArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::THREE) {
        napi_throw_type_error(env, nullptr, "requires 3 parameters");
        return nullptr;
    }
    int year;
    int month;
    int day;
    if (!ExtractDateFromArgs(env, argv, year, month, day)) {
        return nullptr;
    }
    int weekday = DateUtils::GetWeekday(year, month, day);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_int32(env, weekday, &jsResult));
    return jsResult;
}

static napi_value JSGetWeekdayName(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::THREE;
    napi_value argv[DateArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::THREE) {
        napi_throw_type_error(env, nullptr, "requires 3 parameters");
        return nullptr;
    }
    int year;
    int month;
    int day;
    if (!ExtractDateFromArgs(env, argv, year, month, day)) {
        return nullptr;
    }
    int weekday = DateUtils::GetWeekday(year, month, day);
    const char* name = K_WEEKDAY_NAMES[weekday];
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH,
        &jsResult));
    return jsResult;
}

namespace DateArgIndex6 {
    constexpr size_t YEAR1 = 0;
    constexpr size_t MONTH1 = 1;
    constexpr size_t DAY1 = 2;
    constexpr size_t YEAR2 = 3;
    constexpr size_t MONTH2 = 4;
    constexpr size_t DAY2 = 5;
};

static napi_value JSDaysBetween(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::SIX;
    napi_value argv[DateArgCount::SIX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::SIX) {
        napi_throw_type_error(env, nullptr,
            "requires 6 parameters: y1, m1, d1, y2, m2, d2");
        return nullptr;
    }
    int32_t y1;
    int32_t m1;
    int32_t d1;
    int32_t y2;
    int32_t m2;
    int32_t d2;
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex6::YEAR1], &y1));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex6::MONTH1], &m1));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex6::DAY1], &d1));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex6::YEAR2], &y2));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex6::MONTH2], &m2));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex6::DAY2], &d2));
    DateInfo d1(y1, m1, d1);
    DateInfo d2(y2, m2, d2);
    int result = DateUtils::DaysBetween(d1, d2);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_int32(env, result, &jsResult));
    return jsResult;
}

namespace DateArgIndex4 {
    constexpr size_t YEAR = 0;
    constexpr size_t MONTH = 1;
    constexpr size_t DAY = 2;
    constexpr size_t DAYS_TO_ADD = 3;
};

static napi_value JSAddDays(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::FOUR;
    napi_value argv[DateArgCount::FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::FOUR) {
        napi_throw_type_error(env, nullptr,
            "requires 4 parameters: year, month, day, daysToAdd");
        return nullptr;
    }
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t daysToAdd;
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex4::YEAR], &year));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex4::MONTH], &month));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex4::DAY], &day));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex4::DAYS_TO_ADD],
        &daysToAdd));
    DateInfo result = DateUtils::AddDays(year, month, day, daysToAdd);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsResult));
    napi_value yVal;
    napi_value mVal;
    napi_value dVal;
    NAPI_CALL(env, napi_create_int32(env, result.year, &yVal));
    NAPI_CALL(env, napi_create_int32(env, result.month, &mVal));
    NAPI_CALL(env, napi_create_int32(env, result.day, &dVal));
    NAPI_CALL(env, napi_set_named_property(env, jsResult, "year", yVal));
    NAPI_CALL(env, napi_set_named_property(env, jsResult, "month", mVal));
    NAPI_CALL(env, napi_set_named_property(env, jsResult, "day", dVal));
    return jsResult;
}

/***********************************************
 * NAPI Functions - Date Formatting
 ***********************************************/
namespace DateArgIndex4Fmt {
    constexpr size_t YEAR = 0;
    constexpr size_t MONTH = 1;
    constexpr size_t DAY = 2;
    constexpr size_t FORMAT = 3;
};

static napi_value JSFormatDate(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::FOUR;
    napi_value argv[DateArgCount::FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::FOUR) {
        napi_throw_type_error(env, nullptr,
            "requires 4 parameters: year, month, day, format");
        return nullptr;
    }
    int32_t year;
    int32_t month;
    int32_t day;
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex4Fmt::YEAR],
        &year));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex4Fmt::MONTH],
        &month));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex4Fmt::DAY], &day));
    char format[K_DATE_FORMAT_MAX_LEN] = { 0 };
    size_t formatLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env,
        argv[DateArgIndex4Fmt::FORMAT], format,
        sizeof(format), &formatLen));
    std::string result = DateUtils::FormatDate(year, month, day,
        std::string(format, formatLen));
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(),
        result.length(), &jsResult));
    return jsResult;
}

static napi_value JSGetSeason(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::ONE;
    napi_value argv[DateArgCount::ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::ONE) {
        napi_throw_type_error(env, nullptr, "requires 1 parameter: month");
        return nullptr;
    }
    int32_t month;
    NAPI_CALL(env, napi_get_value_int32(env,
        argv[static_cast<size_t>(DateArgIndex::FIRST)], &month));
    std::string result = DateUtils::GetSeason(month);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(),
        result.length(), &jsResult));
    return jsResult;
}

static napi_value JSGetQuarter(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::ONE;
    napi_value argv[DateArgCount::ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::ONE) {
        napi_throw_type_error(env, nullptr, "requires 1 parameter: month");
        return nullptr;
    }
    int32_t month;
    NAPI_CALL(env, napi_get_value_int32(env,
        argv[static_cast<size_t>(DateArgIndex::FIRST)], &month));
    std::string result = DateUtils::GetQuarter(month);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(),
        result.length(), &jsResult));
    return jsResult;
}

/***********************************************
 * NAPI Functions - Date Utilities
 ***********************************************/
namespace DateArgIndex3Age {
    constexpr size_t BIRTH_YEAR = 0;
    constexpr size_t BIRTH_MONTH = 1;
    constexpr size_t BIRTH_DAY = 2;
};

static napi_value JSGetAge(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::THREE;
    napi_value argv[DateArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::THREE) {
        napi_throw_type_error(env, nullptr,
            "requires 3 parameters: birthYear, birthMonth, birthDay");
        return nullptr;
    }
    int32_t birthYear;
    int32_t birthMonth;
    int32_t birthDay;
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex3Age::BIRTH_YEAR],
        &birthYear));
    NAPI_CALL(env, napi_get_value_int32(env,
        argv[DateArgIndex3Age::BIRTH_MONTH], &birthMonth));
    NAPI_CALL(env, napi_get_value_int32(env, argv[DateArgIndex3Age::BIRTH_DAY],
        &birthDay));
    int result = DateUtils::GetAge(birthYear, birthMonth, birthDay);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_int32(env, result, &jsResult));
    return jsResult;
}

static napi_value JSIsWeekend(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::THREE;
    napi_value argv[DateArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::THREE) {
        napi_throw_type_error(env, nullptr, "requires 3 parameters");
        return nullptr;
    }
    int year;
    int month;
    int day;
    if (!ExtractDateFromArgs(env, argv, year, month, day)) {
        napi_value jsFalse = nullptr;
        NAPI_CALL(env, napi_get_boolean(env, false, &jsFalse));
        return jsFalse;
    }
    bool result = DateUtils::IsWeekend(year, month, day);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &jsResult));
    return jsResult;
}

static napi_value JSGetWeekNumber(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::THREE;
    napi_value argv[DateArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < DateArgCount::THREE) {
        napi_throw_type_error(env, nullptr, "requires 3 parameters");
        return nullptr;
    }
    int year;
    int month;
    int day;
    if (!ExtractDateFromArgs(env, argv, year, month, day)) {
        return nullptr;
    }
    int result = DateUtils::GetWeekNumber(year, month, day);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_int32(env, result, &jsResult));
    return jsResult;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value DateUtilsExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("isLeapYear", JSIsLeapYear),
        DECLARE_NAPI_FUNCTION("getDaysInMonth", JSGetDaysInMonth),
        DECLARE_NAPI_FUNCTION("isValidDate", JSIsValidDate),
        DECLARE_NAPI_FUNCTION("getDayOfYear", JSGetDayOfYear),
        DECLARE_NAPI_FUNCTION("getWeekday", JSGetWeekday),
        DECLARE_NAPI_FUNCTION("getWeekdayName", JSGetWeekdayName),
        DECLARE_NAPI_FUNCTION("daysBetween", JSDaysBetween),
        DECLARE_NAPI_FUNCTION("addDays", JSAddDays),
        DECLARE_NAPI_FUNCTION("formatDate", JSFormatDate),
        DECLARE_NAPI_FUNCTION("getSeason", JSGetSeason),
        DECLARE_NAPI_FUNCTION("getQuarter", JSGetQuarter),
        DECLARE_NAPI_FUNCTION("getAge", JSGetAge),
        DECLARE_NAPI_FUNCTION("isWeekend", JSIsWeekend),
        DECLARE_NAPI_FUNCTION("getWeekNumber", JSGetWeekNumber),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
        sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_dateUtilsModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = DateUtilsExport,
    .nm_modname = "date_utils",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void DateUtilsRegister()
{
    napi_module_register(&g_dateUtilsModule);
}
}  // namespace

bool DateUtils::IsLeapYear(int year)
{
    if (year % K_QUAD_CENTURY_DIVISOR == 0) {
        return true;
    }
    if (year % K_CENTURY_DIVISOR == 0) {
        return false;
    }
    return year % K_LEAP_YEAR_DIVISOR == 0;
}

int DateUtils::GetDaysInMonth(int year, int month)
{
    if (month < K_MIN_VALID_MONTH || month > K_MONTHS_PER_YEAR) {
        return 0;
    }
    if (month == K_FEB_MONTH && IsLeapYear(year)) {
        return K_FEB_DAYS_LEAP;
    }
    return K_DAYS_IN_MONTHS[month];
}

bool DateUtils::IsValidDate(int year, int month, int day)
{
    if (year < K_MIN_VALID_YEAR || month < K_MIN_VALID_MONTH) {
        return false;
    }
    if (month > K_MONTHS_PER_YEAR) {
        return false;
    }
    if (day < K_MIN_VALID_DAY) {
        return false;
    }
    return day <= GetDaysInMonth(year, month);
}

int DateUtils::GetDayOfYear(int year, int month, int day)
{
    int dayOfYear = 0;
    for (int m = K_MIN_VALID_MONTH; m < month; m++) {
        dayOfYear += GetDaysInMonth(year, m);
    }
    dayOfYear += day;
    return dayOfYear;
}

int DateUtils::GetWeekday(int year, int month, int day)
{
    if (month <= K_MONTH_THRESHOLD) {
        month += K_MONTHS_PER_YEAR;
        year--;
    }
    int k = year % K_CENTURY_DIVISOR;
    int j = year / K_CENTURY_DIVISOR;
    int h = (day + K_WEEKDAY_FORMULA_MONTH_OFFSET *
        (month + K_MONTH_INDEX_OFFSET) /
        K_WEEKDAY_FORMULA_DIVISOR + k +
        k / K_WEEKDAY_FORMULA_QUARTER +
        j / K_WEEKDAY_FORMULA_QUARTER +
        K_WEEKDAY_FORMULA_MULTIPLIER * j) %
        K_WEEKDAY_FORMULA_MOD;
    return (h + K_WEEKDAY_OFFSET) % K_DAYS_PER_WEEK;
}

int DateUtils::DaysBetween(const DateInfo& date1, const DateInfo& date2)
{
    int days1 = 0;
    for (int y = K_MIN_VALID_YEAR; y < date1.year; y++) {
        if (IsLeapYear(y)) {
            days1 += K_DAYS_PER_YEAR_LEAP;
        } else {
            days1 += K_DAYS_PER_YEAR_COMMON;
        }
    }
    for (int m = K_MIN_VALID_MONTH; m < date1.month; m++) {
        days1 += GetDaysInMonth(date1.year, m);
    }
    days1 += date1.day;
    int days2 = 0;
    for (int y = K_MIN_VALID_YEAR; y < date2.year; y++) {
        if (IsLeapYear(y)) {
            days2 += K_DAYS_PER_YEAR_LEAP;
        } else {
            days2 += K_DAYS_PER_YEAR_COMMON;
        }
    }
    for (int m = K_MIN_VALID_MONTH; m < date2.month; m++) {
        days2 += GetDaysInMonth(date2.year, m);
    }
    days2 += date2.day;
    return days2 - days1;
}

DateInfo DateUtils::AddDays(int year, int month, int day, int daysToAdd)
{
    int targetDay = day + daysToAdd;
    int maxDay = GetDaysInMonth(year, month);
    while (targetDay > maxDay) {
        targetDay -= maxDay;
        month++;
        if (month > K_MONTHS_PER_YEAR) {
            month = K_MIN_VALID_MONTH;
            year++;
        }
        maxDay = GetDaysInMonth(year, month);
    }
    while (targetDay < K_MIN_VALID_DAY) {
        month--;
        if (month < K_MIN_VALID_MONTH) {
            month = K_MONTHS_PER_YEAR;
            year--;
        }
        targetDay += GetDaysInMonth(year, month);
    }
    return DateInfo(year, month, targetDay);
}

std::string DateUtils::FormatDate(int year, int month, int day,
    const std::string& format)
{
    char buffer[K_DATE_FORMAT_MAX_LEN] = { 0 };
    int weekday = GetWeekday(year, month, day);
    const char* weekdayName = K_WEEKDAY_NAMES[weekday];
    const char* monthName = K_MONTH_NAMES[month];
    int ret = snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
        format.c_str(), year, month, day, weekdayName, monthName);
    if (ret < 0) {
        return "";
    }
    return std::string(buffer);
}

std::string DateUtils::GetSeason(int month)
{
    if (month < K_MIN_VALID_MONTH || month > K_MONTHS_PER_YEAR) {
        return "Invalid";
    }
    int seasonIndex = (month - K_MONTH_INDEX_OFFSET) / K_MONTHS_PER_SEASON;
    return K_SEASON_NAMES[seasonIndex];
}

std::string DateUtils::GetQuarter(int month)
{
    if (month < K_MIN_VALID_MONTH || month > K_MONTHS_PER_YEAR) {
        return "Invalid";
    }
    int quarter = (month - K_MONTH_INDEX_OFFSET) /
        K_MONTHS_PER_SEASON + K_MONTH_INDEX_OFFSET;
    char buffer[K_DATE_STR_MAX_LEN] = { 0 };
    int ret = snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
        "Q%d", quarter);
    if (ret < 0) {
        return "";
    }
    return std::string(buffer);
}

int DateUtils::GetAge(int birthYear, int birthMonth, int birthDay)
{
    int age = K_REFERENCE_YEAR - birthYear;
    if (birthMonth > K_REFERENCE_MONTH) {
        age--;
    }
    if (birthMonth == K_REFERENCE_MONTH && birthDay > K_REFERENCE_DAY) {
        age--;
    }
    return age;
}

bool DateUtils::IsWeekend(int year, int month, int day)
{
    int weekday = GetWeekday(year, month, day);
    return weekday == K_WEEKDAY_SUNDAY ||
           weekday == K_WEEKDAY_SATURDAY;
}

int DateUtils::GetWeekNumber(int year, int month, int day)
{
    int dayOfYear = GetDayOfYear(year, month, day);
    int jan1Weekday = GetWeekday(year, K_JANUARY, K_JANUARY);
    int weekNum = (dayOfYear + jan1Weekday - K_MONTH_INDEX_OFFSET) /
        K_DAYS_PER_WEEK + K_MONTH_INDEX_OFFSET;
    if (weekNum > K_MAX_WEEKS_IN_YEAR) {
        weekNum = K_MAX_WEEKS_IN_YEAR;
    }
    return weekNum;
}
