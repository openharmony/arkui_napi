/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_timer_helper.h"

#include "securec.h"

#include <ctime>

/***********************************************
 * TimerManager Implementation
 ***********************************************/
TimerManager& TimerManager::GetInstance()
{
    static TimerManager instance;
    return instance;
}

bool TimerManager::Start(const std::string& label)
{
    auto& record = timers_[label];
    if (record.running) {
        return false;
    }
    record.startTime = std::chrono::steady_clock::now();
    record.running = true;
    record.laps.clear();
    return true;
}

bool TimerManager::Stop(const std::string& label)
{
    auto it = timers_.find(label);
    if (it == timers_.end() || !it->second.running) {
        return false;
    }
    it->second.endTime = std::chrono::steady_clock::now();
    it->second.running = false;
    return true;
}

bool TimerManager::Reset(const std::string& label)
{
    auto it = timers_.find(label);
    if (it == timers_.end()) {
        return false;
    }
    it->second = TimerRecord();
    return true;
}

bool TimerManager::Lap(const std::string& label)
{
    auto it = timers_.find(label);
    if (it == timers_.end() || !it->second.running) {
        return false;
    }
    if (it->second.laps.size() >= MAX_LAP_COUNT) {
        return false;
    }
    it->second.laps.push_back(std::chrono::steady_clock::now());
    return true;
}

double TimerManager::Elapsed(const std::string& label) const
{
    auto it = timers_.find(label);
    if (it == timers_.end()) {
        return 0.0;
    }
    auto end = it->second.running ? std::chrono::steady_clock::now() : it->second.endTime;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - it->second.startTime);
    return static_cast<double>(duration.count());
}

double TimerManager::ElapsedMicro(const std::string& label) const
{
    auto it = timers_.find(label);
    if (it == timers_.end()) {
        return 0.0;
    }
    auto end = it->second.running ? std::chrono::steady_clock::now() : it->second.endTime;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - it->second.startTime);
    return static_cast<double>(duration.count());
}

std::vector<double> TimerManager::GetLaps(const std::string& label) const
{
    std::vector<double> result;
    auto it = timers_.find(label);
    if (it == timers_.end()) {
        return result;
    }
    auto prev = it->second.startTime;
    for (const auto& lap : it->second.laps) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(lap - prev);
        result.push_back(static_cast<double>(duration.count()));
        prev = lap;
    }
    return result;
}

bool TimerManager::IsRunning(const std::string& label) const
{
    auto it = timers_.find(label);
    return it != timers_.end() && it->second.running;
}

bool TimerManager::Exists(const std::string& label) const
{
    return timers_.find(label) != timers_.end();
}

void TimerManager::Remove(const std::string& label)
{
    timers_.erase(label);
}

void TimerManager::Clear()
{
    timers_.clear();
}

std::vector<std::string> TimerManager::GetLabels() const
{
    std::vector<std::string> labels;
    for (const auto& pair : timers_) {
        labels.push_back(pair.first);
    }
    return labels;
}

size_t TimerManager::Count() const
{
    return timers_.size();
}

/***********************************************
 * DateTimeHelper Implementation
 ***********************************************/
int64_t DateTimeHelper::NowMillis()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

int64_t DateTimeHelper::NowMicros()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

