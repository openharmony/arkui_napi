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

const dateModule = require('libnapi_date.so');

module.exports = {
  // 获取当前时间戳
  getCurrentTimestamp: dateModule.getCurrentTimestamp,
  // 格式化日期
  formatDate: dateModule.formatDate,
  // 解析日期字符串
  parseDate: dateModule.parseDate,
  // 计算日期差值
  dateDiff: dateModule.dateDiff,
  // 日期加减
  addDays: dateModule.addDays,
  addMonths: dateModule.addMonths,
  addYears: dateModule.addYears,
  // 检查是否为闰年
  isLeapYear: dateModule.isLeapYear,
  // 获取月份天数
  getDaysInMonth: dateModule.getDaysInMonth,
  // 获取星期几
  getDayOfWeek: dateModule.getDayOfWeek,
  // 获取季度
  getQuarter: dateModule.getQuarter,
  // 获取年份的第几周
  getWeekOfYear: dateModule.getWeekOfYear,
  // 检查是否是同一天
  isSameDay: dateModule.isSameDay,
  // 检查是否在指定范围内
  isInRange: dateModule.isInRange,
  // 获取相对时间描述
  getRelativeTime: dateModule.getRelativeTime,
  // 时区转换
  convertTimezone: dateModule.convertTimezone,
  // 获取时间戳的各个部分
  getDateParts: dateModule.getDateParts,
  // 格式化时间为ISO字符串
  toISOString: dateModule.toISOString,
  // 从ISO字符串解析
  fromISOString: dateModule.fromISOString,
  // 获取当前日期
  getCurrentDate: dateModule.getCurrentDate,
  // 获取当前时间
  getCurrentTime: dateModule.getCurrentTime,
  // 获取当前日期时间
  getCurrentDateTime: dateModule.getCurrentDateTime,
  // 计算年龄
  calculateAge: dateModule.calculateAge,
  // 检查是否是工作日
  isWeekday: dateModule.isWeekday,
  // 检查是否是周末
  isWeekend: dateModule.isWeekend
};
