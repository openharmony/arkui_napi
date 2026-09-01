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

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <algorithm>

#include "napi/native_node_api.h"
#include "geometry_suite_common.h"

double ExtractDoubleArg(napi_env env, napi_value value)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &type), 0.0);
    if (type != napi_number) {
        NAPI_CALL_BASE(env, napi_throw_error(env, nullptr, "Argument must be a number"), 0.0);
        return 0.0;
    }

    double result = 0.0;
    NAPI_CALL_BASE(env, napi_get_value_double(env, value, &result), 0.0);
    return result;
}

bool ValidateNumberArgs(napi_env env, const napi_value* argv, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        napi_valuetype type = napi_undefined;
        NAPI_CALL_BASE(env, napi_typeof(env, argv[i], &type), false);
        if (type != napi_number) {
            return false;
        }
    }
    return true;
}

napi_status ExtractPointArray(napi_env env, napi_value value, std::vector<double>& points)
{
    points.clear();

    bool isArray = false;
    NAPI_CALL_BASE(env, napi_is_array(env, value, &isArray), napi_invalid_arg);
    if (!isArray) {
        return napi_invalid_arg;
    }

    uint32_t length = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, value, &length), napi_invalid_arg);
    if (length % EVEN_CHECK != 0) {
        return napi_invalid_arg;
    }

    for (uint32_t i = 0; i < length; ++i) {
        napi_value element = nullptr;
        NAPI_CALL_BASE(env, napi_get_element(env, value, i, &element), napi_invalid_arg);

        napi_valuetype type = napi_undefined;
        NAPI_CALL_BASE(env, napi_typeof(env, element, &type), napi_invalid_arg);
        if (type != napi_number) {
            return napi_invalid_arg;
        }

        double coordinate = 0.0;
        NAPI_CALL_BASE(env, napi_get_value_double(env, element, &coordinate), napi_invalid_arg);
        points.push_back(coordinate);
    }
    return napi_ok;
}

napi_value CreateNumberValue(napi_env env, double value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));
    return result;
}

napi_value CreateBooleanValue(napi_env env, bool value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, value, &result));
    return result;
}

napi_value CreateDoubleArrayValue(napi_env env, const std::vector<double>& values)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, values.size(), &result));

    for (size_t i = 0; i < values.size(); ++i) {
        napi_value element = nullptr;
        NAPI_CALL(env, napi_create_double(env, values[i], &element));
        NAPI_CALL(env, napi_set_element(env, result, static_cast<uint32_t>(i), element));
    }
    return result;
}

napi_value CreateDoublePairValue(napi_env env, double first, double second)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array(env, &result));

    napi_value firstValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, first, &firstValue));
    NAPI_CALL(env, napi_set_element(env, result, ARG_INDEX_ZERO, firstValue));

    napi_value secondValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, second, &secondValue));
    NAPI_CALL(env, napi_set_element(env, result, ARG_INDEX_ONE, secondValue));
    return result;
}

napi_value PointDistance(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FOUR;
    napi_value argv[REQUIRED_ARGS_FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FOUR, "Requires 4 arguments: x1, y1, x2, y2");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_FOUR), "All arguments must be numbers");

    double x1 = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double y1 = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double x2 = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double y2 = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);

    double dx = x2 - x1;
    double dy = y2 - y1;
    double result = std::sqrt(dx * dx + dy * dy);
    return CreateNumberValue(env, result);
}

napi_value ManhattanDistance(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FOUR;
    napi_value argv[REQUIRED_ARGS_FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FOUR, "Requires 4 arguments: x1, y1, x2, y2");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_FOUR), "All arguments must be numbers");

    double x1 = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double y1 = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double x2 = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double y2 = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);

    double dx = x2 - x1;
    double dy = y2 - y1;
    double result = std::fabs(dx) + std::fabs(dy);
    return CreateNumberValue(env, result);
}

napi_value ChebyshevDistance(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FOUR;
    napi_value argv[REQUIRED_ARGS_FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FOUR, "Requires 4 arguments: x1, y1, x2, y2");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_FOUR), "All arguments must be numbers");

    double x1 = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double y1 = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double x2 = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double y2 = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);

    double dx = std::fabs(x2 - x1);
    double dy = std::fabs(y2 - y1);
    double result = std::max(dx, dy);
    return CreateNumberValue(env, result);
}