int64_t DateTimeHelper::NowNanos()
{
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

static struct tm* GetLocalTime(int64_t millis, struct tm* result)
{
    time_t seconds = static_cast<time_t>(millis / static_cast<int64_t>(TimeConst::MILLIS_PER_SECOND));
    return localtime_r(&seconds, result);
}

int DateTimeHelper::GetYear(int64_t millis)
{
    struct tm tmBuf;
    struct tm* tm = GetLocalTime(millis, &tmBuf);
    return tm ? tm->tm_year + TimeConst::TM_YEAR_BASE : 0;
}

int DateTimeHelper::GetMonth(int64_t millis)
{
    struct tm tmBuf;
    struct tm* tm = GetLocalTime(millis, &tmBuf);
    return tm ? tm->tm_mon + 1 : 0;
}

int DateTimeHelper::GetDay(int64_t millis)
{
    struct tm tmBuf;
    struct tm* tm = GetLocalTime(millis, &tmBuf);
    return tm ? tm->tm_mday : 0;
}

int DateTimeHelper::GetHour(int64_t millis)
{
    struct tm tmBuf;
    struct tm* tm = GetLocalTime(millis, &tmBuf);
    return tm ? tm->tm_hour : 0;
}

int DateTimeHelper::GetMinute(int64_t millis)
{
    struct tm tmBuf;
    struct tm* tm = GetLocalTime(millis, &tmBuf);
    return tm ? tm->tm_min : 0;
}

int DateTimeHelper::GetSecond(int64_t millis)
{
    struct tm tmBuf;
    struct tm* tm = GetLocalTime(millis, &tmBuf);
    return tm ? tm->tm_sec : 0;
}

int DateTimeHelper::GetDayOfWeek(int64_t millis)
{
    struct tm tmBuf;
    struct tm* tm = GetLocalTime(millis, &tmBuf);
    return tm ? tm->tm_wday : 0;
}

int DateTimeHelper::GetDayOfYear(int64_t millis)
{
    struct tm tmBuf;
    struct tm* tm = GetLocalTime(millis, &tmBuf);
    return tm ? tm->tm_yday + 1 : 0;
}

bool DateTimeHelper::IsLeapYear(int year)
{
    return (year % TimeConst::LEAP_YEAR_MOD_4 == 0 && year % TimeConst::LEAP_YEAR_MOD_100 != 0) ||
        (year % TimeConst::LEAP_YEAR_MOD_400 == 0);
}

int DateTimeHelper::DaysInMonth(int year, int month)
{
    static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month < 1 || month > TimeConst::MONTHS_IN_YEAR) {
        return 0;
    }
    if (month == TimeConst::FEBRUARY && IsLeapYear(year)) {
        return TimeConst::FEBRUARY_LEAP_DAYS;
    }
    return days[month - 1];
}

int DateTimeHelper::DaysInYear(int year)
{
    return IsLeapYear(year) ? TimeConst::DAYS_IN_LEAP_YEAR : TimeConst::DAYS_IN_YEAR;
}

int64_t DateTimeHelper::DateToMillis(const DateComponents& date)
{
    struct tm t = {};
    t.tm_year = date.year - TimeConst::TM_YEAR_BASE;
    t.tm_mon = date.month - 1;
    t.tm_mday = date.day;
    t.tm_hour = date.hour;
    t.tm_min = date.minute;
    t.tm_sec = date.second;
    time_t time = mktime(&t);
    return static_cast<int64_t>(time) * static_cast<int64_t>(TimeConst::MILLIS_PER_SECOND);
}

/***********************************************
 * DurationHelper Implementation
 ***********************************************/
int64_t DurationHelper::SecondsToMillis(double seconds)
{
    return static_cast<int64_t>(seconds * TimeConst::MILLIS_PER_SECOND);
}
int64_t DurationHelper::MinutesToMillis(double minutes)
{
    return static_cast<int64_t>(minutes * TimeConst::MILLIS_PER_MINUTE);
}
int64_t DurationHelper::HoursToMillis(double hours)
{
    return static_cast<int64_t>(hours * TimeConst::MILLIS_PER_HOUR);
}
int64_t DurationHelper::DaysToMillis(double days)
{
    return static_cast<int64_t>(days * TimeConst::MILLIS_PER_DAY);
}
double DurationHelper::MillisToSeconds(int64_t millis)
{
    return static_cast<double>(millis) / TimeConst::MILLIS_PER_SECOND;
}
double DurationHelper::MillisToMinutes(int64_t millis)
{
    return static_cast<double>(millis) / TimeConst::MILLIS_PER_MINUTE;
}
double DurationHelper::MillisToHours(int64_t millis)
{
    return static_cast<double>(millis) / TimeConst::MILLIS_PER_HOUR;
}
double DurationHelper::MillisToDays(int64_t millis)
{
    return static_cast<double>(millis) / TimeConst::MILLIS_PER_DAY;
}

