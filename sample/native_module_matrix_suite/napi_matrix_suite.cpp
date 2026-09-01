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

napi_status ExtractVector(napi_env env, napi_value value, Vector& result)
{
    bool isArray = false;
    napi_status status = napi_is_array(env, value, &isArray);
    if (status != napi_ok || !isArray) {
        return (status == napi_ok) ? napi_invalid_arg : status;
    }

    uint32_t length = 0;
    status = napi_get_array_length(env, value, &length);
    if (status != napi_ok) {
        return status;
    }
    if (length < MIN_MATRIX_DIMENSION || length > MAX_MATRIX_DIMENSION) {
        return napi_invalid_arg;
    }

    result.clear();
    result.reserve(length);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value elementValue = nullptr;
        status = napi_get_element(env, value, i, &elementValue);
        if (status != napi_ok) {
            return status;
        }
        double element = ZERO_ELEMENT;
        status = napi_get_value_double(env, elementValue, &element);
        if (status != napi_ok) {
            return status;
        }
        result.push_back(element);
    }
    return napi_ok;
}

napi_status ExtractMatrix(napi_env env, napi_value value, Matrix& result)
{
    bool isArray = false;
    napi_status status = napi_is_array(env, value, &isArray);
    if (status != napi_ok || !isArray) {
        return (status == napi_ok) ? napi_invalid_arg : status;
    }

    uint32_t rowCount = 0;
    status = napi_get_array_length(env, value, &rowCount);
    if (status != napi_ok) {
        return status;
    }
    if (rowCount < MIN_MATRIX_DIMENSION || rowCount > MAX_MATRIX_DIMENSION) {
        return napi_invalid_arg;
    }

    result.clear();
    result.reserve(rowCount);
    uint32_t columnCount = 0;
    for (uint32_t i = 0; i < rowCount; ++i) {
        napi_value rowValue = nullptr;
        status = napi_get_element(env, value, i, &rowValue);
        if (status != napi_ok) {
            return status;
        }
        Vector row;
        status = ExtractVector(env, rowValue, row);
        if (status != napi_ok) {
            return status;
        }
        if (i == ARG_INDEX_ZERO) {
            columnCount = static_cast<uint32_t>(row.size());
        } else if (row.size() != columnCount) {
            return napi_invalid_arg;
        }
        result.push_back(row);
    }
    return napi_ok;
}

napi_value CreateVectorValue(napi_env env, const Vector& vector)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, vector.size(), &result));
    for (size_t i = 0; i < vector.size(); ++i) {
        napi_value elementValue = nullptr;
        NAPI_CALL(env, napi_create_double(env, vector[i], &elementValue));
        NAPI_CALL(env, napi_set_element(env, result, static_cast<uint32_t>(i), elementValue));
    }
    return result;
}

napi_value CreateMatrixValue(napi_env env, const Matrix& matrix)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, matrix.size(), &result));
    for (size_t i = 0; i < matrix.size(); ++i) {
        napi_value rowValue = CreateVectorValue(env, matrix[i]);
        if (rowValue == nullptr) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, result, static_cast<uint32_t>(i), rowValue));
    }
    return result;
}

napi_value CreateDoubleValue(napi_env env, double value)
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

size_t GetRowCount(const Matrix& matrix)
{
    return matrix.size();
}

size_t GetColumnCount(const Matrix& matrix)
{
    return matrix.empty() ? 0 : matrix.front().size();
}

bool HasSameShape(const Matrix& left, const Matrix& right)
{
    return GetRowCount(left) == GetRowCount(right) &&
           GetColumnCount(left) == GetColumnCount(right);
}

napi_value Identity(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: size");

    double sizeValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &sizeValue));

    NAPI_ASSERT(env, sizeValue >= MIN_MATRIX_DIMENSION && sizeValue <= MAX_MATRIX_DIMENSION,
                "Size must be between 1 and 100");

    size_t size = static_cast<size_t>(sizeValue);
    Matrix result(size, Vector(size, ZERO_ELEMENT));
    for (size_t i = 0; i < size; ++i) {
        result[i][i] = IDENTITY_DIAGONAL;
    }
    return CreateMatrixValue(env, result);
}

napi_value Zeros(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: rows, cols");

    double rowsValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &rowsValue));
    double colsValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &colsValue));

    NAPI_ASSERT(env, rowsValue >= MIN_MATRIX_DIMENSION && rowsValue <= MAX_MATRIX_DIMENSION,
                "Rows must be between 1 and 100");
    NAPI_ASSERT(env, colsValue >= MIN_MATRIX_DIMENSION && colsValue <= MAX_MATRIX_DIMENSION,
                "Cols must be between 1 and 100");

    size_t rows = static_cast<size_t>(rowsValue);
    size_t cols = static_cast<size_t>(colsValue);
    Matrix result(rows, Vector(cols, ZERO_ELEMENT));
    return CreateMatrixValue(env, result);
}

