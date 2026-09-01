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

#ifndef SAMPLE_NATIVE_MODULE_MATRIX_SUITE_COMMON_H
#define SAMPLE_NATIVE_MODULE_MATRIX_SUITE_COMMON_H

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <vector>

#include "napi/native_api.h"

using Matrix = std::vector<std::vector<double>>;
using Vector = std::vector<double>;

constexpr int REQUIRED_ARGS_ONE = 1;
constexpr int REQUIRED_ARGS_TWO = 2;
constexpr int REQUIRED_ARGS_THREE = 3;
constexpr int REQUIRED_ARGS_FOUR = 4;
constexpr uint32_t ARG_INDEX_ZERO = 0;
constexpr uint32_t ARG_INDEX_ONE = 1;
constexpr uint32_t ARG_INDEX_TWO = 2;
constexpr uint32_t ARG_INDEX_THREE = 3;
constexpr uint32_t MIN_MATRIX_DIMENSION = 1;
constexpr uint32_t MAX_MATRIX_DIMENSION = 100;
constexpr double IDENTITY_DIAGONAL = 1.0;
constexpr double ZERO_ELEMENT = 0.0;
constexpr uint32_t SQUARED_MATRIX_SIZE = 2;
constexpr uint32_t CUBED_MATRIX_SIZE = 3;
constexpr uint32_t COFACTOR_STEP_ONE = 1;
constexpr uint32_t COFACTOR_STEP_TWO = 2;
constexpr uint32_t MINOR_ROW_ONE = 1;
constexpr uint32_t MINOR_ROW_TWO = 2;
constexpr uint32_t EVEN_INDEX_MODULUS = 2;
constexpr double DETERMINANT_SIGN_MINUS = -1.0;
constexpr double DETERMINANT_SIGN_PLUS = 1.0;
constexpr double MATRIX_EPSILON = 1e-12;
constexpr double STOCHASTIC_ROW_SUM = 1.0;
constexpr double SCALAR_ONE = 1.0;

napi_status ExtractMatrix(napi_env env, napi_value value, Matrix& result);
napi_status ExtractVector(napi_env env, napi_value value, Vector& result);
napi_value CreateMatrixValue(napi_env env, const Matrix& matrix);
napi_value CreateVectorValue(napi_env env, const Vector& vector);
napi_value CreateDoubleValue(napi_env env, double value);
napi_value CreateBooleanValue(napi_env env, bool value);
size_t GetRowCount(const Matrix& matrix);
size_t GetColumnCount(const Matrix& matrix);
bool HasSameShape(const Matrix& left, const Matrix& right);

napi_value Identity(napi_env env, napi_callback_info info);
napi_value Zeros(napi_env env, napi_callback_info info);
napi_value Add(napi_env env, napi_callback_info info);
napi_value Subtract(napi_env env, napi_callback_info info);
napi_value ScalarMultiply(napi_env env, napi_callback_info info);
napi_value Multiply(napi_env env, napi_callback_info info);
napi_value Transpose(napi_env env, napi_callback_info info);
napi_value Trace(napi_env env, napi_callback_info info);
napi_value RowSum(napi_env env, napi_callback_info info);
napi_value ColumnSum(napi_env env, napi_callback_info info);
napi_value IsSquare(napi_env env, napi_callback_info info);
napi_value IsIdentity(napi_env env, napi_callback_info info);
napi_value IsSymmetric(napi_env env, napi_callback_info info);
napi_value IsZero(napi_env env, napi_callback_info info);
napi_value MainDiagonal(napi_env env, napi_callback_info info);
napi_value AntiDiagonal(napi_env env, napi_callback_info info);
napi_value HadamardProduct(napi_env env, napi_callback_info info);
napi_value FrobeniusNorm(napi_env env, napi_callback_info info);
napi_value MaxAbsElement(napi_env env, napi_callback_info info);
napi_value CountNonZero(napi_env env, napi_callback_info info);
napi_value RowAt(napi_env env, napi_callback_info info);
napi_value ColumnAt(napi_env env, napi_callback_info info);

napi_value Determinant2x2(napi_env env, napi_callback_info info);
napi_value Determinant3x3(napi_env env, napi_callback_info info);
napi_value Inverse2x2(napi_env env, napi_callback_info info);
napi_value MatrixVectorMultiply(napi_env env, napi_callback_info info);
napi_value VectorMatrixMultiply(napi_env env, napi_callback_info info);
napi_value Equals(napi_env env, napi_callback_info info);
napi_value Flatten(napi_env env, napi_callback_info info);
napi_value Reshape(napi_env env, napi_callback_info info);
napi_value SwapRows(napi_env env, napi_callback_info info);
napi_value SwapColumns(napi_env env, napi_callback_info info);
napi_value ScaleRow(napi_env env, napi_callback_info info);
napi_value AddRowMultiple(napi_env env, napi_callback_info info);
napi_value IsDiagonal(napi_env env, napi_callback_info info);
napi_value IsUpperTriangular(napi_env env, napi_callback_info info);
napi_value IsLowerTriangular(napi_env env, napi_callback_info info);
napi_value IsOrthogonal(napi_env env, napi_callback_info info);
napi_value IsRowStochastic(napi_env env, napi_callback_info info);
napi_value RowMean(napi_env env, napi_callback_info info);
napi_value ColumnMean(napi_env env, napi_callback_info info);
napi_value Negate(napi_env env, napi_callback_info info);
napi_value AbsMatrix(napi_env env, napi_callback_info info);
napi_value SumAllElements(napi_env env, napi_callback_info info);

napi_status RegisterMatrixOpsFunctions(napi_env env, napi_value exports);

#endif