std::string DurationHelper::FormatDuration(int64_t millis)
{
    bool negative = millis < 0;
    if (negative) {
        millis = -millis;
    }
    int64_t totalSeconds = millis / static_cast<int64_t>(TimeConst::MILLIS_PER_SECOND);
    int ms = static_cast<int>(millis % static_cast<int64_t>(TimeConst::MILLIS_PER_SECOND));
    int s = static_cast<int>(totalSeconds % static_cast<int64_t>(TimeConst::SECONDS_PER_MINUTE));
    int m = static_cast<int>((totalSeconds / static_cast<int64_t>(TimeConst::SECONDS_PER_MINUTE)) %
        static_cast<int64_t>(TimeConst::SECONDS_PER_MINUTE));
    int h = static_cast<int>(totalSeconds / static_cast<int64_t>(TimeConst::SECONDS_PER_HOUR));
    char buf[64] = { 0 };
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%s%02d:%02d:%02d.%03d",
        negative ? "-" : "", h, m, s, ms) < 0) {
        return "00:00:00.000";
    }
    return std::string(buf);
}

/***********************************************
 * NAPI Functions - Timer
 ***********************************************/
static napi_value JSTimerStart(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char label[TIMER_LABEL_SIZE] = { 0 };
    size_t labelLen = 0;
    napi_get_value_string_utf8(env, argv[0], label, TIMER_LABEL_SIZE, &labelLen);
    bool success = TimerManager::GetInstance().Start(std::string(label, labelLen));
    napi_value result = nullptr;
    napi_get_boolean(env, success, &result);
    return result;
}

static napi_value JSTimerStop(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char label[TIMER_LABEL_SIZE] = { 0 };
    size_t labelLen = 0;
    napi_get_value_string_utf8(env, argv[0], label, TIMER_LABEL_SIZE, &labelLen);
    bool success = TimerManager::GetInstance().Stop(std::string(label, labelLen));
    napi_value result = nullptr;
    napi_get_boolean(env, success, &result);
    return result;
}

static napi_value JSTimerReset(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char label[TIMER_LABEL_SIZE] = { 0 };
    size_t labelLen = 0;
    napi_get_value_string_utf8(env, argv[0], label, TIMER_LABEL_SIZE, &labelLen);
    bool success = TimerManager::GetInstance().Reset(std::string(label, labelLen));
    napi_value result = nullptr;
    napi_get_boolean(env, success, &result);
    return result;
}

static napi_value JSTimerLap(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char label[TIMER_LABEL_SIZE] = { 0 };
    size_t labelLen = 0;
    napi_get_value_string_utf8(env, argv[0], label, TIMER_LABEL_SIZE, &labelLen);
    bool success = TimerManager::GetInstance().Lap(std::string(label, labelLen));
    napi_value result = nullptr;
    napi_get_boolean(env, success, &result);
    return result;
}

static napi_value JSTimerElapsed(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char label[TIMER_LABEL_SIZE] = { 0 };
    size_t labelLen = 0;
    napi_get_value_string_utf8(env, argv[0], label, TIMER_LABEL_SIZE, &labelLen);
    double elapsed = TimerManager::GetInstance().Elapsed(std::string(label, labelLen));
    napi_value result = nullptr;
    napi_create_double(env, elapsed, &result);
    return result;
}

static napi_value JSTimerElapsedMicro(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char label[TIMER_LABEL_SIZE] = { 0 };
    size_t labelLen = 0;
    napi_get_value_string_utf8(env, argv[0], label, TIMER_LABEL_SIZE, &labelLen);
    double elapsed = TimerManager::GetInstance().ElapsedMicro(std::string(label, labelLen));
    napi_value result = nullptr;
    napi_create_double(env, elapsed, &result);
    return result;
}