napi_value PointDistance3D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_SIX;
    napi_value argv[REQUIRED_ARGS_SIX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_SIX, "Requires 6 arguments: x1, y1, z1, x2, y2, z2");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_SIX), "All arguments must be numbers");

    double x1 = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double y1 = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double z1 = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double x2 = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);
    double y2 = ExtractDoubleArg(env, argv[ARG_INDEX_FOUR]);
    double z2 = ExtractDoubleArg(env, argv[ARG_INDEX_FIVE]);

    double dx = x2 - x1;
    double dy = y2 - y1;
    double dz = z2 - z1;
    double result = std::sqrt(dx * dx + dy * dy + dz * dz);
    return CreateNumberValue(env, result);
}

napi_value TriangleArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: base, height");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double base = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, base >= 0.0 && height >= 0.0, "Base and height must be non-negative");

    double result = HALF * base * height;
    return CreateNumberValue(env, result);
}

napi_value TriangleAreaHeron(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: a, b, c");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_THREE), "All arguments must be numbers");

    double sideA = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double sideB = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double sideC = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);

    NAPI_ASSERT(env, sideA > 0.0 && sideB > 0.0 && sideC > 0.0, "Side lengths must be positive");

    NAPI_ASSERT(env, sideA + sideB > sideC && sideA + sideC > sideB && sideB + sideC > sideA,
                "Side lengths must satisfy the triangle inequality");

    double semiPerimeter = HALF * (sideA + sideB + sideC);
    double factorA = semiPerimeter - sideA;
    double factorB = semiPerimeter - sideB;
    double factorC = semiPerimeter - sideC;
    double result = std::sqrt(semiPerimeter * factorA * factorB * factorC);
    return CreateNumberValue(env, result);
}

napi_value TrianglePerimeter(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: a, b, c");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_THREE), "All arguments must be numbers");

    double sideA = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double sideB = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double sideC = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);

    NAPI_ASSERT(env, sideA > 0.0 && sideB > 0.0 && sideC > 0.0, "Side lengths must be positive");

    double result = sideA + sideB + sideC;
    return CreateNumberValue(env, result);
}

napi_value RectangleArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: width, height");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double width = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, width > 0.0 && height > 0.0, "Width and height must be positive");

    double result = width * height;
    return CreateNumberValue(env, result);
}

napi_value RectanglePerimeter(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: width, height");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double width = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, width > 0.0 && height > 0.0, "Width and height must be positive");

    double result = TWO * (width + height);
    return CreateNumberValue(env, result);
}

napi_value CircleArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: radius");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_ONE), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);

    NAPI_ASSERT(env, radius >= 0.0, "Radius must be non-negative");

    double radiusSquared = radius * radius;
    double result = GEOMETRY_PI * radiusSquared;
    return CreateNumberValue(env, result);
}

napi_value CircleCircumference(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: radius");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_ONE), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);

    NAPI_ASSERT(env, radius >= 0.0, "Radius must be non-negative");

    double result = TWO * GEOMETRY_PI * radius;
    return CreateNumberValue(env, result);
}

napi_value EllipseArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: semiMajor, semiMinor");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double semiMajor = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double semiMinor = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, semiMajor >= 0.0 && semiMinor >= 0.0, "Semi-axes must be non-negative");

    double result = GEOMETRY_PI * semiMajor * semiMinor;
    return CreateNumberValue(env, result);
}

napi_value EllipsePerimeter(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: semiMajor, semiMinor");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double semiMajor = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double semiMinor = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, semiMajor > 0.0 && semiMinor > 0.0, "Semi-axes must be positive");

    double axisSum = semiMajor + semiMinor;
    double axisDiff = semiMajor - semiMinor;
    double ratio = axisDiff / axisSum;
    double h = ratio * ratio;
    double innerTerm = ELLIPSE_INNER_COEFF - ELLIPSE_INNER_SUB * h;
    double denominator = ELLIPSE_DENOM_CONST + std::sqrt(innerTerm);
    double hFactor = 1.0 + ELLIPSE_NUMERATOR_COEFF * h / denominator;
    double result = GEOMETRY_PI * axisSum * hFactor;
    return CreateNumberValue(env, result);
}

