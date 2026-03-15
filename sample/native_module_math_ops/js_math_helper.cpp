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

#include "js_math_helper.h"

#include <map>

/***********************************************
 * Vector2D Implementation
 ***********************************************/
Vector2D Vector2D::Add(const Vector2D& other) const
{
    return Vector2D(x + other.x, y + other.y);
}

Vector2D Vector2D::Subtract(const Vector2D& other) const
{
    return Vector2D(x - other.x, y - other.y);
}

Vector2D Vector2D::Scale(double scalar) const
{
    return Vector2D(x * scalar, y * scalar);
}

double Vector2D::Dot(const Vector2D& other) const
{
    return x * other.x + y * other.y;
}

double Vector2D::Cross(const Vector2D& other) const
{
    return x * other.y - y * other.x;
}

double Vector2D::Magnitude() const
{
    return std::sqrt(x * x + y * y);
}

Vector2D Vector2D::Normalize() const
{
    double lenSq = x * x + y * y;
    if (lenSq == 0.0) {
        return Vector2D(0.0, 0.0);
    }
    double invLen = std::pow(lenSq, MathConst::NEG_HALF);
    return Vector2D(x * invLen, y * invLen);
}

double Vector2D::Distance(const Vector2D& other) const
{
    double dx = x - other.x;
    double dy = y - other.y;
    return std::sqrt(dx * dx + dy * dy);
}

double Vector2D::Angle() const
{
    return std::atan2(y, x);
}

Vector2D Vector2D::Rotate(double angleRad) const
{
    double cosA = std::cos(angleRad);
    double sinA = std::sin(angleRad);
    return Vector2D(x * cosA - y * sinA, x * sinA + y * cosA);
}

Vector2D Vector2D::Lerp(const Vector2D& other, double t) const
{
    return Vector2D(x + (other.x - x) * t, y + (other.y - y) * t);
}

/***********************************************
 * Vector3D Implementation
 ***********************************************/
Vector3D Vector3D::Add(const Vector3D& other) const
{
    return Vector3D(x + other.x, y + other.y, z + other.z);
}

Vector3D Vector3D::Subtract(const Vector3D& other) const
{
    return Vector3D(x - other.x, y - other.y, z - other.z);
}

Vector3D Vector3D::Scale(double scalar) const
{
    return Vector3D(x * scalar, y * scalar, z * scalar);
}

