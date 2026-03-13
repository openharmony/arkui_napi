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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_MATH_JS_MATH_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_MATH_JS_MATH_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

constexpr size_t MATH_MAX_ARRAY_SIZE = 1024;

enum MathArgIndex {
    MATH_FIRST_ARG = 0,
    MATH_SECOND_ARG,
    MATH_THIRD_ARG,
    MATH_FOURTH_ARG,
    MATH_FIFTH_ARG,
};

namespace MathArgCount {
    constexpr size_t TWO = 2;
    constexpr size_t THREE = 3;
    constexpr size_t FOUR = 4;
    constexpr size_t FIVE = 5;
};

namespace MathConst {
    constexpr double DEGREES_HALF_CIRCLE = 180.0;
    constexpr double MEDIAN_DIVISOR = 2.0;
    constexpr double MILLIS_DIVISOR = 1000.0;
    constexpr int MIN_DATA_SIZE = 2;
    constexpr int PRIME_MIN_COMPOSITE = 4;
    constexpr int PRIME_STEP = 6;
    constexpr double NEG_HALF = -0.5;
};

struct Vector2D {
    double x;
    double y;

    Vector2D() : x(0.0), y(0.0) {}
    Vector2D(double xVal, double yVal) : x(xVal), y(yVal) {}

    Vector2D Add(const Vector2D& other) const;
    Vector2D Subtract(const Vector2D& other) const;
    Vector2D Scale(double scalar) const;
    double Dot(const Vector2D& other) const;
    double Cross(const Vector2D& other) const;
    double Magnitude() const;
    Vector2D Normalize() const;
    double Distance(const Vector2D& other) const;
    double Angle() const;
    Vector2D Rotate(double angleRad) const;
    Vector2D Lerp(const Vector2D& other, double t) const;
};

struct Vector3D {
    double x;
    double y;
    double z;

    Vector3D() : x(0.0), y(0.0), z(0.0) {}
    Vector3D(double xVal, double yVal, double zVal) : x(xVal), y(yVal), z(zVal) {}

    Vector3D Add(const Vector3D& other) const;
    Vector3D Subtract(const Vector3D& other) const;
    Vector3D Scale(double scalar) const;
    double Dot(const Vector3D& other) const;
    Vector3D Cross(const Vector3D& other) const;
    double Magnitude() const;
    Vector3D Normalize() const;
    double Distance(const Vector3D& other) const;
    Vector3D Lerp(const Vector3D& other, double t) const;
};

class Statistics {
public:
    static double Mean(const std::vector<double>& data);
    static double Median(std::vector<double> data);
    static double Variance(const std::vector<double>& data);
    static double StandardDeviation(const std::vector<double>& data);
    static double Min(const std::vector<double>& data);
    static double Max(const std::vector<double>& data);
    static double Sum(const std::vector<double>& data);
    static double Range(const std::vector<double>& data);
    static double Mode(const std::vector<double>& data);
};

class MathUtils {
public:
    static int Clamp(int value, int minVal, int maxVal);
    static double ClampDouble(double value, double minVal, double maxVal);
    static double Lerp(double a, double b, double t);
    static double DegreesToRadians(double degrees);
    static double RadiansToDegrees(double radians);
    static int Factorial(int n);
    static int Fibonacci(int n);
    static int GCD(int a, int b);
    static int LCM(int a, int b);
    static bool IsPrime(int n);
    static double Map(double value, double inMin, double inMax, double outMin, double outMax);
};

#endif