napi_value Add(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: matrix1, matrix2");

    Matrix left;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], left), nullptr);
    Matrix right;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ONE], right), nullptr);

    NAPI_ASSERT(env, HasSameShape(left, right), "Matrices must have the same shape");

    Matrix result(GetRowCount(left), Vector(GetColumnCount(left), ZERO_ELEMENT));
    for (size_t i = 0; i < GetRowCount(left); ++i) {
        for (size_t j = 0; j < GetColumnCount(left); ++j) {
            result[i][j] = left[i][j] + right[i][j];
        }
    }
    return CreateMatrixValue(env, result);
}

napi_value Subtract(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: matrix1, matrix2");

    Matrix left;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], left), nullptr);
    Matrix right;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ONE], right), nullptr);

    NAPI_ASSERT(env, HasSameShape(left, right), "Matrices must have the same shape");

    Matrix result(GetRowCount(left), Vector(GetColumnCount(left), ZERO_ELEMENT));
    for (size_t i = 0; i < GetRowCount(left); ++i) {
        for (size_t j = 0; j < GetColumnCount(left); ++j) {
            result[i][j] = left[i][j] - right[i][j];
        }
    }
    return CreateMatrixValue(env, result);
}

napi_value ScalarMultiply(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: matrix, scalar");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    double scalar = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &scalar));

    Matrix result(GetRowCount(matrix), Vector(GetColumnCount(matrix), ZERO_ELEMENT));
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            result[i][j] = matrix[i][j] * scalar;
        }
    }
    return CreateMatrixValue(env, result);
}

napi_value Multiply(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: matrix1, matrix2");

    Matrix left;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], left), nullptr);
    Matrix right;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ONE], right), nullptr);

    NAPI_ASSERT(env, GetColumnCount(left) == GetRowCount(right), "Columns of first must equal rows of second");

    Matrix result(GetRowCount(left), Vector(GetColumnCount(right), ZERO_ELEMENT));
    for (size_t i = 0; i < GetRowCount(left); ++i) {
        for (size_t j = 0; j < GetColumnCount(right); ++j) {
            double sum = ZERO_ELEMENT;
            for (size_t k = 0; k < GetColumnCount(left); ++k) {
                sum += left[i][k] * right[k][j];
            }
            result[i][j] = sum;
        }
    }
    return CreateMatrixValue(env, result);
}

napi_value Transpose(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    Matrix result(GetColumnCount(matrix), Vector(GetRowCount(matrix), ZERO_ELEMENT));
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            result[j][i] = matrix[i][j];
        }
    }
    return CreateMatrixValue(env, result);
}

napi_value Trace(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    NAPI_ASSERT(env, GetRowCount(matrix) == GetColumnCount(matrix), "Matrix must be square");

    double sum = ZERO_ELEMENT;
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        sum += matrix[i][i];
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, sum, &resultValue));
    return resultValue;
}

napi_value RowSum(napi_env env, napi_callback_info info)
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
        result[i] = sum;
    }
    return CreateVectorValue(env, result);
}

napi_value ColumnSum(napi_env env, napi_callback_info info)
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
        result[j] = sum;
    }
    return CreateVectorValue(env, result);
}

napi_value IsSquare(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    bool result = GetRowCount(matrix) == GetColumnCount(matrix);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &resultValue));
    return resultValue;
}

napi_value IsIdentity(napi_env env, napi_callback_info info)
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
            double expected = (i == j) ? IDENTITY_DIAGONAL : ZERO_ELEMENT;
            result = std::fabs(matrix[i][j] - expected) <= MATRIX_EPSILON;
        }
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &resultValue));
    return resultValue;
}

napi_value IsSymmetric(napi_env env, napi_callback_info info)
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
            result = std::fabs(matrix[i][j] - matrix[j][i]) <= MATRIX_EPSILON;
        }
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &resultValue));
    return resultValue;
}

napi_value IsZero(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    bool result = true;
    for (size_t i = 0; result && i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; result && j < GetColumnCount(matrix); ++j) {
            result = std::fabs(matrix[i][j]) <= MATRIX_EPSILON;
        }
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &resultValue));
    return resultValue;
}

napi_value MainDiagonal(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    NAPI_ASSERT(env, GetRowCount(matrix) == GetColumnCount(matrix), "Matrix must be square");

    Vector result(GetRowCount(matrix), ZERO_ELEMENT);
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        result[i] = matrix[i][i];
    }
    return CreateVectorValue(env, result);
}