double Vector3D::Dot(const Vector3D& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

Vector3D Vector3D::Cross(const Vector3D& other) const
{
    return Vector3D(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x);
}

double Vector3D::Magnitude() const
{
    return std::sqrt(x * x + y * y + z * z);
}

Vector3D Vector3D::Normalize() const
{
    double lenSq = x * x + y * y + z * z;
    if (lenSq == 0.0) {
        return Vector3D(0.0, 0.0, 0.0);
    }
    double invLen = std::pow(lenSq, MathConst::NEG_HALF);
    return Vector3D(x * invLen, y * invLen, z * invLen);
}

double Vector3D::Distance(const Vector3D& other) const
{
    double dx = x - other.x;
    double dy = y - other.y;
    double dz = z - other.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Vector3D Vector3D::Lerp(const Vector3D& other, double t) const
{
    return Vector3D(x + (other.x - x) * t, y + (other.y - y) * t, z + (other.z - z) * t);
}

/***********************************************
 * Statistics Implementation
 ***********************************************/
double Statistics::Mean(const std::vector<double>& data)
{
    if (data.empty()) {
        return 0.0;
    }
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return sum / static_cast<double>(data.size());
}

double Statistics::Median(std::vector<double> data)
{
    if (data.empty()) {
        return 0.0;
    }
    std::sort(data.begin(), data.end());
    size_t mid = data.size() / 2;
    if (data.size() % MathConst::MIN_DATA_SIZE == 0) {
        return (data[mid - 1] + data[mid]) / MathConst::MEDIAN_DIVISOR;
    }
    return data[mid];
}

double Statistics::Variance(const std::vector<double>& data)
{
    if (data.size() < MathConst::MIN_DATA_SIZE) {
        return 0.0;
    }
    double mean = Mean(data);
    double sumSquares = 0.0;
    for (double val : data) {
        double diff = val - mean;
        sumSquares += diff * diff;
    }
    return sumSquares / static_cast<double>(data.size() - 1);
}

double Statistics::StandardDeviation(const std::vector<double>& data)
{
    return std::sqrt(Variance(data));
}

double Statistics::Min(const std::vector<double>& data)
{
    if (data.empty()) {
        return 0.0;
    }
    return *std::min_element(data.begin(), data.end());
}

double Statistics::Max(const std::vector<double>& data)
{
    if (data.empty()) {
        return 0.0;
    }
    return *std::max_element(data.begin(), data.end());
}

double Statistics::Sum(const std::vector<double>& data)
{
    return std::accumulate(data.begin(), data.end(), 0.0);
}

double Statistics::Range(const std::vector<double>& data)
{
    if (data.empty()) {
        return 0.0;
    }
    return Max(data) - Min(data);
}

double Statistics::Mode(const std::vector<double>& data)
{
    if (data.empty()) {
        return 0.0;
    }
    std::map<int, int> freq;
    for (double val : data) {
        int key = static_cast<int>(val * MathConst::MILLIS_DIVISOR);
        freq[key]++;
    }
    int maxFreq = 0;
    int modeKey = 0;
    for (const auto& pair : freq) {
        if (pair.second > maxFreq) {
            maxFreq = pair.second;
            modeKey = pair.first;
        }
    }
    return modeKey / MathConst::MILLIS_DIVISOR;
}

/***********************************************
 * MathUtils Implementation
 ***********************************************/
int MathUtils::Clamp(int value, int minVal, int maxVal)
{
    if (value < minVal) {
        return minVal;
    }
    if (value > maxVal) {
        return maxVal;
    }
    return value;
}

double MathUtils::ClampDouble(double value, double minVal, double maxVal)
{
    if (value < minVal) {
        return minVal;
    }
    if (value > maxVal) {
        return maxVal;
    }
    return value;
}

double MathUtils::Lerp(double a, double b, double t)
{
    return a + (b - a) * t;
}

double MathUtils::DegreesToRadians(double degrees)
{
    return degrees * M_PI / MathConst::DEGREES_HALF_CIRCLE;
}

double MathUtils::RadiansToDegrees(double radians)
{
    return radians * MathConst::DEGREES_HALF_CIRCLE / M_PI;
}

int MathUtils::Factorial(int n)
{
    if (n < 0) {
        return 0;
    }
    if (n <= 1) {
        return 1;
    }
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

int MathUtils::Fibonacci(int n)
{
    if (n <= 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }
    int prev = 0;
    int curr = 1;
    for (int i = 2; i <= n; i++) {
        int next = prev + curr;
        prev = curr;
        curr = next;
    }
    return curr;
}

int MathUtils::GCD(int a, int b)
{
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int MathUtils::LCM(int a, int b)
{
    if (a == 0 || b == 0) {
        return 0;
    }
    return std::abs(a * b) / GCD(a, b);
}

bool MathUtils::IsPrime(int n)
{
    if (n < MathConst::MIN_DATA_SIZE) {
        return false;
    }
    if (n < MathConst::PRIME_MIN_COMPOSITE) {
        return true;
    }
    if (n % MathConst::MIN_DATA_SIZE == 0 || n % MathArgCount::THREE == 0) {
        return false;
    }
    for (int i = 5; i * i <= n; i += MathConst::PRIME_STEP) {
        if (n % i == 0 || n % (i + MathConst::MIN_DATA_SIZE) == 0) {
            return false;
        }
    }
    return true;
}

double MathUtils::Map(double value, double inMin, double inMax, double outMin, double outMax)
{
    if (std::abs(inMax - inMin) < 1e-10) {
        return outMin;
    }
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

/***********************************************
 * NAPI Helper: Extract double array from JS array
 ***********************************************/
static bool GetDoubleArrayFromNapi(napi_env env, napi_value jsArray, std::vector<double>& result)
{
    bool isArray = false;
    napi_is_array(env, jsArray, &isArray);
    if (!isArray) {
        return false;
    }
    uint32_t length = 0;
    napi_get_array_length(env, jsArray, &length);
    if (length > MATH_MAX_ARRAY_SIZE) {
        length = MATH_MAX_ARRAY_SIZE;
    }
    result.resize(length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value element = nullptr;
        napi_get_element(env, jsArray, i, &element);
        napi_get_value_double(env, element, &result[i]);
    }
    return true;
}

/***********************************************
 * NAPI Functions - Vector2D
 ***********************************************/
static napi_value JSVec2Add(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[MATH_FIFTH_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::FOUR, "requires 4 parameters: x1, y1, x2, y2");
    double x1;
    double y1;
    double x2;
    double y2;
    napi_get_value_double(env, argv[0], &x1);
    napi_get_value_double(env, argv[1], &y1);
    napi_get_value_double(env, argv[MATH_THIRD_ARG], &x2);
    napi_get_value_double(env, argv[MATH_FOURTH_ARG], &y2);
    Vector2D v1(x1, y1);
    Vector2D v2(x2, y2);
    Vector2D result = v1.Add(v2);
    napi_value jsResult = nullptr;
    napi_create_object(env, &jsResult);
    napi_value xVal;
    napi_value yVal;
    napi_create_double(env, result.x, &xVal);
    napi_create_double(env, result.y, &yVal);
    napi_set_named_property(env, jsResult, "x", xVal);
    napi_set_named_property(env, jsResult, "y", yVal);
    return jsResult;
}

static napi_value JSVec2Subtract(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[MATH_FIFTH_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::FOUR, "requires 4 parameters: x1, y1, x2, y2");
    double x1;
    double y1;
    double x2;
    double y2;
    napi_get_value_double(env, argv[0], &x1);
    napi_get_value_double(env, argv[1], &y1);
    napi_get_value_double(env, argv[MATH_THIRD_ARG], &x2);
    napi_get_value_double(env, argv[MATH_FOURTH_ARG], &y2);
    Vector2D result = Vector2D(x1, y1).Subtract(Vector2D(x2, y2));
    napi_value jsResult = nullptr;
    napi_create_object(env, &jsResult);
    napi_value xVal;
    napi_value yVal;
    napi_create_double(env, result.x, &xVal);
    napi_create_double(env, result.y, &yVal);
    napi_set_named_property(env, jsResult, "x", xVal);
    napi_set_named_property(env, jsResult, "y", yVal);
    return jsResult;
}

static napi_value JSVec2Dot(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[MATH_FIFTH_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::FOUR, "requires 4 parameters");
    double x1;
    double y1;
    double x2;
    double y2;
    napi_get_value_double(env, argv[0], &x1);
    napi_get_value_double(env, argv[1], &y1);
    napi_get_value_double(env, argv[MATH_THIRD_ARG], &x2);
    napi_get_value_double(env, argv[MATH_FOURTH_ARG], &y2);
    double result = Vector2D(x1, y1).Dot(Vector2D(x2, y2));
    napi_value jsResult = nullptr;
    napi_create_double(env, result, &jsResult);
    return jsResult;
}

static napi_value JSVec2Magnitude(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[MATH_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::TWO, "requires 2 parameters: x, y");
    double x;
    double y;
    napi_get_value_double(env, argv[0], &x);
    napi_get_value_double(env, argv[1], &y);
    double result = Vector2D(x, y).Magnitude();
    napi_value jsResult = nullptr;
    napi_create_double(env, result, &jsResult);
    return jsResult;
}

static napi_value JSVec2Distance(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[MATH_FIFTH_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::FOUR, "requires 4 parameters");
    double x1;
    double y1;
    double x2;
    double y2;
    napi_get_value_double(env, argv[0], &x1);
    napi_get_value_double(env, argv[1], &y1);
    napi_get_value_double(env, argv[MATH_THIRD_ARG], &x2);
    napi_get_value_double(env, argv[MATH_FOURTH_ARG], &y2);
    double result = Vector2D(x1, y1).Distance(Vector2D(x2, y2));
    napi_value jsResult = nullptr;
    napi_create_double(env, result, &jsResult);
    return jsResult;
}

static napi_value JSVec2Normalize(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[MATH_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::TWO, "requires 2 parameters: x, y");
    double x;
    double y;
    napi_get_value_double(env, argv[0], &x);
    napi_get_value_double(env, argv[1], &y);
    Vector2D result = Vector2D(x, y).Normalize();
    napi_value jsResult = nullptr;
    napi_create_object(env, &jsResult);
    napi_value xVal;
    napi_value yVal;
    napi_create_double(env, result.x, &xVal);
    napi_create_double(env, result.y, &yVal);
    napi_set_named_property(env, jsResult, "x", xVal);
    napi_set_named_property(env, jsResult, "y", yVal);
    return jsResult;
}

/***********************************************
 * NAPI Functions - Statistics
 ***********************************************/
static napi_value JSStatMean(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> data;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], data), "argument must be array");
    napi_value result = nullptr;
    napi_create_double(env, Statistics::Mean(data), &result);
    return result;
}

static napi_value JSStatMedian(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> data;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], data), "argument must be array");
    napi_value result = nullptr;
    napi_create_double(env, Statistics::Median(data), &result);
    return result;
}

