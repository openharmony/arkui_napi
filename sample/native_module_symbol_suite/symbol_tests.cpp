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

#include "symbol_helper.h"

#include <vector>

namespace {

using SymbolSuite::K_CREATE_SYMBOL_CASE_COUNT;
using SymbolSuite::K_TYPEOF_CASE_COUNT;
using SymbolSuite::K_IDENTITY_CASE_COUNT;
using SymbolSuite::K_PROPERTY_CASE_COUNT;
using SymbolSuite::K_BOXING_CASE_COUNT;
using SymbolSuite::K_MULTI_PROPERTY_CASE_COUNT;
using SymbolSuite::K_SYMBOL_OBJECT_CASE_COUNT;
using SymbolSuite::K_NESTED_PROPERTY_CASE_COUNT;
using SymbolSuite::K_DELETE_CYCLE_CASE_COUNT;
using SymbolSuite::K_ARG_COUNT_ONE;
using SymbolSuite::K_ARG_COUNT_TWO;
using SymbolSuite::K_ARG_COUNT_THREE;
using SymbolSuite::K_CHECKSUM_MODULO;
using SymbolSuite::K_VALUE_STRIDE;
using SymbolSuite::K_VALUE_SCALE;
using SymbolSuite::K_DELETED_STATUS_BIT;

using SymbolSuite::CreateSymbolCaseSpec;
using SymbolSuite::TypeofCaseSpec;
using SymbolSuite::IdentityCaseSpec;
using SymbolSuite::PropertyCaseSpec;
using SymbolSuite::BoxingCaseSpec;
using SymbolSuite::MultiPropertyCaseSpec;
using SymbolSuite::SymbolObjectCaseSpec;
using SymbolSuite::NestedPropertyCaseSpec;
using SymbolSuite::DeleteCycleCaseSpec;

using SymbolSuite::GetCaseIndex;
using SymbolSuite::GetCreateSymbolCaseSpec;
using SymbolSuite::GetTypeofCaseSpec;
using SymbolSuite::GetIdentityCaseSpec;
using SymbolSuite::GetPropertyCaseSpec;
using SymbolSuite::GetBoxingCaseSpec;
using SymbolSuite::GetMultiPropertyCaseSpec;
using SymbolSuite::GetSymbolObjectCaseSpec;
using SymbolSuite::GetNestedPropertyCaseSpec;
using SymbolSuite::GetDeleteCycleCaseSpec;
using SymbolSuite::BuildCreateSymbolExportName;
using SymbolSuite::BuildTypeofExportName;
using SymbolSuite::BuildIdentityExportName;
using SymbolSuite::BuildPropertyExportName;
using SymbolSuite::BuildBoxingExportName;
using SymbolSuite::BuildMultiPropertyExportName;
using SymbolSuite::BuildSymbolObjectExportName;
using SymbolSuite::BuildNestedPropertyExportName;
using SymbolSuite::BuildDeleteCycleExportName;
using SymbolSuite::CreateSymbolWithDesc;
using SymbolSuite::CreateSymbolWithoutDesc;
using SymbolSuite::CheckSymbolTypeof;
using SymbolSuite::CheckStrictEquals;
using SymbolSuite::SetPropertyWithKey;
using SymbolSuite::GetPropertyWithKey;
using SymbolSuite::HasPropertyWithKey;
using SymbolSuite::DeletePropertyWithKey;
using SymbolSuite::CoerceToObject;
using SymbolSuite::IsSymbolObject;
using SymbolSuite::SetNamedInt32;
using SymbolSuite::SetNamedString;
using SymbolSuite::SetNamedBool;
using SymbolSuite::CreateSymbolKey;

napi_value BuildResultObject(napi_env env, const std::string& name)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", name);
    return result;
}

bool CreateTestKey(napi_env env, bool useSymbol, const std::string& keyStr, napi_value* key)
{
    if (useSymbol) {
        return CreateSymbolWithDesc(env, keyStr, key);
    }
    return napi_create_string_utf8(env, keyStr.c_str(), keyStr.size(), key) == napi_ok;
}