napi_value SphereVolume(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: radius");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_ONE), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);

    NAPI_ASSERT(env, radius >= 0.0, "Radius must be non-negative");

    double radiusCubed = radius * radius * radius;
    double result = FOUR * GEOMETRY_PI * radiusCubed / THREE;
    return CreateNumberValue(env, result);
}

napi_value SphereSurfaceArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: radius");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_ONE), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);

    NAPI_ASSERT(env, radius >= 0.0, "Radius must be non-negative");

    double radiusSquared = radius * radius;
    double result = FOUR * GEOMETRY_PI * radiusSquared;
    return CreateNumberValue(env, result);
}

napi_value CylinderVolume(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: radius, height");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, radius >= 0.0 && height >= 0.0, "Radius and height must be non-negative");

    double radiusSquared = radius * radius;
    double result = GEOMETRY_PI * radiusSquared * height;
    return CreateNumberValue(env, result);
}

napi_value CylinderSurfaceArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: radius, height");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, radius >= 0.0 && height >= 0.0, "Radius and height must be non-negative");

    double radiusSquared = radius * radius;
    double baseAreas = TWO * GEOMETRY_PI * radiusSquared;
    double lateralArea = TWO * GEOMETRY_PI * radius * height;
    double result = baseAreas + lateralArea;
    return CreateNumberValue(env, result);
}

napi_value ConeVolume(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: radius, height");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, radius >= 0.0 && height >= 0.0, "Radius and height must be non-negative");

    double radiusSquared = radius * radius;
    double result = GEOMETRY_PI * radiusSquared * height / THREE;
    return CreateNumberValue(env, result);
}

napi_value ConeSurfaceArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: radius, height");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, radius >= 0.0 && height >= 0.0, "Radius and height must be non-negative");

    double radiusSquared = radius * radius;
    double heightSquared = height * height;
    double slantHeight = std::sqrt(radiusSquared + heightSquared);
    double result = GEOMETRY_PI * radius * (radius + slantHeight);
    return CreateNumberValue(env, result);
}

napi_value CubeVolume(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: side");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_ONE), "All arguments must be numbers");

    double side = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);

    NAPI_ASSERT(env, side >= 0.0, "Side must be non-negative");

    double result = side * side * side;
    return CreateNumberValue(env, result);
}

napi_value CubeSurfaceArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: side");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_ONE), "All arguments must be numbers");

    double side = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);

    NAPI_ASSERT(env, side >= 0.0, "Side must be non-negative");

    double sideSquared = side * side;
    double result = SIX * sideSquared;
    return CreateNumberValue(env, result);
}

napi_value DiagonalLength2D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: width, height");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double width = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, width >= 0.0 && height >= 0.0, "Width and height must be non-negative");

    double result = std::sqrt(width * width + height * height);
    return CreateNumberValue(env, result);
}

napi_value DiagonalLength3D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: width, height, depth");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_THREE), "All arguments must be numbers");

    double width = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double depth = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);

    NAPI_ASSERT(env, width >= 0.0 && height >= 0.0 && depth >= 0.0, "Dimensions must be non-negative");

    double result = std::sqrt(width * width + height * height + depth * depth);
    return CreateNumberValue(env, result);
}

napi_value IsTriangleValid(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: a, b, c");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_THREE), "All arguments must be numbers");

    double sideA = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double sideB = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double sideC = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);

    bool result = sideA > 0.0 && sideB > 0.0 && sideC > 0.0 &&
                  sideA + sideB > sideC && sideA + sideC > sideB && sideB + sideC > sideA;
    return CreateBooleanValue(env, result);
}

napi_value IncircleRadius(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: a, b, c");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_THREE), "All arguments must be numbers");

    double sideA = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double sideB = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double sideC = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);

    NAPI_ASSERT(env, sideA > 0.0 && sideB > 0.0 && sideC > 0.0, "Side lengths must be positive");

    NAPI_ASSERT(env, sideA + sideB > sideC && sideA + sideC > sideB && sideB + sideC > sideA,
                "Side lengths must satisfy the triangle inequality");

    double semiPerimeter = HALF * (sideA + sideB + sideC);
    double factorA = semiPerimeter - sideA;
    double factorB = semiPerimeter - sideB;
    double factorC = semiPerimeter - sideC;
    double area = std::sqrt(semiPerimeter * factorA * factorB * factorC);
    double result = area / semiPerimeter;
    return CreateNumberValue(env, result);
}

