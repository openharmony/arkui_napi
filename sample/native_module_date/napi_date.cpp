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

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include "native_api.h"

namespace {

constexpr int SECONDS_PER_MINUTE = 60;
constexpr int SECONDS_PER_HOUR = 3600;
constexpr int HOURS_PER_DAY = 24;
constexpr int DAYS_PER_WEEK = 7;
constexpr int DAYS_PER_YEAR = 365;
constexpr int MONTHS_PER_YEAR = 12;
constexpr int DAYS_IN_FEBRUARY = 28;
constexpr int DAYS_IN_FEBRUARY_LEAP = 29;
constexpr int BASE_YEAR = 1900;
constexpr int QUARTER_DIVISOR = 3;
constexpr int DAYS_IN_MONTH_DEFAULT = 30;
constexpr int SECONDS_PER_DAY = 86400;
constexpr int DIVISOR_400 = 400;
constexpr int DIVISOR_100 = 100;
constexpr int DIVISOR_4 = 4;
constexpr int TWO_DIGITS = 2;
constexpr int THREE_CHARS = 3;
constexpr int THREE_ARGS = 3;
constexpr int NUMBER_0 = 0;
constexpr int NUMBER_1 = 1;
constexpr int NUMBER_2 = 2;
constexpr int NUMBER_3 = 3;
constexpr int SATURDAY = 6;

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

const std::map<std::string, int> TIMEZONE_OFFSETS = {
    {"UTC", 0},
    {"GMT", 0},
    {"CST", 8},
    {"EST", -5},
    {"PST", -8},
    {"MST", -7},
    {"CST", -6},
    {"AST", -4},
    {"HST", -10}
};

const std::vector<std::string> WEEKDAYS = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};

const std::vector<std::string> MONTHS = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

std::time_t ToTimeT(const TimePoint& tp)
{
    return std::chrono::system_clock::to_time_t(tp);
}

TimePoint FromTimeT(std::time_t tt)
{
    return std::chrono::system_clock::from_time_t(tt);
}

TimePoint Now()
{
    return std::chrono::system_clock::now();
}

bool IsLeapYear(int year)
{
    if (year % DIVISOR_4 != 0) {
        return false;
    }
    if (year % DIVISOR_100 != 0) {
        return true;
    }
    return year % DIVISOR_400 == 0;
}

int GetDaysInMonth(int year, int month)
{
    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 1 && IsLeapYear(year)) {
        return DAYS_IN_FEBRUARY_LEAP;
    }
    return daysInMonth[month];
}

void ProcessFormatSpecifier(std::stringstream& ss, const std::string& format, size_t& i, const std::tm& localTm);

std::string FormatDateTime(const TimePoint& tp, const std::string& format)
{
    std::time_t tt = ToTimeT(tp);
    std::tm localTm = {};
    std::tm* tm = std::localtime(&tt);
    if (tm != nullptr) {
        localTm = *tm;
    }
    std::stringstream ss;

    for (size_t i = 0; i < format.size(); ++i) {
        if (format[i] == '%') {
            if (i + 1 < format.size()) {
                int idx = i;
                ProcessFormatSpecifier(ss, format, idx, localTm);
            } else {
                ss << format[i];
            }
        } else {
            ss << format[i];
        }
    }

    return ss.str();
}

void ProcessFormatSpecifier(std::stringstream& ss, const std::string& format, size_t& i, const std::tm& localTm)
{
    switch (format[i + 1]) {
        case 'm':
            ss << std::setw(TWO_DIGITS) << std::setfill('0') << (localTm.tm_mon + 1);
            break;
        case 'd':
            ss << std::setw(TWO_DIGITS) << std::setfill('0') << localTm.tm_mday;
            break;
        case 'H':
            ss << std::setw(TWO_DIGITS) << std::setfill('0') << localTm.tm_hour;
            break;
        case 'M':
            ss << std::setw(TWO_DIGITS) << std::setfill('0') << localTm.tm_min;
            break;
        case 'S':
            ss << std::setw(TWO_DIGITS) << std::setfill('0') << localTm.tm_sec;
            break;
        case 'w':
            ss << localTm.tm_wday;
            break;
        case 'a':
            ss << WEEKDAYS[localTm.tm_wday].substr(0, THREE_CHARS);
            break;
        case 'A':
            ss << WEEKDAYS[localTm.tm_wday];
            break;
        case 'b':
            ss << MONTHS[localTm.tm_mon].substr(0, THREE_CHARS);
            break;
        case 'B':
            ss << MONTHS[localTm.tm_mon];
            break;
        default:
            ss << format[i] << format[i + 1];
            break;
    }
    ++i;
}