void SetResultChecksum(napi_env env, napi_value result, int32_t value)
{
    SetNamedInt32(env, result, "checksum", value % K_CHECKSUM_MODULO);
}

static napi_value RunCreateSymbolCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_CREATE_SYMBOL_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid create symbol case");
        return nullptr;
    }
    const auto spec = GetCreateSymbolCaseSpec(caseIndex);
    napi_value symbol = nullptr;
    bool createOk = spec.hasDescription ?
        CreateSymbolWithDesc(env, spec.description, &symbol) :
        CreateSymbolWithoutDesc(env, &symbol);
    if (!createOk || symbol == nullptr) {
        return nullptr;
    }
    bool isSymbol = false;
    if (!CheckSymbolTypeof(env, symbol, &isSymbol)) {
        return nullptr;
    }
    napi_value result = BuildResultObject(env, spec.name);
    SetNamedBool(env, result, "hasDescription", spec.hasDescription);
    SetNamedBool(env, result, "isSymbolType", isSymbol);
    SetNamedInt32(env, result, "descLength", spec.descLength);
    napi_set_named_property(env, result, "symbol", symbol);
    SetResultChecksum(env, result, static_cast<int32_t>(isSymbol ? 1 : 0));
    return result;
}

static napi_value RunTypeofCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_TYPEOF_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid typeof case");
        return nullptr;
    }
    const auto spec = GetTypeofCaseSpec(caseIndex);
    napi_value testValue = nullptr;
    if (spec.expectSymbol) {
        if (!CreateSymbolWithoutDesc(env, &testValue)) {
            return nullptr;
        }
    } else if (spec.valueType == "number") {
        NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(caseIndex), &testValue));
    } else if (spec.valueType == "string") {
        std::string strVal = "test" + std::to_string(caseIndex);
        NAPI_CALL(env, napi_create_string_utf8(env, strVal.c_str(), strVal.size(), &testValue));
    } else if (spec.valueType == "boolean") {
        NAPI_CALL(env, napi_get_boolean(env, (caseIndex % K_ARG_COUNT_TWO) == 0, &testValue));
    } else {
        NAPI_CALL(env, napi_create_object(env, &testValue));
    }
    bool isSymbol = false;
    if (!CheckSymbolTypeof(env, testValue, &isSymbol)) {
        return nullptr;
    }
    napi_value result = BuildResultObject(env, spec.name);
    SetNamedString(env, result, "valueType", spec.valueType);
    SetNamedBool(env, result, "expectSymbol", spec.expectSymbol);
    SetNamedBool(env, result, "isSymbolType", isSymbol);
    SetNamedBool(env, result, "typeMatch", isSymbol == spec.expectSymbol);
    SetNamedInt32(env, result, "typeCode", spec.typeCode);
    napi_set_named_property(env, result, "value", testValue);
    SetResultChecksum(env, result, static_cast<int32_t>(isSymbol == spec.expectSymbol ? 1 : 0));
    return result;
}

static napi_value RunIdentityCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_IDENTITY_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid identity case");
        return nullptr;
    }
    const auto spec = GetIdentityCaseSpec(caseIndex);
    napi_value symA = nullptr;
    napi_value symB = nullptr;
    if (!CreateSymbolWithDesc(env, spec.descA, &symA) ||
        !CreateSymbolWithDesc(env, spec.descB, &symB)) {
        return nullptr;
    }
    bool equalAB = false;
    CheckStrictEquals(env, symA, symB, &equalAB);
    napi_value result = BuildResultObject(env, spec.name);
    SetNamedBool(env, result, "sameSeeds", spec.sameSeeds);
    SetNamedBool(env, result, "equalAB", equalAB);
    SetNamedInt32(env, result, "seedA", spec.seedA);
    SetNamedInt32(env, result, "seedB", spec.seedB);
    napi_set_named_property(env, result, "symbolA", symA);
    napi_set_named_property(env, result, "symbolB", symB);
    SetResultChecksum(env, result, static_cast<int32_t>(spec.sameSeeds == equalAB ? 1 : 0));
    return result;
}

struct PropertyCheckResult {
    bool hasBefore;
    int32_t retrievedInt;
};

