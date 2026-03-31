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
#include <cmath>
#include <string>
#include <vector>
#include <stack>
#include <cctype>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr double PI = 3.14159265358979323846;
constexpr double E = 2.71828182845904523536;

static napi_value Add(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double a = 0.0;
    double b = 0.0;
    
    NAPI_CALL(env, napi_get_value_double(env, args[0], &a));
    NAPI_CALL(env, napi_get_value_double(env, args[1], &b));
    
    double result = a + b;
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Subtract(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double a = 0.0;
    double b = 0.0;
    
    NAPI_CALL(env, napi_get_value_double(env, args[0], &a));
    NAPI_CALL(env, napi_get_value_double(env, args[1], &b));
    
    double result = a - b;
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Multiply(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double a = 0.0;
    double b = 0.0;
    
    NAPI_CALL(env, napi_get_value_double(env, args[0], &a));
    NAPI_CALL(env, napi_get_value_double(env, args[1], &b));
    
    double result = a * b;
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Divide(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double a = 0.0;
    double b = 0.0;
    
    NAPI_CALL(env, napi_get_value_double(env, args[0], &a));
    NAPI_CALL(env, napi_get_value_double(env, args[1], &b));
    
    if (b == 0) {
        napi_throw_error(env, "DIVISION_BY_ZERO", "Division by zero");
        return nullptr;
    }
    
    double result = a / b;
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Power(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double base = 0.0;
    double exponent = 0.0;
    
    NAPI_CALL(env, napi_get_value_double(env, args[0], &base));
    NAPI_CALL(env, napi_get_value_double(env, args[1], &exponent));
    
    double result = std::pow(base, exponent);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value SquareRoot(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double value = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value));
    
    if (value < 0) {
        napi_throw_error(env, "INVALID_INPUT", "Square root of negative number");
        return nullptr;
    }
    
    double result = std::sqrt(value);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Sin(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double angle = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &angle));
    
    double result = std::sin(angle);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Cos(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double angle = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &angle));
    
    double result = std::cos(angle);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Tan(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double angle = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &angle));
    
    double result = std::tan(angle);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Log(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double value = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value));
    
    if (value <= 0) {
        napi_throw_error(env, "INVALID_INPUT", "Logarithm of non-positive number");
        return nullptr;
    }
    
    double result = std::log(value);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Log10(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double value = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value));
    
    if (value <= 0) {
        napi_throw_error(env, "INVALID_INPUT", "Logarithm of non-positive number");
        return nullptr;
    }
    
    double result = std::log10(value);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Abs(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double value = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value));
    
    double result = std::abs(value);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Floor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double value = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value));
    
    double result = std::floor(value);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Ceil(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double value = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value));
    
    double result = std::ceil(value);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value Round(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    double value = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value));
    
    double result = std::round(value);
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static int Precedence(char op)
{
    const int prec1 = 1;
    const int prec2 = 2;
    const int prec3 = 3;

    if (op == '+' || op == '-') {
        return prec1;
    }
    if (op == '*' || op == '/') {
        return prec2;
    }
    if (op == '^') {
        return prec3;
    }
    return 0;
}

static double ApplyOp(double a, double b, char op)
{
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '^': return std::pow(a, b);
        default: return 0;
    }
}

static void ParseNumber(const std::string& expr, int& i, int length, std::stack<double>& values)
{
    std::string numStr;
    while (i < length && (std::isdigit(expr[i]) || expr[i] == '.')) {
        numStr += expr[i++];
    }
    values.push(std::stod(numStr));
}

static void ProcessClosingParenthesis(std::stack<double>& values, std::stack<char>& ops)
{
    while (!ops.empty() && ops.top() != '(') {
        double val2 = values.top();
        values.pop();
        double val1 = values.top();
        values.pop();
        char op = ops.top();
        ops.pop();
        values.push(ApplyOp(val1, val2, op));
    }
    if (!ops.empty()) {
        ops.pop();
    }
}