napi_value Circumradius(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: a, b, c");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_THREE), "All arguments must be numbers");

    double sideA = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double sideB = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double sideC = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);

    NAPI_ASSERT(env, sideA > 0.0 && sideB > 0.0 && sideC > 0.0, "Side lengths must be positive");

    NAPI_ASSERT(env, sideA + sideB > sideC && sideA + sideC > sideB && sideB + sideC > sideA,
                "Side lengths must satisfy the triangle inequality");

    double semiPerimeter = HALF * (sideA + sideB + sideC);
    double factorA = semiPerimeter - sideA;
    double factorB = semiPerimeter - sideB;
    double factorC = semiPerimeter - sideC;
    double area = std::sqrt(semiPerimeter * factorA * factorB * factorC);
    double result = sideA * sideB * sideC / (FOUR * area);
    return CreateNumberValue(env, result);
}

napi_value SectorArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: radius, angle");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double angle = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, radius >= 0.0 && angle >= 0.0, "Radius and angle must be non-negative");

    double radiusSquared = radius * radius;
    double result = HALF * radiusSquared * angle;
    return CreateNumberValue(env, result);
}

napi_value ArcLength(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: radius, angle");

    NAPI_ASSERT(env, ValidateNumberArgs(env, argv, REQUIRED_ARGS_TWO), "All arguments must be numbers");

    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double angle = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, radius >= 0.0 && angle >= 0.0, "Radius and angle must be non-negative");

    double result = radius * angle;
    return CreateNumberValue(env, result);
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("pointDistance", PointDistance),
        DECLARE_NAPI_FUNCTION("manhattanDistance", ManhattanDistance),
        DECLARE_NAPI_FUNCTION("chebyshevDistance", ChebyshevDistance),
        DECLARE_NAPI_FUNCTION("pointDistance3D", PointDistance3D),
        DECLARE_NAPI_FUNCTION("triangleArea", TriangleArea),
        DECLARE_NAPI_FUNCTION("triangleAreaHeron", TriangleAreaHeron),
        DECLARE_NAPI_FUNCTION("trianglePerimeter", TrianglePerimeter),
        DECLARE_NAPI_FUNCTION("rectangleArea", RectangleArea),
        DECLARE_NAPI_FUNCTION("rectanglePerimeter", RectanglePerimeter),
        DECLARE_NAPI_FUNCTION("circleArea", CircleArea),
        DECLARE_NAPI_FUNCTION("circleCircumference", CircleCircumference),
        DECLARE_NAPI_FUNCTION("ellipseArea", EllipseArea),
        DECLARE_NAPI_FUNCTION("ellipsePerimeter", EllipsePerimeter),
        DECLARE_NAPI_FUNCTION("sphereVolume", SphereVolume),
        DECLARE_NAPI_FUNCTION("sphereSurfaceArea", SphereSurfaceArea),
        DECLARE_NAPI_FUNCTION("cylinderVolume", CylinderVolume),
        DECLARE_NAPI_FUNCTION("cylinderSurfaceArea", CylinderSurfaceArea),
        DECLARE_NAPI_FUNCTION("coneVolume", ConeVolume),
        DECLARE_NAPI_FUNCTION("coneSurfaceArea", ConeSurfaceArea),
        DECLARE_NAPI_FUNCTION("cubeVolume", CubeVolume),
        DECLARE_NAPI_FUNCTION("cubeSurfaceArea", CubeSurfaceArea),
        DECLARE_NAPI_FUNCTION("diagonalLength2D", DiagonalLength2D),
        DECLARE_NAPI_FUNCTION("diagonalLength3D", DiagonalLength3D),
        DECLARE_NAPI_FUNCTION("isTriangleValid", IsTriangleValid),
        DECLARE_NAPI_FUNCTION("incircleRadius", IncircleRadius),
        DECLARE_NAPI_FUNCTION("circumradius", Circumradius),
        DECLARE_NAPI_FUNCTION("sectorArea", SectorArea),
        DECLARE_NAPI_FUNCTION("arcLength", ArcLength),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
        sizeof(desc) / sizeof(desc[0]), desc));
    NAPI_CALL_BASE(env, RegisterGeometryOpsFunctions(env, exports), nullptr);
    return exports;
}

static napi_module geometrySuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "geometrySuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void GeometrySuiteRegisterModule(void)
{
    napi_module_register(&geometrySuiteModule);
}