void SetPropertyAndCheck(napi_env env, napi_value object, napi_value key, int32_t value,
    PropertyCheckResult* result)
{
    napi_value propValue = nullptr;
    napi_create_int32(env, value, &propValue);
    SetPropertyWithKey(env, object, key, propValue);
    HasPropertyWithKey(env, object, key, &result->hasBefore);
    napi_value retrievedValue = nullptr;
    GetPropertyWithKey(env, object, key, &retrievedValue);
    if (retrievedValue != nullptr) {
        napi_get_value_int32(env, retrievedValue, &result->retrievedInt);
    }
}

static napi_value RunPropertyCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_PROPERTY_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid property case");
        return nullptr;
    }
    const auto spec = GetPropertyCaseSpec(caseIndex);
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));
    napi_value key = nullptr;
    std::string keyStr = spec.useSymbolKey ? "key" + std::to_string(spec.keyValue) : spec.propertyKey;
    if (!CreateTestKey(env, spec.useSymbolKey, keyStr, &key)) {
        return nullptr;
    }
    PropertyCheckResult checkResult = {};
    SetPropertyAndCheck(env, object, key, spec.propertyValue, &checkResult);
    bool deleted = false;
    if (spec.shouldDelete) {
        DeletePropertyWithKey(env, object, key, &deleted);
    }
    bool hasAfter = false;
    HasPropertyWithKey(env, object, key, &hasAfter);
    napi_value result = BuildResultObject(env, spec.name);
    SetNamedBool(env, result, "useSymbolKey", spec.useSymbolKey);
    SetNamedBool(env, result, "hasBefore", checkResult.hasBefore);
    SetNamedBool(env, result, "hasAfter", hasAfter);
    SetNamedBool(env, result, "deleted", deleted);
    napi_set_named_property(env, result, "object", object);
    SetResultChecksum(env, result, static_cast<int32_t>(checkResult.hasBefore ? 1 : 0) +
        static_cast<int32_t>(deleted ? K_DELETED_STATUS_BIT : 0));
    return result;
}

static napi_value RunBoxingCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_BOXING_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid boxing case");
        return nullptr;
    }
    const auto spec = GetBoxingCaseSpec(caseIndex);
    napi_value originalSymbol = nullptr;
    std::string desc = "boxedSym" + std::to_string(spec.symbolId);
    if (!CreateSymbolWithDesc(env, desc, &originalSymbol)) {
        return nullptr;
    }
    napi_value boxedValue = originalSymbol;
    if (spec.shouldBox) {
        CoerceToObject(env, originalSymbol, &boxedValue);
    }
    bool isSymbolObject = false;
    IsSymbolObject(env, boxedValue, &isSymbolObject);
    napi_value result = BuildResultObject(env, spec.name);
    SetNamedBool(env, result, "shouldBox", spec.shouldBox);
    SetNamedBool(env, result, "expectSymbolObject", spec.expectSymbolObject);
    SetNamedBool(env, result, "isSymbolObject", isSymbolObject);
    SetNamedBool(env, result, "boxMatch", isSymbolObject == spec.expectSymbolObject);
    SetNamedInt32(env, result, "symbolId", spec.symbolId);
    napi_set_named_property(env, result, "originalSymbol", originalSymbol);
    napi_set_named_property(env, result, "boxedValue", boxedValue);
    SetResultChecksum(env, result, static_cast<int32_t>(isSymbolObject == spec.expectSymbolObject ? 1 : 0));
    return result;
}

bool BuildMultiPropertyKeys(napi_env env, bool useSymbolKeys, int32_t count,
    std::vector<napi_value>& keys)
{
    for (int32_t i = 0; i < count; i++) {
        napi_value key = nullptr;
        if (useSymbolKeys) {
            if (!CreateSymbolKey(env, i, &key)) {
                return false;
            }
        } else {
            std::string keyStr = "prop" + std::to_string(i);
            if (napi_create_string_utf8(env, keyStr.c_str(), keyStr.size(), &key) != napi_ok) {
                return false;
            }
        }
        keys.push_back(key);
    }
    return true;
}