double DateDiffDays(const TimePoint& start, const TimePoint& end)
{
    auto duration = end - start;
    return std::chrono::duration<double, std::ratio<SECONDS_PER_DAY>>(duration).count();
}

TimePoint AddDays(const TimePoint& tp, int days)
{
    return tp + std::chrono::hours(HOURS_PER_DAY * days);
}

TimePoint AddMonths(const TimePoint& tp, int monthsToAdd)
{
    std::time_t tt = ToTimeT(tp);
    std::tm localTm = {};
    std::tm* tm = std::localtime(&tt);
    if (tm != nullptr) {
        localTm = *tm;
    }

    int year = localTm.tm_year + BASE_YEAR;
    int month = localTm.tm_mon + 1;
    int day = localTm.tm_mday;

    month += monthsToAdd;
    while (month > MONTHS_PER_YEAR) {
        month -= MONTHS_PER_YEAR;
        year++;
    }
    while (month < 1) {
        month += MONTHS_PER_YEAR;
        year--;
    }

    int daysInCurrentMonth = GetDaysInMonth(year, month - 1);
    if (day > daysInCurrentMonth) {
        day = daysInCurrentMonth;
    }

    localTm.tm_year = year - BASE_YEAR;
    localTm.tm_mon = month - 1;
    localTm.tm_mday = day;

    return FromTimeT(std::mktime(&localTm));
}

TimePoint AddYears(const TimePoint& tp, int years)
{
    std::time_t tt = ToTimeT(tp);
    std::tm localTm = {};
    std::tm* tm = std::localtime(&tt);
    if (tm != nullptr) {
        localTm = *tm;
    }

    int year = localTm.tm_year + BASE_YEAR + years;
    int month = localTm.tm_mon;
    int day = localTm.tm_mday;

    int daysInMonth = GetDaysInMonth(year, month);
    if (day > daysInMonth) {
        day = daysInMonth;
    }

    localTm.tm_year = year - BASE_YEAR;
    localTm.tm_mday = day;

    return FromTimeT(std::mktime(&localTm));
}

int GetDayOfWeek(const TimePoint& tp)
{
    std::time_t tt = ToTimeT(tp);
    std::tm localTm = {};
    std::tm* tm = std::localtime(&tt);
    if (tm != nullptr) {
        localTm = *tm;
    }
    return localTm.tm_wday;
}

int GetQuarter(const TimePoint& tp)
{
    std::time_t tt = ToTimeT(tp);
    std::tm localTm = {};
    std::tm* tm = std::localtime(&tt);
    if (tm != nullptr) {
        localTm = *tm;
    }
    return (localTm.tm_mon / QUARTER_DIVISOR) + 1;
}

int GetWeekOfYear(const TimePoint& tp)
{
    std::time_t tt = ToTimeT(tp);
    std::tm localTm = {};
    std::tm* tm = std::localtime(&tt);
    if (tm != nullptr) {
        localTm = *tm;
    }
    return localTm.tm_yday / DAYS_PER_WEEK + 1;
}

bool IsSameDay(const TimePoint& tp1, const TimePoint& tp2)
{
    std::time_t tt1 = ToTimeT(tp1);
    std::time_t tt2 = ToTimeT(tp2);
    std::tm localTm1 = {};
    std::tm localTm2 = {};
    std::tm* tm1 = std::localtime(&tt1);
    std::tm* tm2 = std::localtime(&tt2);
    if (tm1 != nullptr) {
        localTm1 = *tm1;
    }
    if (tm2 != nullptr) {
        localTm2 = *tm2;
    }

    return localTm1.tm_year == localTm2.tm_year &&
           localTm1.tm_mon == localTm2.tm_mon &&
           localTm1.tm_mday == localTm2.tm_mday;
}

bool IsInRange(const TimePoint& tp, const TimePoint& start, const TimePoint& end)
{
    return tp >= start && tp <= end;
}

