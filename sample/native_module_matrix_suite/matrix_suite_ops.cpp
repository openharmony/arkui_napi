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

#include "napi/native_node_api.h"

#include "matrix_suite_common.h"

napi_value Determinant2x2(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    NAPI_ASSERT(env, GetRowCount(matrix) == SQUARED_MATRIX_SIZE &&
                GetColumnCount(matrix) == SQUARED_MATRIX_SIZE, "Matrix must be 2x2");
    double determinant = matrix[ARG_INDEX_ZERO][ARG_INDEX_ZERO] * matrix[ARG_INDEX_ONE][ARG_INDEX_ONE] -
                         matrix[ARG_INDEX_ZERO][ARG_INDEX_ONE] * matrix[ARG_INDEX_ONE][ARG_INDEX_ZERO];
    return CreateDoubleValue(env, determinant);
}

napi_value Determinant3x3(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    NAPI_ASSERT(env, GetRowCount(matrix) == CUBED_MATRIX_SIZE &&
                GetColumnCount(matrix) == CUBED_MATRIX_SIZE, "Matrix must be 3x3");
    double determinant = ZERO_ELEMENT;
    for (uint32_t col = 0; col < CUBED_MATRIX_SIZE; ++col) {
        uint32_t colNext = (col + COFACTOR_STEP_ONE) % CUBED_MATRIX_SIZE;
        uint32_t colNextNext = (col + COFACTOR_STEP_TWO) % CUBED_MATRIX_SIZE;
        double minor = matrix[MINOR_ROW_ONE][colNext] * matrix[MINOR_ROW_TWO][colNextNext] -
                       matrix[MINOR_ROW_ONE][colNextNext] * matrix[MINOR_ROW_TWO][colNext];
        double sign = (col % EVEN_INDEX_MODULUS == 0) ? DETERMINANT_SIGN_PLUS : DETERMINANT_SIGN_MINUS;
        determinant += sign * matrix[ARG_INDEX_ZERO][col] * minor;
    }
    return CreateDoubleValue(env, determinant);
}

napi_value Inverse2x2(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    NAPI_ASSERT(env, GetRowCount(matrix) == SQUARED_MATRIX_SIZE &&
                GetColumnCount(matrix) == SQUARED_MATRIX_SIZE, "Matrix must be 2x2");
    double determinant = matrix[ARG_INDEX_ZERO][ARG_INDEX_ZERO] * matrix[ARG_INDEX_ONE][ARG_INDEX_ONE] -
                         matrix[ARG_INDEX_ZERO][ARG_INDEX_ONE] * matrix[ARG_INDEX_ONE][ARG_INDEX_ZERO];
    NAPI_ASSERT(env, std::fabs(determinant) > MATRIX_EPSILON, "Matrix is singular");
    double inverseDeterminant = SCALAR_ONE / determinant;
    Matrix result(SQUARED_MATRIX_SIZE, Vector(SQUARED_MATRIX_SIZE, ZERO_ELEMENT));
    result[ARG_INDEX_ZERO][ARG_INDEX_ZERO] = inverseDeterminant * matrix[ARG_INDEX_ONE][ARG_INDEX_ONE];
    result[ARG_INDEX_ZERO][ARG_INDEX_ONE] = DETERMINANT_SIGN_MINUS * inverseDeterminant *
                                            matrix[ARG_INDEX_ZERO][ARG_INDEX_ONE];
    result[ARG_INDEX_ONE][ARG_INDEX_ZERO] = DETERMINANT_SIGN_MINUS * inverseDeterminant *
                                            matrix[ARG_INDEX_ONE][ARG_INDEX_ZERO];
    result[ARG_INDEX_ONE][ARG_INDEX_ONE] = inverseDeterminant * matrix[ARG_INDEX_ZERO][ARG_INDEX_ZERO];
    return CreateMatrixValue(env, result);
}

napi_value MatrixVectorMultiply(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: matrix, vector");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    Vector vector;
    NAPI_CALL_BASE(env, ExtractVector(env, argv[ARG_INDEX_ONE], vector), nullptr);
    NAPI_ASSERT(env, GetColumnCount(matrix) == vector.size(), "Columns must match vector length");
    Vector result(GetRowCount(matrix), ZERO_ELEMENT);
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        double sum = ZERO_ELEMENT;
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            sum += matrix[i][j] * vector[j];
        }
        result[i] = sum;
    }
    return CreateVectorValue(env, result);
}

napi_value VectorMatrixMultiply(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: vector, matrix");
    Vector vector;
    NAPI_CALL_BASE(env, ExtractVector(env, argv[ARG_INDEX_ZERO], vector), nullptr);
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ONE], matrix), nullptr);
    NAPI_ASSERT(env, vector.size() == GetRowCount(matrix), "Vector length must match rows");
    Vector result(GetColumnCount(matrix), ZERO_ELEMENT);
    for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
        double sum = ZERO_ELEMENT;
        for (size_t i = 0; i < GetRowCount(matrix); ++i) {
            sum += vector[i] * matrix[i][j];
        }
        result[j] = sum;
    }
    return CreateVectorValue(env, result);
}