static napi_value RunMultiPropertyCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_MULTI_PROPERTY_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid multi property case");
        return nullptr;
    }
    const auto spec = GetMultiPropertyCaseSpec(caseIndex);
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));
    std::vector<napi_value> keys;
    if (!BuildMultiPropertyKeys(env, spec.useSymbolKeys, spec.propertyCount, keys)) {
        return nullptr;
    }
    for (size_t i = 0; i < keys.size(); i++) {
        napi_value val = nullptr;
        NAPI_CALL(env, napi_create_int32(env, spec.values[i], &val));
        SetPropertyWithKey(env, object, keys[i], val);
    }
    int32_t verifiedCount = 0;
    for (size_t i = 0; i < keys.size(); i++) {
        bool hasProp = false;
        if (HasPropertyWithKey(env, object, keys[i], &hasProp) && hasProp) {
            verifiedCount++;
        }
    }
    napi_value result = BuildResultObject(env, spec.name);
    SetNamedInt32(env, result, "propertyCount", spec.propertyCount);
    SetNamedBool(env, result, "useSymbolKeys", spec.useSymbolKeys);
    SetNamedInt32(env, result, "verifiedCount", verifiedCount);
    napi_set_named_property(env, result, "object", object);
    SetResultChecksum(env, result, verifiedCount);
    return result;
}

static napi_value RunSymbolObjectCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_SYMBOL_OBJECT_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid symbol object case");
        return nullptr;
    }
    const auto spec = GetSymbolObjectCaseSpec(caseIndex);
    napi_value symbol = nullptr;
    std::string desc = "symObj" + std::to_string(caseIndex);
    if (!CreateSymbolWithDesc(env, desc, &symbol)) {
        return nullptr;
    }
    napi_value objValue = symbol;
    if (spec.createBoxed) {
        CoerceToObject(env, symbol, &objValue);
    }
    bool isSymbolObject = false;
    if (spec.checkIsSymbolObject) {
        IsSymbolObject(env, objValue, &isSymbolObject);
    }
    napi_value result = BuildResultObject(env, spec.name);
    SetNamedBool(env, result, "createBoxed", spec.createBoxed);
    SetNamedBool(env, result, "expectSymbolObject", spec.expectSymbolObject);
    SetNamedBool(env, result, "isSymbolObject", isSymbolObject);
    SetNamedBool(env, result, "match", isSymbolObject == spec.expectSymbolObject);
    napi_set_named_property(env, result, "symbol", symbol);
    napi_set_named_property(env, result, "objValue", objValue);
    SetResultChecksum(env, result, static_cast<int32_t>(isSymbolObject == spec.expectSymbolObject ? 1 : 0));
    return result;
}

struct NestedWalkContext {
    int32_t maxDepth;
    bool useSymbolKeys;
    int32_t baseValue;
};

bool ProcessNestedLevel(napi_env env, napi_value currentObj, int32_t depth,
    const NestedWalkContext& context, int32_t* totalDepth)
{
    if (depth >= context.maxDepth) {
        return true;
    }
    napi_value key = nullptr;
    std::string keyStr = "nested" + std::to_string(depth);
    if (context.useSymbolKeys) {
        if (!CreateSymbolWithDesc(env, keyStr, &key)) {
            return false;
        }
    } else {
        if (napi_create_string_utf8(env, keyStr.c_str(), keyStr.size(), &key) != napi_ok) {
            return false;
        }
    }
    napi_value childObject = nullptr;
    if (napi_create_object(env, &childObject) != napi_ok) {
        return false;
    }
    napi_value valueProp = nullptr;
    if (napi_create_int32(env, context.baseValue + depth * K_VALUE_STRIDE, &valueProp) != napi_ok) {
        return false;
    }
    napi_set_named_property(env, childObject, "value", valueProp);
    if (!SetPropertyWithKey(env, currentObj, key, childObject)) {
        return false;
    }
    bool hasProp = false;
    if (!HasPropertyWithKey(env, currentObj, key, &hasProp) || !hasProp) {
        return false;
    }
    napi_value retrievedChild = nullptr;
    if (!GetPropertyWithKey(env, currentObj, key, &retrievedChild)) {
        return false;
    }
    (*totalDepth)++;
    return ProcessNestedLevel(env, retrievedChild, depth + 1, context, totalDepth);
}