static napi_value JSStatVariance(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> data;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], data), "argument must be array");
    napi_value result = nullptr;
    napi_create_double(env, Statistics::Variance(data), &result);
    return result;
}

static napi_value JSStatStdDev(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> data;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], data), "argument must be array");
    napi_value result = nullptr;
    napi_create_double(env, Statistics::StandardDeviation(data), &result);
    return result;
}

static napi_value JSStatMin(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> data;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], data), "argument must be array");
    napi_value result = nullptr;
    napi_create_double(env, Statistics::Min(data), &result);
    return result;
}

static napi_value JSStatMax(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> data;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], data), "argument must be array");
    napi_value result = nullptr;
    napi_create_double(env, Statistics::Max(data), &result);
    return result;
}

static napi_value JSStatSum(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> data;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], data), "argument must be array");
    napi_value result = nullptr;
    napi_create_double(env, Statistics::Sum(data), &result);
    return result;
}

/***********************************************
 * NAPI Functions - MathUtils
 ***********************************************/
static napi_value JSClamp(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[MATH_FOURTH_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::THREE, "requires 3 parameters: value, min, max");
    double value;
    double minVal;
    double maxVal;
    napi_get_value_double(env, argv[0], &value);
    napi_get_value_double(env, argv[1], &minVal);
    napi_get_value_double(env, argv[MATH_THIRD_ARG], &maxVal);
    double result = MathUtils::ClampDouble(value, minVal, maxVal);
    napi_value jsResult = nullptr;
    napi_create_double(env, result, &jsResult);
    return jsResult;
}

