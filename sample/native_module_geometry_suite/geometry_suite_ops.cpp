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

napi_value PolygonArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: points");

    std::vector<double> points;
    napi_status status = ExtractPointArray(env, argv[ARG_INDEX_ZERO], points);
    NAPI_ASSERT(env, status == napi_ok, "Points must be a flat array of numbers");

    NAPI_ASSERT(env, points.size() >= static_cast<size_t>(MIN_POLYGON_COORDS),
                "Polygon needs at least 3 vertices");

    size_t count = points.size();
    double areaTwice = 0.0;
    for (size_t i = 0; i < count; i += COORD_STRIDE) {
        size_t next = i + COORD_STRIDE;
        if (next >= count) {
            next = 0;
        }
        double cross = points[i] * points[next + 1] - points[next] * points[i + 1];
        areaTwice += cross;
    }

    double result = HALF * std::fabs(areaTwice);
    return CreateNumberValue(env, result);
}

napi_value PolygonPerimeter(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: points");

    std::vector<double> points;
    napi_status status = ExtractPointArray(env, argv[ARG_INDEX_ZERO], points);
    NAPI_ASSERT(env, status == napi_ok, "Points must be a flat array of numbers");

    NAPI_ASSERT(env, points.size() >= static_cast<size_t>(MIN_POLYGON_COORDS),
                "Polygon needs at least 3 vertices");

    size_t count = points.size();
    double perimeter = 0.0;
    for (size_t i = 0; i < count; i += COORD_STRIDE) {
        size_t next = i + COORD_STRIDE;
        if (next >= count) {
            next = 0;
        }
        double dx = points[next] - points[i];
        double dy = points[next + 1] - points[i + 1];
        perimeter += std::sqrt(dx * dx + dy * dy);
    }
    return CreateNumberValue(env, perimeter);
}

napi_value LineSlope(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FOUR;
    napi_value argv[REQUIRED_ARGS_FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FOUR, "Requires 4 arguments: x1, y1, x2, y2");

    double x1 = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double y1 = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double x2 = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double y2 = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);

    NAPI_ASSERT(env, std::fabs(x2 - x1) > GEOMETRY_EPSILON, "Line must not be vertical");

    double result = (y2 - y1) / (x2 - x1);
    return CreateNumberValue(env, result);
}

napi_value LineMidpoint(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FOUR;
    napi_value argv[REQUIRED_ARGS_FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FOUR, "Requires 4 arguments: x1, y1, x2, y2");

    double x1 = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double y1 = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double x2 = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double y2 = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);

    double midpointX = HALF * (x1 + x2);
    double midpointY = HALF * (y1 + y2);
    return CreateDoublePairValue(env, midpointX, midpointY);
}

napi_value PointToLineDistance(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_SIX;
    napi_value argv[REQUIRED_ARGS_SIX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_SIX, "Requires 6 arguments: px, py, x1, y1, x2, y2");

    double px = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double py = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double x1 = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double y1 = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);
    double x2 = ExtractDoubleArg(env, argv[ARG_INDEX_FOUR]);
    double y2 = ExtractDoubleArg(env, argv[ARG_INDEX_FIVE]);

    double dx = x2 - x1;
    double dy = y2 - y1;
    double length = std::sqrt(dx * dx + dy * dy);
    if (length == 0.0 || length <= GEOMETRY_EPSILON) {
        NAPI_CALL(env, napi_throw_error(env, nullptr, "Line segment must have positive length"));
        return nullptr;
    }

    double numerator = std::fabs(dx * (y1 - py) - (x1 - px) * dy);
    double result = numerator / length;
    return CreateNumberValue(env, result);
}

napi_value DotProduct2D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FOUR;
    napi_value argv[REQUIRED_ARGS_FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FOUR, "Requires 4 arguments: vx, vy, wx, wy");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double wx = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double wy = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);

    double result = vx * wx + vy * wy;
    return CreateNumberValue(env, result);
}

napi_value DotProduct3D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_SIX;
    napi_value argv[REQUIRED_ARGS_SIX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_SIX, "Requires 6 arguments: vx, vy, vz, wx, wy, wz");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double vz = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double wx = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);
    double wy = ExtractDoubleArg(env, argv[ARG_INDEX_FOUR]);
    double wz = ExtractDoubleArg(env, argv[ARG_INDEX_FIVE]);

    double result = vx * wx + vy * wy + vz * wz;
    return CreateNumberValue(env, result);
}

napi_value CrossProduct2D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FOUR;
    napi_value argv[REQUIRED_ARGS_FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FOUR, "Requires 4 arguments: vx, vy, wx, wy");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double wx = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double wy = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);

    double result = vx * wy - vy * wx;
    return CreateNumberValue(env, result);
}

napi_value CrossProduct3D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_SIX;
    napi_value argv[REQUIRED_ARGS_SIX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_SIX, "Requires 6 arguments: vx, vy, vz, wx, wy, wz");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double vz = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double wx = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);
    double wy = ExtractDoubleArg(env, argv[ARG_INDEX_FOUR]);
    double wz = ExtractDoubleArg(env, argv[ARG_INDEX_FIVE]);

    double cx = vy * wz - vz * wy;
    double cy = vz * wx - vx * wz;
    double cz = vx * wy - vy * wx;
    std::vector<double> components = { cx, cy, cz };
    return CreateDoubleArrayValue(env, components);
}