std::string GetRelativeTime(const TimePoint& tp)
{
    TimePoint nowTp = Now();
    auto duration = nowTp - tp;

    double seconds = std::chrono::duration<double>(duration).count();
    if (seconds < SECONDS_PER_MINUTE) {
        return "just now";
    }

    double minutes = seconds / SECONDS_PER_MINUTE;
    if (minutes < SECONDS_PER_HOUR / SECONDS_PER_MINUTE) {
        return std::to_string(static_cast<int>(minutes)) + " minutes ago";
    }

    double hours = minutes / SECONDS_PER_MINUTE;
    if (hours < HOURS_PER_DAY) {
        return std::to_string(static_cast<int>(hours)) + " hours ago";
    }

    double days = hours / HOURS_PER_DAY;
    if (days < DAYS_IN_MONTH_DEFAULT) {
        return std::to_string(static_cast<int>(days)) + " days ago";
    }

    double months = days / 30;
    if (months < MONTHS_PER_YEAR) {
        return std::to_string(static_cast<int>(months)) + " months ago";
    }

    double years = months / MONTHS_PER_YEAR;
    return std::to_string(static_cast<int>(years)) + " years ago";
}

TimePoint ConvertTimezone(const TimePoint& tp, const std::string& fromTz, const std::string& toTz)
{
    int fromOffset = 0;
    int toOffset = 0;
    auto fromIt = TIMEZONE_OFFSETS.find(fromTz);
    if (fromIt != TIMEZONE_OFFSETS.end()) {
        fromOffset = fromIt->second;
    }
    auto toIt = TIMEZONE_OFFSETS.find(toTz);
    if (toIt != TIMEZONE_OFFSETS.end()) {
        toOffset = toIt->second;
    }
    int offsetDiff = (toOffset - fromOffset) * SECONDS_PER_HOUR;

    return tp + std::chrono::seconds(offsetDiff);
}

std::string ToISOString(const TimePoint& tp)
{
    std::time_t tt = ToTimeT(tp);
    std::tm utcTm = {};
    std::tm* tm = std::gmtime(&tt);
    if (tm != nullptr) {
        utcTm = *tm;
    }
    std::stringstream ss;
    ss << std::set(TWO_DIGITS) << std::setfill('0') << (utcTm.tm_year + BASE_YEAR) << '-' <<
       std::setw(TWO_DIGITS) << std::setfill('0') << (utcTm.tm_mon + 1) << '-' <<
       std::setw(TWO_DIGITS) << std::setfill('0') << utcTm.tm_mday << 'T' <<
       std::setw(TWO_DIGITS) << std::setfill('0') << utcTm.tm_hour << ':' <<
       std::setw(TWO_DIGITS) << std::setfill('0') << utcTm.tm_min << ':' <<
       std::setw(TWO_DIGITS) << std::setfill('0') << utcTm.tm_sec << 'Z';
    return ss.str();
}

TimePoint FromISOString(const std::string& isoStr)
{
    std::tm tm = {};
    std::istringstream ss(isoStr);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return FromTimeT(std::mktime(&tm));
}

int CalculateAge(const TimePoint& birthDate)
{
    TimePoint nowTp = Now();
    std::time_t tt1 = ToTimeT(birthDate);
    std::time_t tt2 = ToTimeT(nowTp);
    std::tm localTm1 = {};
    std::tm localTm2 = {};
    std::tm* tm1 = std::localtime(&tt1);
    std::tm* tm2 = std::localtime(&tt2);
    if (tm1 != nullptr) {
        localTm1 = *tm1;
    }
    if (tm2 != nullptr) {
        localTm2 = *tm2;
    }

    int age = localTm2.tm_year - localTm1.tm_year;
    if (localTm2.tm_mon < localTm1.tm_mon ||
        (localTm2.tm_mon == localTm1.tm_mon && localTm2.tm_mday < localTm1.tm_mday)) {
        age--;
    }
    return age;
}

bool IsWeekday(const TimePoint& tp)
{
    int day = GetDayOfWeek(tp);
    return day != NUMBER_0 && day != SATURDAY;
}

bool IsWeekend(const TimePoint& tp)
{
    int day = GetDayOfWeek(tp);
    return day == NUMBER_0 || day == SATURDAY;
}

}