napi_value Equals(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: matrix1, matrix2");
    Matrix left;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], left), nullptr);
    Matrix right;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ONE], right), nullptr);
    bool result = HasSameShape(left, right);
    for (size_t i = 0; result && i < GetRowCount(left); ++i) {
        for (size_t j = 0; result && j < GetColumnCount(left); ++j) {
            result = std::fabs(left[i][j] - right[i][j]) <= MATRIX_EPSILON;
        }
    }
    return CreateBooleanValue(env, result);
}

napi_value Flatten(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    Vector result;
    result.reserve(GetRowCount(matrix) * GetColumnCount(matrix));
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            result.push_back(matrix[i][j]);
        }
    }
    return CreateVectorValue(env, result);
}

napi_value Reshape(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: matrix, rows, cols");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    double rowsValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &rowsValue));
    double colsValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_TWO], &colsValue));
    NAPI_ASSERT(env, rowsValue >= MIN_MATRIX_DIMENSION && rowsValue <= MAX_MATRIX_DIMENSION, "Rows out of range");
    NAPI_ASSERT(env, colsValue >= MIN_MATRIX_DIMENSION && colsValue <= MAX_MATRIX_DIMENSION, "Cols out of range");
    size_t newRows = static_cast<size_t>(rowsValue);
    size_t newCols = static_cast<size_t>(colsValue);
    size_t total = GetRowCount(matrix) * GetColumnCount(matrix);
    NAPI_ASSERT(env, newRows * newCols == total, "Element count mismatch");
    Vector flat;
    flat.reserve(total);
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            flat.push_back(matrix[i][j]);
        }
    }
    Matrix result(newRows, Vector(newCols, ZERO_ELEMENT));
    for (size_t i = 0; i < newRows; ++i) {
        for (size_t j = 0; j < newCols; ++j) {
            result[i][j] = flat[i * newCols + j];
        }
    }
    return CreateMatrixValue(env, result);
}

napi_value SwapRows(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: matrix, row1, row2");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    double firstValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &firstValue));
    double secondValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_TWO], &secondValue));
    uint32_t first = static_cast<uint32_t>(firstValue);
    uint32_t second = static_cast<uint32_t>(secondValue);
    NAPI_ASSERT(env, first < GetRowCount(matrix) && second < GetRowCount(matrix), "Row index out of range");
    Vector temp = matrix[first];
    matrix[first] = matrix[second];
    matrix[second] = temp;
    return CreateMatrixValue(env, matrix);
}

napi_value SwapColumns(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: matrix, col1, col2");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    double firstValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &firstValue));
    double secondValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_TWO], &secondValue));
    uint32_t first = static_cast<uint32_t>(firstValue);
    uint32_t second = static_cast<uint32_t>(secondValue);
    NAPI_ASSERT(env, first < GetColumnCount(matrix) && second < GetColumnCount(matrix), "Column index out of range");
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        double temp = matrix[i][first];
        matrix[i][first] = matrix[i][second];
        matrix[i][second] = temp;
    }
    return CreateMatrixValue(env, matrix);
}

napi_value ScaleRow(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: matrix, index, factor");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    double indexValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &indexValue));
    double factor = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_TWO], &factor));
    uint32_t index = static_cast<uint32_t>(indexValue);
    NAPI_ASSERT(env, index < GetRowCount(matrix), "Row index out of range");
    for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
        matrix[index][j] *= factor;
    }
    return CreateMatrixValue(env, matrix);
}

napi_value AddRowMultiple(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_FOUR;
    napi_value argv[REQUIRED_ARGS_FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_FOUR, "Requires 4 arguments: matrix, target, source, factor");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    double targetValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &targetValue));
    double sourceValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_TWO], &sourceValue));
    double factor = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_THREE], &factor));
    uint32_t target = static_cast<uint32_t>(targetValue);
    uint32_t source = static_cast<uint32_t>(sourceValue);
    NAPI_ASSERT(env, target < GetRowCount(matrix) && source < GetRowCount(matrix), "Row index out of range");
    for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
        matrix[target][j] += factor * matrix[source][j];
    }
    return CreateMatrixValue(env, matrix);
}

napi_value IsDiagonal(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    bool result = GetRowCount(matrix) == GetColumnCount(matrix);
    for (size_t i = 0; result && i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; result && j < GetColumnCount(matrix); ++j) {
            result = (i == j) || std::fabs(matrix[i][j]) <= MATRIX_EPSILON;
        }
    }
    return CreateBooleanValue(env, result);
}

napi_value IsUpperTriangular(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    bool result = GetRowCount(matrix) == GetColumnCount(matrix);
    for (size_t i = 0; result && i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; result && j < i; ++j) {
            result = std::fabs(matrix[i][j]) <= MATRIX_EPSILON;
        }
    }
    return CreateBooleanValue(env, result);
}