napi_value VectorMagnitude2D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: vx, vy");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    double result = std::sqrt(vx * vx + vy * vy);
    return CreateNumberValue(env, result);
}

napi_value VectorMagnitude3D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: vx, vy, vz");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double vz = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);

    double result = std::sqrt(vx * vx + vy * vy + vz * vz);
    return CreateNumberValue(env, result);
}

napi_value VectorNormalize2D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: vx, vy");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    double magnitude = std::sqrt(vx * vx + vy * vy);
    if (magnitude == 0.0 || magnitude <= GEOMETRY_EPSILON) {
        NAPI_CALL(env, napi_throw_error(env, nullptr, "Vector magnitude must be positive"));
        return nullptr;
    }

    return CreateDoublePairValue(env, vx / magnitude, vy / magnitude);
}

napi_value VectorNormalize3D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: vx, vy, vz");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double vz = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);

    double magnitude = std::sqrt(vx * vx + vy * vy + vz * vz);
    if (magnitude == 0.0 || magnitude <= GEOMETRY_EPSILON) {
        NAPI_CALL(env, napi_throw_error(env, nullptr, "Vector magnitude must be positive"));
        return nullptr;
    }

    std::vector<double> unit = { vx / magnitude, vy / magnitude, vz / magnitude };
    return CreateDoubleArrayValue(env, unit);
}

napi_value AngleBetweenVectors2D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FOUR;
    napi_value argv[REQUIRED_ARGS_FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FOUR, "Requires 4 arguments: vx, vy, wx, wy");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double wx = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double wy = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);

    double dot = vx * wx + vy * wy;
    double magnitudeV = std::sqrt(vx * vx + vy * vy);
    double magnitudeW = std::sqrt(wx * wx + wy * wy);
    NAPI_ASSERT(env, magnitudeV > GEOMETRY_EPSILON && magnitudeW > GEOMETRY_EPSILON,
                "Vectors must have positive magnitude");

    double cosine = dot / (magnitudeV * magnitudeW);
    double clamped = std::max(ACOS_MIN_COSINE, std::min(ACOS_MAX_COSINE, cosine));
    double result = std::acos(clamped);
    return CreateNumberValue(env, result);
}

napi_value AngleBetweenVectors3D(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_SIX;
    napi_value argv[REQUIRED_ARGS_SIX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_SIX, "Requires 6 arguments: vx, vy, vz, wx, wy, wz");

    double vx = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vy = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double vz = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double wx = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);
    double wy = ExtractDoubleArg(env, argv[ARG_INDEX_FOUR]);
    double wz = ExtractDoubleArg(env, argv[ARG_INDEX_FIVE]);

    double dot = vx * wx + vy * wy + vz * wz;
    double magnitudeV = std::sqrt(vx * vx + vy * vy + vz * vz);
    double magnitudeW = std::sqrt(wx * wx + wy * wy + wz * wz);
    NAPI_ASSERT(env, magnitudeV > GEOMETRY_EPSILON && magnitudeW > GEOMETRY_EPSILON,
                "Vectors must have positive magnitude");

    double cosine = dot / (magnitudeV * magnitudeW);
    double clamped = std::max(ACOS_MIN_COSINE, std::min(ACOS_MAX_COSINE, cosine));
    double result = std::acos(clamped);
    return CreateNumberValue(env, result);
}

napi_value IsPointInCircle(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FIVE;
    napi_value argv[REQUIRED_ARGS_FIVE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FIVE, "Requires 5 arguments: px, py, cx, cy, r");

    double px = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double py = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double cx = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double cy = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);
    double radius = ExtractDoubleArg(env, argv[ARG_INDEX_FOUR]);

    NAPI_ASSERT(env, radius >= 0.0, "Radius must be non-negative");

    double dx = px - cx;
    double dy = py - cy;
    bool result = dx * dx + dy * dy <= radius * radius;
    return CreateBooleanValue(env, result);
}

napi_value IsPointInRectangle(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_SIX;
    napi_value argv[REQUIRED_ARGS_SIX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_SIX, "Requires 6 arguments: px, py, x, y, w, h");

    double px = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double py = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);
    double x = ExtractDoubleArg(env, argv[ARG_INDEX_TWO]);
    double y = ExtractDoubleArg(env, argv[ARG_INDEX_THREE]);
    double width = ExtractDoubleArg(env, argv[ARG_INDEX_FOUR]);
    double height = ExtractDoubleArg(env, argv[ARG_INDEX_FIVE]);

    NAPI_ASSERT(env, width >= 0.0 && height >= 0.0, "Width and height must be non-negative");

    bool result = px >= x && px <= x + width && py >= y && py <= y + height;
    return CreateBooleanValue(env, result);
}