static napi_value GetCurrentTimestamp(napi_env env, napi_callback_info info)
{
    auto tp = Now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();

    napi_value result;
    napi_status status = napi_create_double(env, static_cast<double>(timestamp), &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create double");
        return nullptr;
    }
    return result;
}

static napi_value FormatDate(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp and format string");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    size_t formatLength = 0;
    status = napi_get_value_string_utf8(env, args[1], nullptr, 0, &formatLength);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string length");
        return nullptr;
    }
    char* formatBuffer = new char[formatLength + 1];
    status = napi_get_value_string_utf8(env, args[1], formatBuffer, formatLength + 1, nullptr);
    if (status != napi_ok) {
        delete[] formatBuffer;
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string value");
        return nullptr;
    }
    std::string format(formatBuffer);
    delete[] formatBuffer;

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    std::string result = FormatDateTime(tp, format);

    napi_value resultValue;
    status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create string");
        return nullptr;
    }
    return resultValue;
}

static napi_value ParseDate(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected date string and format string");
        return nullptr;
    }

    size_t dateLength = 0;
    status = napi_get_value_string_utf8(env, args[0], nullptr, 0, &dateLength);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string length");
        return nullptr;
    }
    char* dateBuffer = new char[dateLength + 1];
    status = napi_get_value_string_utf8(env, args[0], dateBuffer, dateLength + 1, nullptr);
    if (status != napi_ok) {
        delete[] dateBuffer;
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string value");
        return nullptr;
    }
    std::string dateStr(dateBuffer);
    delete[] dateBuffer;

    size_t formatLength = 0;
    status = napi_get_value_string_utf8(env, args[1], nullptr, 0, &formatLength);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string length");
        return nullptr;
    }
    char* formatBuffer = new char[formatLength + 1];
    status = napi_get_value_string_utf8(env, args[1], formatBuffer, formatLength + 1, nullptr);
    if (status != napi_ok) {
        delete[] formatBuffer;
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string value");
        return nullptr;
    }
    std::string format(formatBuffer);
    delete[] formatBuffer;

    TimePoint tp = ParseDateTime(dateStr, format);
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();

    napi_value result;
    status = napi_create_double(env, static_cast<double>(timestamp), &result);
    return result;
}

static napi_value DateDiff(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected two timestamps");
        return nullptr;
    }

    double timestamp1 = 0.0;
    double timestamp2 = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp1);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }
    status = napi_get_value_double(env, args[1], &timestamp2);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp1 = FromTimeT(static_cast<std::time_t>(timestamp1 / 1000));
    TimePoint tp2 = FromTimeT(static_cast<std::time_t>(timestamp2 / 1000));
    double diff = DateDiffDays(tp1, tp2);

    napi_value result;
    status = napi_create_double(env, diff, &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create double");
        return nullptr;
    }
    return result;
}

static napi_value AddDays(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp and days");
        return nullptr;
    }

    double timestamp = 0.0;
    int days = 0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[1], &days);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get int32 value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    TimePoint newTp = AddDays(tp, days);
    auto newTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(newTp.time_since_epoch()).count();

    napi_value result;
    status = napi_create_double(env, static_cast<double>(newTimestamp), &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create double");
        return nullptr;
    }
    return result;
}

static napi_value AddMonths(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp and months");
        return nullptr;
    }

    double timestamp = 0.0;
    int months = 0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[1], &months);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get int32 value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    TimePoint newTp = AddMonths(tp, months);
    auto newTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(newTp.time_since_epoch()).count();

    napi_value result;
    status = napi_create_double(env, static_cast<double>(newTimestamp), &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create double");
        return nullptr;
    }
    return result;
}

static napi_value AddYears(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp and years");
        return nullptr;
    }

    double timestamp = 0.0;
    int years = 0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[1], &years);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get int32 value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    TimePoint newTp = AddYears(tp, years);
    auto newTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(newTp.time_since_epoch()).count();

    napi_value result;
    status = napi_create_double(env, static_cast<double>(newTimestamp), &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create double");
        return nullptr;
    }
    return result;
}

static napi_value IsLeapYear(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected year");
        return nullptr;
    }

    int year = 0;
    status = napi_get_value_int32(env, args[0], &year);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get int32 value");
        return nullptr;
    }

    bool result = IsLeapYear(year);
    napi_value resultValue;
    status = napi_create_boolean(env, result, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create boolean");
        return nullptr;
    }
    return resultValue;
}

