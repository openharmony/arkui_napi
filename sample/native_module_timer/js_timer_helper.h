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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_TIMER_JS_TIMER_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_TIMER_JS_TIMER_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

constexpr size_t TIMER_LABEL_SIZE = 128;
constexpr size_t MAX_LAP_COUNT = 256;

enum TimerArgIndex {
    TIMER_FIRST_ARG = 0,
    TIMER_SECOND_ARG,
    TIMER_THIRD_ARG,
};

namespace TimerArgCount {
    constexpr size_t ONE = 1;
    constexpr size_t TWO = 2;
    constexpr size_t THREE = 3;
};

namespace TimeConst {
    constexpr double MILLIS_PER_SECOND = 1000.0;
    constexpr double MILLIS_PER_MINUTE = 60000.0;
    constexpr double MILLIS_PER_HOUR = 3600000.0;
    constexpr double MILLIS_PER_DAY = 86400000.0;
    constexpr double SECONDS_PER_MINUTE = 60.0;
    constexpr double SECONDS_PER_HOUR = 3600.0;
    constexpr double SECONDS_PER_DAY = 86400.0;
    constexpr int TM_YEAR_BASE = 1900;
    constexpr int MONTHS_IN_YEAR = 12;
    constexpr int DAYS_IN_LEAP_YEAR = 366;
    constexpr int FEBRUARY = 2;
    constexpr int LEAP_YEAR_MOD_4 = 4;
    constexpr int LEAP_YEAR_MOD_100 = 100;
    constexpr int LEAP_YEAR_MOD_400 = 400;
    constexpr int PERCENT_MULTIPLIER = 100;
    constexpr int FEBRUARY_LEAP_DAYS = 29;
    constexpr int DAYS_IN_YEAR = 365;
};

struct TimerRecord {
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    bool running;
    std::vector<std::chrono::steady_clock::time_point> laps;

    TimerRecord() : running(false) {}
};

struct TimerAsyncContext {
    napi_env env = nullptr;
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    int64_t delayMs = 0;
    int status = 0;
};

class TimerManager {
public:
    static TimerManager& GetInstance();

    bool Start(const std::string& label);
    bool Stop(const std::string& label);
    bool Reset(const std::string& label);
    bool Lap(const std::string& label);
    double Elapsed(const std::string& label) const;
    double ElapsedMicro(const std::string& label) const;
    std::vector<double> GetLaps(const std::string& label) const;
    bool IsRunning(const std::string& label) const;
    bool Exists(const std::string& label) const;
    void Remove(const std::string& label);
    void Clear();
    std::vector<std::string> GetLabels() const;
    size_t Count() const;

private:
    TimerManager() {}
    std::map<std::string, TimerRecord> timers_;
};

struct DateComponents {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

class DateTimeHelper {
public:
    static int64_t NowMillis();
    static int64_t NowMicros();
    static int64_t NowNanos();
    static int GetYear(int64_t millis);
    static int GetMonth(int64_t millis);
    static int GetDay(int64_t millis);
    static int GetHour(int64_t millis);
    static int GetMinute(int64_t millis);
    static int GetSecond(int64_t millis);
    static int GetDayOfWeek(int64_t millis);
    static int GetDayOfYear(int64_t millis);
    static bool IsLeapYear(int year);
    static int DaysInMonth(int year, int month);
    static int DaysInYear(int year);
    static int64_t DateToMillis(const DateComponents& date);
};

class DurationHelper {
public:
    static int64_t SecondsToMillis(double seconds);
    static int64_t MinutesToMillis(double minutes);
    static int64_t HoursToMillis(double hours);
    static int64_t DaysToMillis(double days);
    static double MillisToSeconds(int64_t millis);
    static double MillisToMinutes(int64_t millis);
    static double MillisToHours(int64_t millis);
    static double MillisToDays(int64_t millis);
    static std::string FormatDuration(int64_t millis);
};

#endif