static napi_value RunNestedPropertyCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_NESTED_PROPERTY_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid nested property case");
        return nullptr;
    }
    const auto spec = GetNestedPropertyCaseSpec(caseIndex);
    napi_value rootObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &rootObject));
    int32_t totalDepth = 0;
    NestedWalkContext context = {spec.depth, spec.useSymbolKeys, spec.baseValue};
    bool allVerified = ProcessNestedLevel(env, rootObject, 0, context, &totalDepth);
    napi_value result = BuildResultObject(env, spec.name);
    SetNamedInt32(env, result, "depth", spec.depth);
    SetNamedBool(env, result, "useSymbolKeys", spec.useSymbolKeys);
    SetNamedBool(env, result, "allVerified", allVerified);
    SetNamedInt32(env, result, "totalDepth", totalDepth);
    napi_set_named_property(env, result, "rootObject", rootObject);
    SetResultChecksum(env, result, totalDepth);
    return result;
}

bool ExecuteDeleteCycle(napi_env env, napi_value object, napi_value key, int32_t cycle,
    bool* success)
{
    napi_value value = nullptr;
    if (napi_create_int32(env, cycle * K_VALUE_SCALE, &value) != napi_ok) {
        return false;
    }
    if (!SetPropertyWithKey(env, object, key, value)) {
        return false;
    }
    bool hasBefore = false;
    if (!HasPropertyWithKey(env, object, key, &hasBefore) || !hasBefore) {
        return false;
    }
    bool deleted = false;
    if (!DeletePropertyWithKey(env, object, key, &deleted) || !deleted) {
        return false;
    }
    bool hasAfter = false;
    if (!HasPropertyWithKey(env, object, key, &hasAfter) || hasAfter) {
        return false;
    }
    *success = true;
    return true;
}

static napi_value RunDeleteCycleCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_DELETE_CYCLE_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid delete cycle case");
        return nullptr;
    }
    const auto spec = GetDeleteCycleCaseSpec(caseIndex);
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));
    napi_value key = nullptr;
    std::string keyStr = "cycleKey" + std::to_string(caseIndex);
    if (!CreateTestKey(env, spec.useSymbolKey, keyStr, &key)) {
        return nullptr;
    }
    int32_t successfulCycles = 0;
    for (int32_t cycle = 0; cycle < spec.cycles; cycle++) {
        bool success = false;
        if (!ExecuteDeleteCycle(env, object, key, cycle, &success) || !success) {
            break;
        }
        successfulCycles++;
    }
    napi_value result = BuildResultObject(env, spec.name);
    SetNamedInt32(env, result, "cycles", spec.cycles);
    SetNamedBool(env, result, "useSymbolKey", spec.useSymbolKey);
    SetNamedInt32(env, result, "successfulCycles", successfulCycles);
    napi_set_named_property(env, result, "object", object);
    SetResultChecksum(env, result, successfulCycles);
    return result;
}

std::vector<napi_property_descriptor> BuildDescriptors(
    size_t count, std::vector<std::string>& exportNames,
    std::string (*buildName)(size_t), napi_callback runCase)
{
    exportNames.reserve(count);
    std::vector<napi_property_descriptor> descriptors(count);
    for (size_t caseIndex = 0; caseIndex < count; caseIndex++) {
        exportNames.emplace_back(buildName(caseIndex));
        descriptors[caseIndex] = napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            runCase,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex)),
        };
    }
    return descriptors;
}

void AppendAllDescriptors(std::vector<napi_property_descriptor>& all,
    const std::vector<napi_property_descriptor>& src)
{
    for (const auto& desc : src) {
        all.push_back(desc);
    }
}

bool ValidateSymbolKey(napi_env env, napi_value key, bool* isValidSymbol)
{
    if (key == nullptr) {
        *isValidSymbol = false;
        return true;
    }
    return CheckSymbolTypeof(env, key, isValidSymbol);
}