static napi_value GetDaysInMonth(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected year and month");
        return nullptr;
    }

    int year = 0;
    int month = 0;
    status = napi_get_value_int32(env, args[0], &year);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get int32 value");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[1], &month);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get int32 value");
        return nullptr;
    }

    int days = GetDaysInMonth(year, month - 1);
    napi_value result;
    status = napi_create_int32(env, days, &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create int32");
        return nullptr;
    }
    return result;
}

static napi_value GetDayOfWeek(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    int day = GetDayOfWeek(tp);

    napi_value result;
    status = napi_create_int32(env, day, &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create int32");
        return nullptr;
    }
    return result;
}

static napi_value GetQuarter(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    int quarter = GetQuarter(tp);

    napi_value result;
    status = napi_create_int32(env, quarter, &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create int32");
        return nullptr;
    }
    return result;
}

static napi_value GetWeekOfYear(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    int week = GetWeekOfYear(tp);

    napi_value result;
    status = napi_create_int32(env, week, &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create int32");
        return nullptr;
    }
    return result;
}

static napi_value IsSameDay(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected two timestamps");
        return nullptr;
    }

    double timestamp1 = 0.0;
    double timestamp2 = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp1);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }
    status = napi_get_value_double(env, args[1], &timestamp2);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp1 = FromTimeT(static_cast<std::time_t>(timestamp1 / 1000));
    TimePoint tp2 = FromTimeT(static_cast<std::time_t>(timestamp2 / 1000));
    bool result = IsSameDay(tp1, tp2);

    napi_value resultValue;
    status = napi_create_boolean(env, result, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create boolean");
        return nullptr;
    }
    return resultValue;
}

static napi_value IsInRange(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_3;
    napi_value args[NUMBER_3];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_3) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp, start and end");
        return nullptr;
    }

    double timestamp = 0.0;
    double start = 0.0;
    double end = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }
    status = napi_get_value_double(env, args[1], &start);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }
    status = napi_get_value_double(env, args[NUMBER_2], &end);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    TimePoint startTp = FromTimeT(static_cast<std::time_t>(start / 1000));
    TimePoint endTp = FromTimeT(static_cast<std::time_t>(end / 1000));
    bool result = IsInRange(tp, startTp, endTp);

    napi_value resultValue;
    status = napi_create_boolean(env, result, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create boolean");
        return nullptr;
    }
    return resultValue;
}

static napi_value GetRelativeTime(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    std::string result = GetRelativeTime(tp);

    napi_value resultValue;
    status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create string");
        return nullptr;
    }
    return resultValue;
}

static napi_status GetTimezoneString(napi_env env, napi_value arg, std::string& result)
{
    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, arg, nullptr, 0, &length);
    if (status != napi_ok) {
        return status;
    }
    char* buffer = new char[length + 1];
    status = napi_get_value_string_utf8(env, arg, buffer, length + 1, nullptr);
    if (status != napi_ok) {
        delete[] buffer;
        return status;
    }
    result = std::string(buffer);
    delete[] buffer;
    return napi_ok;
}

static napi_value ConvertTimezone(napi_env env, napi_callback_info info)
{
    size_t argc = THREE_ARGS;
    napi_value args[THREE_ARGS];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < THREE_ARGS) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp, from timezone and to timezone");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    std::string fromTz;
    status = GetTimezoneString(env, args[1], fromTz);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get from timezone");
        return nullptr;
    }

    std::string toTz;
    status = GetTimezoneString(env, args[1], toTz);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get to timezone");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    TimePoint newTp = ConvertTimezone(tp, fromTz, toTz);
    auto newTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(newTp.time_since_epoch()).count();

    napi_value result;
    status = napi_create_double(env, static_cast<double>(newTimestamp), &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create double");
        return nullptr;
    }
    return result;
}

static napi_value GetDateParts(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    std::time_t tt = ToTimeT(tp);
    std::tm localTm = {};
    std::tm* tm = std::localtime(&tt);
    if (tm != nullptr) {
        localTm = *tm;
    }

    napi_value result;
    status = napi_create_object(env, &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create object");
        return nullptr;
    }
    return result;
}