static napi_value JSLerp(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[MATH_FOURTH_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::THREE, "requires 3 parameters: a, b, t");
    double a;
    double b;
    double t;
    napi_get_value_double(env, argv[0], &a);
    napi_get_value_double(env, argv[1], &b);
    napi_get_value_double(env, argv[MATH_THIRD_ARG], &t);
    napi_value result = nullptr;
    napi_create_double(env, MathUtils::Lerp(a, b, t), &result);
    return result;
}

static napi_value JSDegreesToRadians(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    double degrees;
    napi_get_value_double(env, argv[0], &degrees);
    napi_value result = nullptr;
    napi_create_double(env, MathUtils::DegreesToRadians(degrees), &result);
    return result;
}

static napi_value JSRadiansToDegrees(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    double radians;
    napi_get_value_double(env, argv[0], &radians);
    napi_value result = nullptr;
    napi_create_double(env, MathUtils::RadiansToDegrees(radians), &result);
    return result;
}

static napi_value JSFactorial(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int32_t n;
    napi_get_value_int32(env, argv[0], &n);
    napi_value result = nullptr;
    napi_create_int32(env, MathUtils::Factorial(n), &result);
    return result;
}

static napi_value JSFibonacci(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int32_t n;
    napi_get_value_int32(env, argv[0], &n);
    napi_value result = nullptr;
    napi_create_int32(env, MathUtils::Fibonacci(n), &result);
    return result;
}