static napi_value JSTimerIsRunning(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char label[TIMER_LABEL_SIZE] = { 0 };
    size_t labelLen = 0;
    napi_get_value_string_utf8(env, argv[0], label, TIMER_LABEL_SIZE, &labelLen);
    napi_value result = nullptr;
    napi_get_boolean(env, TimerManager::GetInstance().IsRunning(std::string(label, labelLen)), &result);
    return result;
}

static napi_value JSTimerRemove(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char label[TIMER_LABEL_SIZE] = { 0 };
    size_t labelLen = 0;
    napi_get_value_string_utf8(env, argv[0], label, TIMER_LABEL_SIZE, &labelLen);
    TimerManager::GetInstance().Remove(std::string(label, labelLen));
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSTimerClear(napi_env env, napi_callback_info info)
{
    TimerManager::GetInstance().Clear();
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSTimerCount(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_create_int32(env, static_cast<int32_t>(TimerManager::GetInstance().Count()), &result);
    return result;
}

/***********************************************
 * NAPI Functions - DateTime
 ***********************************************/
static napi_value JSNowMillis(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_create_int64(env, DateTimeHelper::NowMillis(), &result);
    return result;
}

static napi_value JSNowMicros(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_create_int64(env, DateTimeHelper::NowMicros(), &result);
    return result;
}

static napi_value JSIsLeapYear(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int32_t year;
    napi_get_value_int32(env, argv[0], &year);
    napi_value result = nullptr;
    napi_get_boolean(env, DateTimeHelper::IsLeapYear(year), &result);
    return result;
}

static napi_value JSDaysInMonth(napi_env env, napi_callback_info info)
{
    size_t argc = TimerArgCount::TWO;
    napi_value argv[TimerArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= TimerArgCount::TWO, "requires 2 parameters: year, month");
    int32_t year;
    int32_t month;
    napi_get_value_int32(env, argv[TIMER_FIRST_ARG], &year);
    napi_get_value_int32(env, argv[TIMER_SECOND_ARG], &month);
    napi_value result = nullptr;
    napi_create_int32(env, DateTimeHelper::DaysInMonth(year, month), &result);
    return result;
}

static napi_value JSDaysInYear(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int32_t year;
    napi_get_value_int32(env, argv[0], &year);
    napi_value result = nullptr;
    napi_create_int32(env, DateTimeHelper::DaysInYear(year), &result);
    return result;
}

/***********************************************
 * NAPI Functions - Duration
 ***********************************************/
static napi_value JSFormatDuration(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int64_t millis;
    napi_get_value_int64(env, argv[0], &millis);
    std::string formatted = DurationHelper::FormatDuration(millis);
    napi_value result = nullptr;
    napi_create_string_utf8(env, formatted.c_str(), formatted.length(), &result);
    return result;
}

static napi_value JSSecondsToMillis(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    double seconds;
    napi_get_value_double(env, argv[0], &seconds);
    napi_value result = nullptr;
    napi_create_int64(env, DurationHelper::SecondsToMillis(seconds), &result);
    return result;
}

static napi_value JSMillisToSeconds(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int64_t millis;
    napi_get_value_int64(env, argv[0], &millis);
    napi_value result = nullptr;
    napi_create_double(env, DurationHelper::MillisToSeconds(millis), &result);
    return result;
}

static napi_value JSHoursToMillis(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    double hours;
    napi_get_value_double(env, argv[0], &hours);
    napi_value result = nullptr;
    napi_create_int64(env, DurationHelper::HoursToMillis(hours), &result);
    return result;
}

static napi_value JSMillisToHours(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int64_t millis;
    napi_get_value_int64(env, argv[0], &millis);
    napi_value result = nullptr;
    napi_create_double(env, DurationHelper::MillisToHours(millis), &result);
    return result;
}

static napi_value JSMinutesToMillis(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    double minutes;
    napi_get_value_double(env, argv[0], &minutes);
    napi_value result = nullptr;
    napi_create_int64(env, DurationHelper::MinutesToMillis(minutes), &result);
    return result;
}

static napi_value JSMillisToMinutes(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int64_t millis;
    napi_get_value_int64(env, argv[0], &millis);
    napi_value result = nullptr;
    napi_create_double(env, DurationHelper::MillisToMinutes(millis), &result);
    return result;
}

static napi_value JSDaysToMillis(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    double days;
    napi_get_value_double(env, argv[0], &days);
    napi_value result = nullptr;
    napi_create_int64(env, DurationHelper::DaysToMillis(days), &result);
    return result;
}

static napi_value JSMillisToDays(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int64_t millis;
    napi_get_value_int64(env, argv[0], &millis);
    napi_value result = nullptr;
    napi_create_double(env, DurationHelper::MillisToDays(millis), &result);
    return result;
}

static napi_value JSTimerGetLaps(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char label[TIMER_LABEL_SIZE] = { 0 };
    size_t labelLen = 0;
    napi_get_value_string_utf8(env, argv[0], label, TIMER_LABEL_SIZE, &labelLen);
    std::vector<double> laps = TimerManager::GetInstance().GetLaps(std::string(label, labelLen));
    napi_value result = nullptr;
    napi_create_array_with_length(env, laps.size(), &result);
    for (size_t i = 0; i < laps.size(); i++) {
        napi_value val = nullptr;
        napi_create_double(env, laps[i], &val);
        napi_set_element(env, result, i, val);
    }
    return result;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value TimerExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("start", JSTimerStart),
        DECLARE_NAPI_FUNCTION("stop", JSTimerStop),
        DECLARE_NAPI_FUNCTION("reset", JSTimerReset),
        DECLARE_NAPI_FUNCTION("lap", JSTimerLap),
        DECLARE_NAPI_FUNCTION("elapsed", JSTimerElapsed),
        DECLARE_NAPI_FUNCTION("elapsedMicro", JSTimerElapsedMicro),
        DECLARE_NAPI_FUNCTION("isRunning", JSTimerIsRunning),
        DECLARE_NAPI_FUNCTION("remove", JSTimerRemove),
        DECLARE_NAPI_FUNCTION("clear", JSTimerClear),
        DECLARE_NAPI_FUNCTION("count", JSTimerCount),
        DECLARE_NAPI_FUNCTION("nowMillis", JSNowMillis),
        DECLARE_NAPI_FUNCTION("nowMicros", JSNowMicros),
        DECLARE_NAPI_FUNCTION("isLeapYear", JSIsLeapYear),
        DECLARE_NAPI_FUNCTION("daysInMonth", JSDaysInMonth),
        DECLARE_NAPI_FUNCTION("daysInYear", JSDaysInYear),
        DECLARE_NAPI_FUNCTION("formatDuration", JSFormatDuration),
        DECLARE_NAPI_FUNCTION("secondsToMillis", JSSecondsToMillis),
        DECLARE_NAPI_FUNCTION("millisToSeconds", JSMillisToSeconds),
        DECLARE_NAPI_FUNCTION("hoursToMillis", JSHoursToMillis),
        DECLARE_NAPI_FUNCTION("millisToHours", JSMillisToHours),
        DECLARE_NAPI_FUNCTION("minutesToMillis", JSMinutesToMillis),
        DECLARE_NAPI_FUNCTION("millisToMinutes", JSMillisToMinutes),
        DECLARE_NAPI_FUNCTION("daysToMillis", JSDaysToMillis),
        DECLARE_NAPI_FUNCTION("millisToDays", JSMillisToDays),
        DECLARE_NAPI_FUNCTION("getLaps", JSTimerGetLaps),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_timerModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = TimerExport,
    .nm_modname = "timer",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void TimerRegister()
{
    napi_module_register(&g_timerModule);
}