static napi_value ToISOString(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    std::string result = ToISOString(tp);

    napi_value resultValue;
    status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create string");
        return nullptr;
    }
    return resultValue;
}

static napi_value FromISOString(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected ISO string");
        return nullptr;
    }

    size_t isoLength = 0;
    status = napi_get_value_string_utf8(env, args[0], nullptr, 0, &isoLength);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string length");
        return nullptr;
    }
    char* isoBuffer = new char[isoLength + 1];
    status = napi_get_value_string_utf8(env, args[0], isoBuffer, isoLength + 1, nullptr);
    if (status != napi_ok) {
        delete[] isoBuffer;
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string value");
        return nullptr;
    }
    std::string isoStr(isoBuffer);
    delete[] isoBuffer;

    TimePoint tp = FromISOString(isoStr);
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();

    napi_value result;
    status = napi_create_double(env, static_cast<double>(timestamp), &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create double");
        return nullptr;
    }
    return result;
}

static napi_value GetCurrentDate(napi_env env, napi_callback_info info)
{
    TimePoint tp = Now();
    std::string result = FormatDateTime(tp, "%Y-%m-%d");

    napi_value resultValue;
    napi_status status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create string");
        return nullptr;
    }
    return resultValue;
}

static napi_value GetCurrentTime(napi_env env, napi_callback_info info)
{
    TimePoint tp = Now();
    std::string result = FormatDateTime(tp, "%H:%M:%S");

    napi_value resultValue;
    napi_status status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create string");
        return nullptr;
    }
    return resultValue;
}

static napi_value GetCurrentDateTime(napi_env env, napi_callback_info info)
{
    TimePoint tp = Now();
    std::string result = FormatDateTime(tp, "%Y-%m-%d %H:%M:%S");

    napi_value resultValue;
    napi_status status = napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create string");
        return nullptr;
    }
    return resultValue;
}

static napi_value CalculateAge(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected birth timestamp");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    int age = CalculateAge(tp);

    napi_value result;
    status = napi_create_int32(env, age, &result);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create int32");
        return nullptr;
    }
    return result;
}

static napi_value IsWeekday(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    bool result = IsWeekday(tp);

    napi_value resultValue;
    status = napi_create_boolean(env, result, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create boolean");
        return nullptr;
    }
    return resultValue;
}

static napi_value IsWeekend(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected timestamp");
        return nullptr;
    }

    double timestamp = 0.0;
    status = napi_get_value_double(env, args[0], &timestamp);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get double value");
        return nullptr;
    }

    TimePoint tp = FromTimeT(static_cast<std::time_t>(timestamp / 1000));
    bool result = IsWeekend(tp);

    napi_value resultValue;
    status = napi_create_boolean(env, result, &resultValue);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to create boolean");
        return nullptr;
    }
    return resultValue;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        {"getCurrentTimestamp", nullptr, GetCurrentTimestamp, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"formatDate", nullptr, FormatDate, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"parseDate", nullptr, ParseDate, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"dateDiff", nullptr, DateDiff, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"addDays", nullptr, AddDays, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"addMonths", nullptr, AddMonths, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"addYears", nullptr, AddYears, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"isLeapYear", nullptr, IsLeapYear, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getDaysInMonth", nullptr, GetDaysInMonth, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getDayOfWeek", nullptr, GetDayOfWeek, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getQuarter", nullptr, GetQuarter, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getWeekOfYear", nullptr, GetWeekOfYear, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"isSameDay", nullptr, IsSameDay, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"isInRange", nullptr, IsInRange, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getRelativeTime", nullptr, GetRelativeTime, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"convertTimezone", nullptr, ConvertTimezone, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getDateParts", nullptr, GetDateParts, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"toISOString", nullptr, ToISOString, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"fromISOString", nullptr, FromISOString, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getCurrentDate", nullptr, GetCurrentDate, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getCurrentTime", nullptr, GetCurrentTime, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getCurrentDateTime", nullptr, GetCurrentDateTime, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"calculateAge", nullptr, CalculateAge, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"isWeekday", nullptr, IsWeekday, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"isWeekend", nullptr, IsWeekend, nullptr, nullptr, nullptr, napi_default, nullptr}
    };

    napi_status status =
        napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to define properties");
        return nullptr;
    }
    return exports;
}

NAPI_MODULE(napi_date, Init)
