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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_DATE_UTILS_JS_DATE_UTILS_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_DATE_UTILS_JS_DATE_UTILS_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <string>

struct DateInfo {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    DateInfo() : year(0), month(0), day(0), hour(0), minute(0), second(0) {}
    DateInfo(int y, int m, int d) : year(y), month(m), day(d), hour(0), minute(0), second(0) {}
    DateInfo(int y, int m, int d, int h, int min, int s) : year(y), month(m), day(d), hour(h), minute(min), second(s) {}
};

class DateUtils {
public:
    static bool IsLeapYear(int year);
    static int GetDaysInMonth(int year, int month);
    static bool IsValidDate(int year, int month, int day);
    static int GetDayOfYear(int year, int month, int day);
    static int GetWeekday(int year, int month, int day);
    static int DaysBetween(int y1, int m1, int d1, int y2, int m2, int d2);
    static DateInfo AddDays(int year, int month, int day, int daysToAdd);
    static std::string FormatDate(int year, int month, int day,
        const std::string& format);
    static std::string GetSeason(int month);
    static std::string GetQuarter(int month);
    static int GetAge(int birthYear, int birthMonth, int birthDay);
    static bool IsWeekend(int year, int month, int day);
    static int GetWeekNumber(int year, int month, int day);
};

#endif
