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

#ifndef FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_SYMBOL_SUITE_SYMBOL_HELPER_H
#define FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_SYMBOL_SUITE_SYMBOL_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <cstdint>
#include <string>
#include <vector>

namespace SymbolSuite {

constexpr size_t K_SYMBOL_CASE_COUNT = 20;
constexpr size_t K_CREATE_SYMBOL_CASE_COUNT = 12;
constexpr size_t K_TYPEOF_CASE_COUNT = 10;
constexpr size_t K_IDENTITY_CASE_COUNT = 10;
constexpr size_t K_PROPERTY_CASE_COUNT = 14;
constexpr size_t K_BOXING_CASE_COUNT = 8;
constexpr size_t K_MULTI_PROPERTY_CASE_COUNT = 8;
constexpr size_t K_SYMBOL_OBJECT_CASE_COUNT = 6;
constexpr size_t K_NESTED_PROPERTY_CASE_COUNT = 6;
constexpr size_t K_DELETE_CYCLE_CASE_COUNT = 6;

constexpr size_t K_ARG_COUNT_ONE = 1;
constexpr size_t K_ARG_COUNT_TWO = 2;
constexpr size_t K_ARG_COUNT_THREE = 3;
constexpr size_t K_ARG_COUNT_FOUR = 4;
constexpr size_t K_ARG_COUNT_FIVE = 5;

constexpr size_t K_FIRST_ARG_INDEX = 0;
constexpr size_t K_SECOND_ARG_INDEX = 1;
constexpr size_t K_THIRD_ARG_INDEX = 2;
constexpr size_t K_FOURTH_ARG_INDEX = 3;
constexpr size_t K_FIFTH_ARG_INDEX = 4;

constexpr int32_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr int32_t K_IDENTITY_SEED_BASE = 42;
constexpr int32_t K_IDENTITY_SEED_STEP = 7;
constexpr int32_t K_IDENTITY_SEED_CYCLE = 13;
constexpr int32_t K_PROPERTY_KEY_CYCLE = 5;
constexpr int32_t K_PROPERTY_VALUE_BASE = 100;
constexpr int32_t K_PROPERTY_VALUE_STEP = 11;
constexpr int32_t K_PROPERTY_VALUE_CYCLE = 7;
constexpr int32_t K_CHECKSUM_MODULO = 7;
constexpr int32_t K_MULTI_PROPERTY_COUNT = 3;
constexpr int32_t K_NESTED_DEPTH = 2;
constexpr int32_t K_DELETE_CYCLE_COUNT = 3;
constexpr int32_t K_SYMBOL_HASH_BASE = 31;
constexpr int32_t K_SYMBOL_HASH_MODULO = 1000000007;
constexpr int32_t K_VALUE_STRIDE = 10;
constexpr int32_t K_VALUE_SCALE = 100;
constexpr int32_t K_DELETED_STATUS_BIT = 2;
constexpr int32_t K_TYPE_CODE_SYMBOL = 1;
constexpr int32_t K_TYPE_CODE_NUMBER = 2;
constexpr int32_t K_TYPE_CODE_STRING = 3;
constexpr int32_t K_TYPE_CODE_OBJECT = 4;
constexpr int32_t K_TYPE_CODE_BOOLEAN = 5;

struct CreateSymbolCaseSpec {
    std::string name;
    std::string description;
    bool hasDescription;
    int32_t descLength;
};

struct TypeofCaseSpec {
    std::string name;
    std::string valueType;
    bool expectSymbol;
    int32_t typeCode;
};

struct IdentityCaseSpec {
    std::string name;
    int32_t seedA;
    int32_t seedB;
    bool sameSeeds;
    std::string descA;
    std::string descB;
};

struct PropertyCaseSpec {
    std::string name;
    std::string propertyKey;
    int32_t keyValue;
    bool useSymbolKey;
    int32_t propertyValue;
    bool expectPropertyFound;
    bool shouldDelete;
};

struct BoxingCaseSpec {
    std::string name;
    bool shouldBox;
    bool expectSymbolObject;
    int32_t symbolId;
};

struct MultiPropertyCaseSpec {
    std::string name;
    int32_t propertyCount;
    bool useSymbolKeys;
    std::vector<int32_t> values;
};

struct SymbolObjectCaseSpec {
    std::string name;
    bool createBoxed;
    bool checkIsSymbolObject;
    bool expectSymbolObject;
};

struct NestedPropertyCaseSpec {
    std::string name;
    int32_t depth;
    bool useSymbolKeys;
    int32_t baseValue;
};

struct DeleteCycleCaseSpec {
    std::string name;
    int32_t cycles;
    bool useSymbolKey;
};

std::string BuildIndexedName(const char* prefix, size_t caseNumber);

size_t GetCaseIndex(void* data);

CreateSymbolCaseSpec GetCreateSymbolCaseSpec(size_t caseIndex);

TypeofCaseSpec GetTypeofCaseSpec(size_t caseIndex);

IdentityCaseSpec GetIdentityCaseSpec(size_t caseIndex);

PropertyCaseSpec GetPropertyCaseSpec(size_t caseIndex);

BoxingCaseSpec GetBoxingCaseSpec(size_t caseIndex);

MultiPropertyCaseSpec GetMultiPropertyCaseSpec(size_t caseIndex);

SymbolObjectCaseSpec GetSymbolObjectCaseSpec(size_t caseIndex);

NestedPropertyCaseSpec GetNestedPropertyCaseSpec(size_t caseIndex);

DeleteCycleCaseSpec GetDeleteCycleCaseSpec(size_t caseIndex);

std::string BuildCreateSymbolExportName(size_t caseIndex);

std::string BuildTypeofExportName(size_t caseIndex);

std::string BuildIdentityExportName(size_t caseIndex);

std::string BuildPropertyExportName(size_t caseIndex);

std::string BuildBoxingExportName(size_t caseIndex);

std::string BuildMultiPropertyExportName(size_t caseIndex);

std::string BuildSymbolObjectExportName(size_t caseIndex);

std::string BuildNestedPropertyExportName(size_t caseIndex);

std::string BuildDeleteCycleExportName(size_t caseIndex);

bool CreateSymbolWithDesc(napi_env env, const std::string& desc, napi_value* symbol);

bool CreateSymbolWithoutDesc(napi_env env, napi_value* symbol);

bool CheckSymbolTypeof(napi_env env, napi_value value, bool* isSymbol);

bool CheckStrictEquals(napi_env env, napi_value a, napi_value b, bool* result);

bool SetPropertyWithKey(napi_env env, napi_value object, napi_value key, napi_value value);

bool GetPropertyWithKey(napi_env env, napi_value object, napi_value key, napi_value* value);

bool HasPropertyWithKey(napi_env env, napi_value object, napi_value key, bool* result);

bool DeletePropertyWithKey(napi_env env, napi_value object, napi_value key, bool* result);

bool CoerceToObject(napi_env env, napi_value value, napi_value* object);

bool IsSymbolObject(napi_env env, napi_value value, bool* result);

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value);

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value);

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value);

bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value);

bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value);

bool SetNamedUint32(napi_env env, napi_value object, const char* name, uint32_t value);

bool GetInt32Property(napi_env env, napi_value object, const char* name, int32_t* value);

bool GetStringProperty(napi_env env, napi_value object, const char* name, std::string* value);

bool GetBoolProperty(napi_env env, napi_value object, const char* name, bool* value);

bool GetDoubleProperty(napi_env env, napi_value object, const char* name, double* value);

bool GetInt64Property(napi_env env, napi_value object, const char* name, int64_t* value);

bool GetUint32Property(napi_env env, napi_value object, const char* name, uint32_t* value);

int32_t ComputeSymbolHash(const std::string& desc);

bool CreateSymbolKey(napi_env env, int32_t keyIndex, napi_value* key);

bool CreateMultipleSymbolKeys(napi_env env, int32_t count, std::vector<napi_value>& keys);

}  // namespace SymbolSuite

#endif  // FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_SYMBOL_SUITE_SYMBOL_HELPER_H