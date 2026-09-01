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

#ifndef SAMPLE_NATIVE_MODULE_GEOMETRY_SUITE_COMMON_H
#define SAMPLE_NATIVE_MODULE_GEOMETRY_SUITE_COMMON_H

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <vector>

#include "napi/native_api.h"

constexpr int REQUIRED_ARGS_ONE = 1;
constexpr int REQUIRED_ARGS_TWO = 2;
constexpr int REQUIRED_ARGS_THREE = 3;
constexpr int REQUIRED_ARGS_FOUR = 4;
constexpr int REQUIRED_ARGS_FIVE = 5;
constexpr int REQUIRED_ARGS_SIX = 6;

constexpr int ARG_INDEX_ZERO = 0;
constexpr int ARG_INDEX_ONE = 1;
constexpr int ARG_INDEX_TWO = 2;
constexpr int ARG_INDEX_THREE = 3;
constexpr int ARG_INDEX_FOUR = 4;
constexpr int ARG_INDEX_FIVE = 5;

constexpr double GEOMETRY_PI = 3.14159265358979323846;
constexpr double HALF = 0.5;
constexpr double TWO = 2.0;
constexpr double THREE = 3.0;
constexpr double FOUR = 4.0;
constexpr double SIX = 6.0;
constexpr double ELLIPSE_NUMERATOR_COEFF = 3.0;
constexpr double ELLIPSE_DENOM_CONST = 10.0;
constexpr double ELLIPSE_INNER_COEFF = 4.0;
constexpr double ELLIPSE_INNER_SUB = 3.0;
constexpr double GEOMETRY_EPSILON = 1e-12;
constexpr double ACOS_MIN_COSINE = -1.0;
constexpr double ACOS_MAX_COSINE = 1.0;

constexpr int MIN_POLYGON_VERTICES = 3;
constexpr int EVEN_CHECK = 2;
constexpr int MIN_POLYGON_COORDS = MIN_POLYGON_VERTICES * EVEN_CHECK;
constexpr int QUADRANT_ONE = 1;
constexpr int QUADRANT_TWO = 2;
constexpr int QUADRANT_THREE = 3;
constexpr int QUADRANT_FOUR = 4;
constexpr size_t COORD_STRIDE = 2;

double ExtractDoubleArg(napi_env env, napi_value value);
bool ValidateNumberArgs(napi_env env, const napi_value* argv, size_t count);
napi_status ExtractPointArray(napi_env env, napi_value value, std::vector<double>& points);
napi_value CreateNumberValue(napi_env env, double value);
napi_value CreateBooleanValue(napi_env env, bool value);
napi_value CreateDoubleArrayValue(napi_env env, const std::vector<double>& values);
napi_value CreateDoublePairValue(napi_env env, double first, double second);

napi_value PointDistance(napi_env env, napi_callback_info info);
napi_value ManhattanDistance(napi_env env, napi_callback_info info);
napi_value ChebyshevDistance(napi_env env, napi_callback_info info);
napi_value PointDistance3D(napi_env env, napi_callback_info info);
napi_value TriangleArea(napi_env env, napi_callback_info info);
napi_value TriangleAreaHeron(napi_env env, napi_callback_info info);
napi_value TrianglePerimeter(napi_env env, napi_callback_info info);
napi_value RectangleArea(napi_env env, napi_callback_info info);
napi_value RectanglePerimeter(napi_env env, napi_callback_info info);
napi_value CircleArea(napi_env env, napi_callback_info info);
napi_value CircleCircumference(napi_env env, napi_callback_info info);
napi_value EllipseArea(napi_env env, napi_callback_info info);
napi_value EllipsePerimeter(napi_env env, napi_callback_info info);
napi_value SphereVolume(napi_env env, napi_callback_info info);
napi_value SphereSurfaceArea(napi_env env, napi_callback_info info);
napi_value CylinderVolume(napi_env env, napi_callback_info info);
napi_value CylinderSurfaceArea(napi_env env, napi_callback_info info);
napi_value ConeVolume(napi_env env, napi_callback_info info);
napi_value ConeSurfaceArea(napi_env env, napi_callback_info info);
napi_value CubeVolume(napi_env env, napi_callback_info info);
napi_value CubeSurfaceArea(napi_env env, napi_callback_info info);
napi_value DiagonalLength2D(napi_env env, napi_callback_info info);
napi_value DiagonalLength3D(napi_env env, napi_callback_info info);
napi_value IsTriangleValid(napi_env env, napi_callback_info info);
napi_value IncircleRadius(napi_env env, napi_callback_info info);
napi_value Circumradius(napi_env env, napi_callback_info info);
napi_value SectorArea(napi_env env, napi_callback_info info);
napi_value ArcLength(napi_env env, napi_callback_info info);

napi_value PolygonArea(napi_env env, napi_callback_info info);
napi_value PolygonPerimeter(napi_env env, napi_callback_info info);
napi_value LineSlope(napi_env env, napi_callback_info info);
napi_value LineMidpoint(napi_env env, napi_callback_info info);
napi_value PointToLineDistance(napi_env env, napi_callback_info info);
napi_value DotProduct2D(napi_env env, napi_callback_info info);
napi_value DotProduct3D(napi_env env, napi_callback_info info);
napi_value CrossProduct2D(napi_env env, napi_callback_info info);
napi_value CrossProduct3D(napi_env env, napi_callback_info info);
napi_value VectorMagnitude2D(napi_env env, napi_callback_info info);
napi_value VectorMagnitude3D(napi_env env, napi_callback_info info);
napi_value VectorNormalize2D(napi_env env, napi_callback_info info);
napi_value VectorNormalize3D(napi_env env, napi_callback_info info);
napi_value AngleBetweenVectors2D(napi_env env, napi_callback_info info);
napi_value AngleBetweenVectors3D(napi_env env, napi_callback_info info);
napi_value IsPointInCircle(napi_env env, napi_callback_info info);
napi_value IsPointInRectangle(napi_env env, napi_callback_info info);
napi_value PolygonCentroid(napi_env env, napi_callback_info info);
napi_value RegularPolygonArea(napi_env env, napi_callback_info info);
napi_value RegularPolygonPerimeter(napi_env env, napi_callback_info info);
napi_value QuadrantOf(napi_env env, napi_callback_info info);

napi_status RegisterGeometryOpsFunctions(napi_env env, napi_value exports);

#endif // SAMPLE_NATIVE_MODULE_GEOMETRY_SUITE_COMMON_H