napi_value PolygonCentroid(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: points");

    std::vector<double> points;
    napi_status status = ExtractPointArray(env, argv[ARG_INDEX_ZERO], points);
    NAPI_ASSERT(env, status == napi_ok, "Points must be a flat array of numbers");

    NAPI_ASSERT(env, points.size() >= static_cast<size_t>(MIN_POLYGON_COORDS),
                "Polygon needs at least 3 vertices");

    size_t count = points.size();
    double signedAreaTwice = 0.0;
    double sumX = 0.0;
    double sumY = 0.0;
    for (size_t i = 0; i < count; i += COORD_STRIDE) {
        size_t next = i + COORD_STRIDE;
        if (next >= count) {
            next = 0;
        }
        double cross = points[i] * points[next + 1] - points[next] * points[i + 1];
        signedAreaTwice += cross;
        sumX += (points[i] + points[next]) * cross;
        sumY += (points[i + 1] + points[next + 1]) * cross;
    }

    NAPI_ASSERT(env, std::fabs(signedAreaTwice) > GEOMETRY_EPSILON, "Polygon area must be non-zero");

    double centroidX = sumX / (THREE * signedAreaTwice);
    double centroidY = sumY / (THREE * signedAreaTwice);
    return CreateDoublePairValue(env, centroidX, centroidY);
}

napi_value RegularPolygonArea(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: side, vertices");

    double side = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vertices = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, std::fmod(vertices, 1.0) == 0.0, "Vertices must be an integer");

    if (vertices == 0.0 || vertices < static_cast<double>(MIN_POLYGON_VERTICES)) {
        NAPI_CALL(env, napi_throw_error(env, nullptr, "Polygon needs at least 3 vertices"));
        return nullptr;
    }

    double tangent = std::tan(GEOMETRY_PI / vertices);
    double result = vertices * side * side / (FOUR * tangent);
    return CreateNumberValue(env, result);
}

napi_value RegularPolygonPerimeter(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: side, vertices");

    double side = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double vertices = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, std::fmod(vertices, 1.0) == 0.0, "Vertices must be an integer");

    NAPI_ASSERT(env, vertices >= static_cast<double>(MIN_POLYGON_VERTICES),
                "Polygon needs at least 3 vertices");

    double result = vertices * side;
    return CreateNumberValue(env, result);
}

napi_value QuadrantOf(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: x, y");

    double x = ExtractDoubleArg(env, argv[ARG_INDEX_ZERO]);
    double y = ExtractDoubleArg(env, argv[ARG_INDEX_ONE]);

    double quadrant = static_cast<double>(QUADRANT_ONE);
    if (x < 0.0 && y >= 0.0) {
        quadrant = static_cast<double>(QUADRANT_TWO);
    } else if (x < 0.0 && y < 0.0) {
        quadrant = static_cast<double>(QUADRANT_THREE);
    } else if (x >= 0.0 && y < 0.0) {
        quadrant = static_cast<double>(QUADRANT_FOUR);
    }
    return CreateNumberValue(env, quadrant);
}

napi_status RegisterGeometryOpsFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("polygonArea", PolygonArea),
        DECLARE_NAPI_FUNCTION("polygonPerimeter", PolygonPerimeter),
        DECLARE_NAPI_FUNCTION("lineSlope", LineSlope),
        DECLARE_NAPI_FUNCTION("lineMidpoint", LineMidpoint),
        DECLARE_NAPI_FUNCTION("pointToLineDistance", PointToLineDistance),
        DECLARE_NAPI_FUNCTION("dotProduct2D", DotProduct2D),
        DECLARE_NAPI_FUNCTION("dotProduct3D", DotProduct3D),
        DECLARE_NAPI_FUNCTION("crossProduct2D", CrossProduct2D),
        DECLARE_NAPI_FUNCTION("crossProduct3D", CrossProduct3D),
        DECLARE_NAPI_FUNCTION("vectorMagnitude2D", VectorMagnitude2D),
        DECLARE_NAPI_FUNCTION("vectorMagnitude3D", VectorMagnitude3D),
        DECLARE_NAPI_FUNCTION("vectorNormalize2D", VectorNormalize2D),
        DECLARE_NAPI_FUNCTION("vectorNormalize3D", VectorNormalize3D),
        DECLARE_NAPI_FUNCTION("angleBetweenVectors2D", AngleBetweenVectors2D),
        DECLARE_NAPI_FUNCTION("angleBetweenVectors3D", AngleBetweenVectors3D),
        DECLARE_NAPI_FUNCTION("isPointInCircle", IsPointInCircle),
        DECLARE_NAPI_FUNCTION("isPointInRectangle", IsPointInRectangle),
        DECLARE_NAPI_FUNCTION("polygonCentroid", PolygonCentroid),
        DECLARE_NAPI_FUNCTION("regularPolygonArea", RegularPolygonArea),
        DECLARE_NAPI_FUNCTION("regularPolygonPerimeter", RegularPolygonPerimeter),
        DECLARE_NAPI_FUNCTION("quadrantOf", QuadrantOf),
    };
    return napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}
