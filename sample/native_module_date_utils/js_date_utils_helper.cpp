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
    int32_t y, m, d;
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
    NAPI_ASSERT(env, argc >= DateArgCount::ONE, "requires 1 parameter: year");
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
    NAPI_ASSERT(env, argc >= DateArgCount::TWO,
        "requires 2 parameters: year, month");
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
    NAPI_ASSERT(env, argc >= DateArgCount::THREE,
        "requires 3 parameters: year, month, day");
    int year, month, day;
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
    NAPI_ASSERT(env, argc >= DateArgCount::THREE, "requires 3 parameters");
    int year, month, day;
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
    NAPI_ASSERT(env, argc >= DateArgCount::THREE, "requires 3 parameters");
    int year, month, day;
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
    NAPI_ASSERT(env, argc >= DateArgCount::THREE, "requires 3 parameters");
    int year, month, day;
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

static napi_value JSDaysBetween(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::SIX;
    napi_value argv[DateArgCount::SIX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= DateArgCount::SIX,
        "requires 6 parameters: y1, m1, d1, y2, m2, d2");
    int32_t y1, m1, d1, y2, m2, d2;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &y1));
    NAPI_CALL(env, napi_get_value_int32(env, argv[1], &m1));
    NAPI_CALL(env, napi_get_value_int32(env, argv[2], &d1));
    NAPI_CALL(env, napi_get_value_int32(env, argv[3], &y2));
    NAPI_CALL(env, napi_get_value_int32(env, argv[4], &m2));
    NAPI_CALL(env, napi_get_value_int32(env, argv[5], &d2));
    int result = DateUtils::DaysBetween(y1, m1, d1, y2, m2, d2);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_int32(env, result, &jsResult));
    return jsResult;
}

static napi_value JSAddDays(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::FOUR;
    napi_value argv[DateArgCount::FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= DateArgCount::FOUR,
        "requires 4 parameters: year, month, day, daysToAdd");
    int32_t year, month, day, daysToAdd;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &year));
    NAPI_CALL(env, napi_get_value_int32(env, argv[1], &month));
    NAPI_CALL(env, napi_get_value_int32(env, argv[2], &day));
    NAPI_CALL(env, napi_get_value_int32(env, argv[3], &daysToAdd));
    DateInfo result = DateUtils::AddDays(year, month, day, daysToAdd);
    napi_value jsResult = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsResult));
    napi_value yVal, mVal, dVal;
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
static napi_value JSFormatDate(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::FOUR;
    napi_value argv[DateArgCount::FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= DateArgCount::FOUR,
        "requires 4 parameters: year, month, day, format");
    int32_t year, month, day;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &year));
    NAPI_CALL(env, napi_get_value_int32(env, argv[1], &month));
    NAPI_CALL(env, napi_get_value_int32(env, argv[2], &day));
    char format[K_DATE_FORMAT_MAX_LEN] = { 0 };
    size_t formatLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[3], format,
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
    NAPI_ASSERT(env, argc >= DateArgCount::ONE, "requires 1 parameter: month");
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
    NAPI_ASSERT(env, argc >= DateArgCount::ONE, "requires 1 parameter: month");
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
static napi_value JSGetAge(napi_env env, napi_callback_info info)
{
    size_t argc = DateArgCount::THREE;
    napi_value argv[DateArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= DateArgCount::THREE,
        "requires 3 parameters: birthYear, birthMonth, birthDay");
    int32_t birthYear, birthMonth, birthDay;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &birthYear));
    NAPI_CALL(env, napi_get_value_int32(env, argv[1], &birthMonth));
    NAPI_CALL(env, napi_get_value_int32(env, argv[2], &birthDay));
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
    NAPI_ASSERT(env, argc >= DateArgCount::THREE, "requires 3 parameters");
    int year, month, day;
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
    NAPI_ASSERT(env, argc >= DateArgCount::THREE, "requires 3 parameters");
    int year, month, day;
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

int DateUtils::DaysBetween(int y1, int m1, int d1, int y2, int m2, int d2)
{
    int days1 = 0;
    for (int y = K_MIN_VALID_YEAR; y < y1; y++) {
        if (IsLeapYear(y)) {
            days1 += K_DAYS_PER_YEAR_LEAP;
        } else {
            days1 += K_DAYS_PER_YEAR_COMMON;
        }
    }
    for (int m = K_MIN_VALID_MONTH; m < m1; m++) {
        days1 += GetDaysInMonth(y1, m);
    }
    days1 += d1;
    int days2 = 0;
    for (int y = K_MIN_VALID_YEAR; y < y2; y++) {
        if (IsLeapYear(y)) {
            days2 += K_DAYS_PER_YEAR_LEAP;
        } else {
            days2 += K_DAYS_PER_YEAR_COMMON;
        }
    }
    for (int m = K_MIN_VALID_MONTH; m < m2; m++) {
        days2 += GetDaysInMonth(y2, m);
    }
    days2 += d2;
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
    snprintf(buffer, sizeof(buffer), format.c_str(),
        year, month, day, weekdayName, monthName);
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
    snprintf(buffer, sizeof(buffer), "Q%d", quarter);
    return std::string(buffer);
}

int DateUtils::GetAge(int birthYear, int birthMonth, int birthDay)
{
    int age = K_REFERENCE_YEAR - birthYear;
    if (K_REFERENCE_MONTH < birthMonth) {
        age--;
    }
    if (K_REFERENCE_MONTH == birthMonth && K_REFERENCE_DAY < birthDay) {
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