napi_value IsLowerTriangular(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    bool result = GetRowCount(matrix) == GetColumnCount(matrix);
    for (size_t i = 0; result && i < GetRowCount(matrix); ++i) {
        for (size_t j = i + 1; result && j < GetColumnCount(matrix); ++j) {
            result = std::fabs(matrix[i][j]) <= MATRIX_EPSILON;
        }
    }
    return CreateBooleanValue(env, result);
}

napi_value IsOrthogonal(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    bool result = GetRowCount(matrix) == GetColumnCount(matrix);
    for (size_t i = 0; result && i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; result && j < GetColumnCount(matrix); ++j) {
            double sum = ZERO_ELEMENT;
            for (size_t k = 0; k < GetRowCount(matrix); ++k) {
                sum += matrix[i][k] * matrix[j][k];
            }
            double expected = (i == j) ? IDENTITY_DIAGONAL : ZERO_ELEMENT;
            result = std::fabs(sum - expected) <= MATRIX_EPSILON;
        }
    }
    return CreateBooleanValue(env, result);
}

napi_value IsRowStochastic(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    bool result = GetRowCount(matrix) == GetColumnCount(matrix);
    for (size_t i = 0; result && i < GetRowCount(matrix); ++i) {
        double sum = ZERO_ELEMENT;
        for (size_t j = 0; result && j < GetColumnCount(matrix); ++j) {
            result = matrix[i][j] >= ZERO_ELEMENT;
            sum += matrix[i][j];
        }
        result = result && std::fabs(sum - STOCHASTIC_ROW_SUM) <= MATRIX_EPSILON;
    }
    return CreateBooleanValue(env, result);
}

napi_value RowMean(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    Vector result(GetRowCount(matrix), ZERO_ELEMENT);
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        double sum = ZERO_ELEMENT;
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            sum += matrix[i][j];
        }
        result[i] = sum / static_cast<double>(GetColumnCount(matrix));
    }
    return CreateVectorValue(env, result);
}

napi_value ColumnMean(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    Vector result(GetColumnCount(matrix), ZERO_ELEMENT);
    for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
        double sum = ZERO_ELEMENT;
        for (size_t i = 0; i < GetRowCount(matrix); ++i) {
            sum += matrix[i][j];
        }
        result[j] = sum / static_cast<double>(GetRowCount(matrix));
    }
    return CreateVectorValue(env, result);
}

napi_value Negate(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    Matrix result(GetRowCount(matrix), Vector(GetColumnCount(matrix), ZERO_ELEMENT));
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            result[i][j] = -matrix[i][j];
        }
    }
    return CreateMatrixValue(env, result);
}

napi_value AbsMatrix(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    Matrix result(GetRowCount(matrix), Vector(GetColumnCount(matrix), ZERO_ELEMENT));
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            result[i][j] = std::fabs(matrix[i][j]);
        }
    }
    return CreateMatrixValue(env, result);
}

napi_value SumAllElements(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");
    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);
    double sum = ZERO_ELEMENT;
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            sum += matrix[i][j];
        }
    }
    return CreateDoubleValue(env, sum);
}

napi_status RegisterMatrixOpsFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("determinant2x2", Determinant2x2),
        DECLARE_NAPI_FUNCTION("determinant3x3", Determinant3x3),
        DECLARE_NAPI_FUNCTION("inverse2x2", Inverse2x2),
        DECLARE_NAPI_FUNCTION("matrixVectorMultiply", MatrixVectorMultiply),
        DECLARE_NAPI_FUNCTION("vectorMatrixMultiply", VectorMatrixMultiply),
        DECLARE_NAPI_FUNCTION("equals", Equals),
        DECLARE_NAPI_FUNCTION("flatten", Flatten),
        DECLARE_NAPI_FUNCTION("reshape", Reshape),
        DECLARE_NAPI_FUNCTION("swapRows", SwapRows),
        DECLARE_NAPI_FUNCTION("swapColumns", SwapColumns),
        DECLARE_NAPI_FUNCTION("scaleRow", ScaleRow),
        DECLARE_NAPI_FUNCTION("addRowMultiple", AddRowMultiple),
        DECLARE_NAPI_FUNCTION("isDiagonal", IsDiagonal),
        DECLARE_NAPI_FUNCTION("isUpperTriangular", IsUpperTriangular),
        DECLARE_NAPI_FUNCTION("isLowerTriangular", IsLowerTriangular),
        DECLARE_NAPI_FUNCTION("isOrthogonal", IsOrthogonal),
        DECLARE_NAPI_FUNCTION("isRowStochastic", IsRowStochastic),
        DECLARE_NAPI_FUNCTION("rowMean", RowMean),
        DECLARE_NAPI_FUNCTION("columnMean", ColumnMean),
        DECLARE_NAPI_FUNCTION("negate", Negate),
        DECLARE_NAPI_FUNCTION("absMatrix", AbsMatrix),
        DECLARE_NAPI_FUNCTION("sumAllElements", SumAllElements),
    };
    NAPI_CALL_BASE(env, napi_define_properties(env, exports,
                                         sizeof(desc) / sizeof(desc[0]), desc), napi_invalid_arg);
    return napi_ok;
}