bool ValidateSymbolProperty(napi_env env, napi_value object, napi_value key,
    int32_t expectedValue, bool* match)
{
    napi_value value = nullptr;
    if (!GetPropertyWithKey(env, object, key, &value)) {
        *match = false;
        return true;
    }
    int32_t actualValue = 0;
    if (napi_get_value_int32(env, value, &actualValue) != napi_ok) {
        *match = false;
        return true;
    }
    *match = (actualValue == expectedValue);
    return true;
}

bool CreateAndTestSymbol(napi_env env, const std::string& desc, napi_value* symbol,
    bool* isSymbolType)
{
    if (!CreateSymbolWithDesc(env, desc, symbol)) {
        return false;
    }
    return CheckSymbolTypeof(env, *symbol, isSymbolType);
}

bool TestSymbolEquality(napi_env env, const std::string& descA, const std::string& descB,
    bool* equal)
{
    napi_value symA = nullptr;
    napi_value symB = nullptr;
    if (!CreateSymbolWithDesc(env, descA, &symA) || !CreateSymbolWithDesc(env, descB, &symB)) {
        return false;
    }
    return CheckStrictEquals(env, symA, symB, equal);
}

}  // namespace

static napi_value InitSymbolSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> createSymbolNames;
    std::vector<std::string> typeofNames;
    std::vector<std::string> identityNames;
    std::vector<std::string> propertyNames;
    std::vector<std::string> boxingNames;
    std::vector<std::string> multiPropertyNames;
    std::vector<std::string> symbolObjectNames;
    std::vector<std::string> nestedPropertyNames;
    std::vector<std::string> deleteCycleNames;
    auto createSymbolDescs = BuildDescriptors(K_CREATE_SYMBOL_CASE_COUNT, createSymbolNames,
        BuildCreateSymbolExportName, RunCreateSymbolCase);
    auto typeofDescs = BuildDescriptors(K_TYPEOF_CASE_COUNT, typeofNames,
        BuildTypeofExportName, RunTypeofCase);
    auto identityDescs = BuildDescriptors(K_IDENTITY_CASE_COUNT, identityNames,
        BuildIdentityExportName, RunIdentityCase);
    auto propertyDescs = BuildDescriptors(K_PROPERTY_CASE_COUNT, propertyNames,
        BuildPropertyExportName, RunPropertyCase);
    auto boxingDescs = BuildDescriptors(K_BOXING_CASE_COUNT, boxingNames,
        BuildBoxingExportName, RunBoxingCase);
    auto multiPropertyDescs = BuildDescriptors(K_MULTI_PROPERTY_CASE_COUNT, multiPropertyNames,
        BuildMultiPropertyExportName, RunMultiPropertyCase);
    auto symbolObjectDescs = BuildDescriptors(K_SYMBOL_OBJECT_CASE_COUNT, symbolObjectNames,
        BuildSymbolObjectExportName, RunSymbolObjectCase);
    auto nestedPropertyDescs = BuildDescriptors(K_NESTED_PROPERTY_CASE_COUNT, nestedPropertyNames,
        BuildNestedPropertyExportName, RunNestedPropertyCase);
    auto deleteCycleDescs = BuildDescriptors(K_DELETE_CYCLE_CASE_COUNT, deleteCycleNames,
        BuildDeleteCycleExportName, RunDeleteCycleCase);

    std::vector<napi_property_descriptor> all;
    AppendAllDescriptors(all, createSymbolDescs);
    AppendAllDescriptors(all, typeofDescs);
    AppendAllDescriptors(all, identityDescs);
    AppendAllDescriptors(all, propertyDescs);
    AppendAllDescriptors(all, boxingDescs);
    AppendAllDescriptors(all, multiPropertyDescs);
    AppendAllDescriptors(all, symbolObjectDescs);
    AppendAllDescriptors(all, nestedPropertyDescs);
    AppendAllDescriptors(all, deleteCycleDescs);

    NAPI_CALL(env, napi_define_properties(env, exports, all.size(), all.data()));
    return exports;
}

static napi_module g_symbolSuiteModule = {
    .nm_version = SymbolSuite::K_MODULE_VERSION,
    .nm_flags = SymbolSuite::K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitSymbolSuite,
    .nm_modname = "symbol_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterSymbolSuiteModule(void)
{
    napi_module_register(&g_symbolSuiteModule);
}