static napi_value JSGCD(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[MATH_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::TWO, "requires 2 parameters");
    int32_t a;
    int32_t b;
    napi_get_value_int32(env, argv[0], &a);
    napi_get_value_int32(env, argv[1], &b);
    napi_value result = nullptr;
    napi_create_int32(env, MathUtils::GCD(a, b), &result);
    return result;
}

static napi_value JSLCM(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[MATH_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::TWO, "requires 2 parameters");
    int32_t a;
    int32_t b;
    napi_get_value_int32(env, argv[0], &a);
    napi_get_value_int32(env, argv[1], &b);
    napi_value result = nullptr;
    napi_create_int32(env, MathUtils::LCM(a, b), &result);
    return result;
}

static napi_value JSIsPrime(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    int32_t n;
    napi_get_value_int32(env, argv[0], &n);
    napi_value result = nullptr;
    napi_get_boolean(env, MathUtils::IsPrime(n), &result);
    return result;
}

static napi_value JSMapRange(napi_env env, napi_callback_info info)
{
    size_t argc = 5;
    napi_value argv[5] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= MathArgCount::FIVE, "requires 5 parameters");
    double value;
    double inMin;
    double inMax;
    double outMin;
    double outMax;
    napi_get_value_double(env, argv[0], &value);
    napi_get_value_double(env, argv[1], &inMin);
    napi_get_value_double(env, argv[MATH_THIRD_ARG], &inMax);
    napi_get_value_double(env, argv[MATH_FOURTH_ARG], &outMin);
    napi_get_value_double(env, argv[MATH_FIFTH_ARG], &outMax);
    napi_value result = nullptr;
    napi_create_double(env, MathUtils::Map(value, inMin, inMax, outMin, outMax), &result);
    return result;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value MathExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("vec2Add", JSVec2Add),
        DECLARE_NAPI_FUNCTION("vec2Subtract", JSVec2Subtract),
        DECLARE_NAPI_FUNCTION("vec2Dot", JSVec2Dot),
        DECLARE_NAPI_FUNCTION("vec2Magnitude", JSVec2Magnitude),
        DECLARE_NAPI_FUNCTION("vec2Distance", JSVec2Distance),
        DECLARE_NAPI_FUNCTION("vec2Normalize", JSVec2Normalize),
        DECLARE_NAPI_FUNCTION("mean", JSStatMean),
        DECLARE_NAPI_FUNCTION("median", JSStatMedian),
        DECLARE_NAPI_FUNCTION("variance", JSStatVariance),
        DECLARE_NAPI_FUNCTION("stddev", JSStatStdDev),
        DECLARE_NAPI_FUNCTION("min", JSStatMin),
        DECLARE_NAPI_FUNCTION("max", JSStatMax),
        DECLARE_NAPI_FUNCTION("sum", JSStatSum),
        DECLARE_NAPI_FUNCTION("clamp", JSClamp),
        DECLARE_NAPI_FUNCTION("lerp", JSLerp),
        DECLARE_NAPI_FUNCTION("degreesToRadians", JSDegreesToRadians),
        DECLARE_NAPI_FUNCTION("radiansToDegrees", JSRadiansToDegrees),
        DECLARE_NAPI_FUNCTION("factorial", JSFactorial),
        DECLARE_NAPI_FUNCTION("fibonacci", JSFibonacci),
        DECLARE_NAPI_FUNCTION("gcd", JSGCD),
        DECLARE_NAPI_FUNCTION("lcm", JSLCM),
        DECLARE_NAPI_FUNCTION("isPrime", JSIsPrime),
        DECLARE_NAPI_FUNCTION("mapRange", JSMapRange),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_mathModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = MathExport,
    .nm_modname = "math_ops",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void MathRegister()
{
    napi_module_register(&g_mathModule);
}