napi_value AntiDiagonal(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    NAPI_ASSERT(env, GetRowCount(matrix) == GetColumnCount(matrix), "Matrix must be square");

    Vector result(GetRowCount(matrix), ZERO_ELEMENT);
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        result[i] = matrix[i][GetColumnCount(matrix) - 1 - i];
    }
    return CreateVectorValue(env, result);
}

napi_value HadamardProduct(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: matrix1, matrix2");

    Matrix left;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], left), nullptr);
    Matrix right;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ONE], right), nullptr);

    NAPI_ASSERT(env, HasSameShape(left, right), "Matrices must have the same shape");

    Matrix result(GetRowCount(left), Vector(GetColumnCount(left), ZERO_ELEMENT));
    for (size_t i = 0; i < GetRowCount(left); ++i) {
        for (size_t j = 0; j < GetColumnCount(left); ++j) {
            result[i][j] = left[i][j] * right[i][j];
        }
    }
    return CreateMatrixValue(env, result);
}

napi_value FrobeniusNorm(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    double sumOfSquares = ZERO_ELEMENT;
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            sumOfSquares += matrix[i][j] * matrix[i][j];
        }
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, std::sqrt(sumOfSquares), &resultValue));
    return resultValue;
}

napi_value MaxAbsElement(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    double maxAbs = ZERO_ELEMENT;
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            double elementAbs = std::fabs(matrix[i][j]);
            if (elementAbs > maxAbs) {
                maxAbs = elementAbs;
            }
        }
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, maxAbs, &resultValue));
    return resultValue;
}

napi_value CountNonZero(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: matrix");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    uint32_t count = 0;
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        for (size_t j = 0; j < GetColumnCount(matrix); ++j) {
            if (std::fabs(matrix[i][j]) > MATRIX_EPSILON) {
                ++count;
            }
        }
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, count, &resultValue));
    return resultValue;
}

napi_value RowAt(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: matrix, index");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    double indexValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &indexValue));
    uint32_t index = static_cast<uint32_t>(indexValue);
    NAPI_ASSERT(env, index < GetRowCount(matrix), "Row index out of range");

    return CreateVectorValue(env, matrix[index]);
}

napi_value ColumnAt(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: matrix, index");

    Matrix matrix;
    NAPI_CALL_BASE(env, ExtractMatrix(env, argv[ARG_INDEX_ZERO], matrix), nullptr);

    double indexValue = ZERO_ELEMENT;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &indexValue));
    uint32_t index = static_cast<uint32_t>(indexValue);
    NAPI_ASSERT(env, index < GetColumnCount(matrix), "Column index out of range");

    Vector result(GetRowCount(matrix), ZERO_ELEMENT);
    for (size_t i = 0; i < GetRowCount(matrix); ++i) {
        result[i] = matrix[i][index];
    }
    return CreateVectorValue(env, result);
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("identity", Identity),
        DECLARE_NAPI_FUNCTION("zeros", Zeros),
        DECLARE_NAPI_FUNCTION("add", Add),
        DECLARE_NAPI_FUNCTION("subtract", Subtract),
        DECLARE_NAPI_FUNCTION("scalarMultiply", ScalarMultiply),
        DECLARE_NAPI_FUNCTION("multiply", Multiply),
        DECLARE_NAPI_FUNCTION("transpose", Transpose),
        DECLARE_NAPI_FUNCTION("trace", Trace),
        DECLARE_NAPI_FUNCTION("rowSum", RowSum),
        DECLARE_NAPI_FUNCTION("columnSum", ColumnSum),
        DECLARE_NAPI_FUNCTION("isSquare", IsSquare),
        DECLARE_NAPI_FUNCTION("isIdentity", IsIdentity),
        DECLARE_NAPI_FUNCTION("isSymmetric", IsSymmetric),
        DECLARE_NAPI_FUNCTION("isZero", IsZero),
        DECLARE_NAPI_FUNCTION("mainDiagonal", MainDiagonal),
        DECLARE_NAPI_FUNCTION("antiDiagonal", AntiDiagonal),
        DECLARE_NAPI_FUNCTION("hadamardProduct", HadamardProduct),
        DECLARE_NAPI_FUNCTION("frobeniusNorm", FrobeniusNorm),
        DECLARE_NAPI_FUNCTION("maxAbsElement", MaxAbsElement),
        DECLARE_NAPI_FUNCTION("countNonZero", CountNonZero),
        DECLARE_NAPI_FUNCTION("rowAt", RowAt),
        DECLARE_NAPI_FUNCTION("columnAt", ColumnAt),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    NAPI_CALL_BASE(env, RegisterMatrixOpsFunctions(env, exports), nullptr);
    return exports;
}

static napi_module matrixSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "matrixSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void MatrixSuiteRegisterModule(void)
{
    napi_module_register(&matrixSuiteModule);
}