static void ProcessOperator(char op, std::stack<double>& values, std::stack<char>& ops)
{
    while (!ops.empty() && Precedence(ops.top()) >= Precedence(op)) {
        double val2 = values.top();
        values.pop();
        double val1 = values.top();
        values.pop();
        char topOp = ops.top();
        ops.pop();
        values.push(ApplyOp(val1, val2, topOp));
    }
    ops.push(op);
}

static void FinalizeCalculation(std::stack<double>& values, std::stack<char>& ops)
{
    while (!ops.empty()) {
        double val2 = values.top();
        values.pop();
        double val1 = values.top();
        values.pop();
        char op = ops.top();
        ops.pop();
        values.push(ApplyOp(val1, val2, op));
    }
}

static napi_value EvaluateExpression(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    size_t length = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], nullptr, 0, &length));
    
    char* expr = new char[length + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], expr, length + 1, nullptr));
    
    std::stack<double> values;
    std::stack<char> ops;
    std::string exprStr(expr);
    delete[] expr;
    
    for (int i = 0; i < static_cast<int>(exprStr.length()); i++) {
        if (std::isspace(exprStr[i])) {
            continue;
        }
        
        if (std::isdigit(exprStr[i]) || exprStr[i] == '.') {
            ParseNumber(exprStr, i, exprStr.length(), values);
        } else if (exprStr[i] == '(') {
            ops.push(exprStr[i]);
        } else if (exprStr[i] == ')') {
            ProcessClosingParenthesis(values, ops);
        } else {
            ProcessOperator(exprStr[i], values, ops);
        }
    }
    
    FinalizeCalculation(values, ops);
    
    double result = values.top();
    napi_value resultValue;
    NAPI_CALL(env, napi_create_number(env, result, &resultValue));
    return resultValue;
}

static napi_value GetPi(napi_env env, napi_callback_info info)
{
    napi_value result;
    NAPI_CALL(env, napi_create_number(env, PI, &result));
    return result;
}

static napi_value GetE(napi_env env, napi_callback_info info)
{
    napi_value result;
    NAPI_CALL(env, napi_create_number(env, E, &result));
    return result;
}

} // namespace

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("add", Add),
        DECLARE_NAPI_FUNCTION("subtract", Subtract),
        DECLARE_NAPI_FUNCTION("multiply", Multiply),
        DECLARE_NAPI_FUNCTION("divide", Divide),
        DECLARE_NAPI_FUNCTION("power", Power),
        DECLARE_NAPI_FUNCTION("sqrt", SquareRoot),
        DECLARE_NAPI_FUNCTION("sin", Sin),
        DECLARE_NAPI_FUNCTION("cos", Cos),
        DECLARE_NAPI_FUNCTION("tan", Tan),
        DECLARE_NAPI_FUNCTION("log", Log),
        DECLARE_NAPI_FUNCTION("log10", Log10),
        DECLARE_NAPI_FUNCTION("abs", Abs),
        DECLARE_NAPI_FUNCTION("floor", Floor),
        DECLARE_NAPI_FUNCTION("ceil", Ceil),
        DECLARE_NAPI_FUNCTION("round", Round),
        DECLARE_NAPI_FUNCTION("evaluate", EvaluateExpression),
        DECLARE_NAPI_FUNCTION("getPi", GetPi),
        DECLARE_NAPI_FUNCTION("getE", GetE),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

extern "C" __attribute__((visibility("default"))) void NapiCalculatorGetJsCode(const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = BINARY_CALCULATOR_JS_START;
    }

    if (bufLen != nullptr) {
        *bufLen = BINARY_CALCULATOR_JS_END - BINARY_CALCULATOR_JS_START;
    }
}

static napi_module calculatorModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "calculator",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void CalculatorRegisterModule(void)
{
    napi_module_register(&calculatorModule);
}
