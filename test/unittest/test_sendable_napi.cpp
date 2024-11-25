/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <chrono>
#include <thread>

#include "test.h"
#include "test_common.h"
#include "gtest/gtest.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi/native_common.h"
#include "securec.h"
#include "utils/log.h"

static constexpr size_t TEST_STR_LENGTH = 30;

class NapiSendableTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiSendableTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiSendableTest TearDownTestCase";
    }

    void SetUp() override
    {
        napi_env env = reinterpret_cast<napi_env>(engine_);
        napi_open_handle_scope(env, &scope_);
    }

    void TearDown() override
    {
        napi_env env = reinterpret_cast<napi_env>(engine_);
        napi_value exception = nullptr;
        napi_get_and_clear_last_exception(env, &exception);
        napi_close_handle_scope(env, scope_);
    }
private:
    napi_handle_scope scope_ = nullptr;
};

napi_value GetSendableClass(napi_env env)
{
    auto constructor = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };

    napi_value static_str;
    napi_value non_static_str;
    napi_value instance_str;
    napi_create_string_utf8(env, "static", NAPI_AUTO_LENGTH, &static_str);
    napi_create_string_utf8(env, "nonStatic", NAPI_AUTO_LENGTH, &non_static_str);
    napi_create_string_utf8(env, "instance", NAPI_AUTO_LENGTH, &instance_str);

    napi_property_attributes napi_instance = static_cast<napi_property_attributes>(1 << 9 | 1 << 0);
    napi_property_descriptor desc[] = { DECLARE_NAPI_STATIC_PROPERTY("static", static_str),
                                        DECLARE_NAPI_DEFAULT_PROPERTY("nonStatic", non_static_str),
                                        { "instance", nullptr, nullptr, nullptr, nullptr, instance_str, napi_instance,
                                          nullptr } };

    napi_value sendableClass = nullptr;
    napi_define_sendable_class(env, "SendableClass", NAPI_AUTO_LENGTH, constructor, nullptr,
                               sizeof(desc) / sizeof(desc[0]), desc, nullptr, &sendableClass);

    return sendableClass;
}

napi_value GetSendableChildClass(napi_env env, napi_value parentClass)
{
    auto constructor = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };

    napi_value static_str;
    napi_value non_static_str;
    napi_value instance_str;
    napi_create_string_utf8(env, "static", NAPI_AUTO_LENGTH, &static_str);
    napi_create_string_utf8(env, "nonStatic", NAPI_AUTO_LENGTH, &non_static_str);
    napi_create_string_utf8(env, "instance", NAPI_AUTO_LENGTH, &instance_str);

    napi_property_attributes napi_instance = static_cast<napi_property_attributes>(1 << 9 | 1 << 0);
    napi_property_descriptor desc[] = { DECLARE_NAPI_STATIC_PROPERTY("static", static_str),
                                        DECLARE_NAPI_DEFAULT_PROPERTY("nonStatic", non_static_str),
                                        { "instance", nullptr, nullptr, nullptr, nullptr, instance_str, napi_instance,
                                          nullptr } };

    napi_value childClass = nullptr;
    napi_define_sendable_class(env, "SendableClass", NAPI_AUTO_LENGTH, constructor, nullptr,
                               sizeof(desc) / sizeof(desc[0]), desc, parentClass, &childClass);

    return childClass;
}

napi_value GetSendableParentClass(napi_env env)
{
    auto constructor = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };

    napi_value parent_static_str;
    napi_value parent_non_static_str;
    napi_value parent_instance_str;
    napi_create_string_utf8(env, "parentStatic", NAPI_AUTO_LENGTH, &parent_static_str);
    napi_create_string_utf8(env, "parentNonStatic", NAPI_AUTO_LENGTH, &parent_non_static_str);
    napi_create_string_utf8(env, "parentInstance", NAPI_AUTO_LENGTH, &parent_instance_str);

    napi_property_attributes napi_instance = static_cast<napi_property_attributes>(1 << 9 | 1 << 0);
    napi_property_descriptor parentDesc[] = { DECLARE_NAPI_STATIC_PROPERTY("parentStatic", parent_static_str),
                                              DECLARE_NAPI_DEFAULT_PROPERTY("parentNonStatic", parent_non_static_str),
                                              { "parentInstance", nullptr, nullptr, nullptr, nullptr,
                                                parent_instance_str, napi_instance, nullptr } };

    napi_value parentClass = nullptr;
    napi_define_sendable_class(env, "ParentClass", NAPI_AUTO_LENGTH, constructor, nullptr,
                               sizeof(parentDesc) / sizeof(parentDesc[0]), parentDesc, nullptr, &parentClass);

    return parentClass;
}

HWTEST_F(NapiSendableTest, IsSendableTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    res = napi_create_sendable_map(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_sendable_map(env, &result));

    bool isShared = false;
    ASSERT_CHECK_CALL(napi_is_sendable(env, result, &isShared));
    ASSERT_EQ(isShared, true);
}

HWTEST_F(NapiSendableTest, IsSendableTest002, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    res = napi_create_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, false);
}

HWTEST_F(NapiSendableTest, SendableClassTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    auto constructor = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };

    napi_value static_str;
    napi_value non_static_str;
    napi_value name_str;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "static", NAPI_AUTO_LENGTH, &static_str));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "nonStatic", NAPI_AUTO_LENGTH, &non_static_str));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "name", NAPI_AUTO_LENGTH, &name_str));

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("static", static_str),
        DECLARE_NAPI_DEFAULT_PROPERTY("nonStatic", non_static_str),
        DECLARE_NAPI_STATIC_FUNCTION("staticFunc",
                                     [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_FUNCTION("func", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_GETTER_SETTER(
            "getterSetter", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
            [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
    };

    napi_value sendableClass = nullptr;
    ASSERT_CHECK_CALL(napi_define_sendable_class(env, "SendableClass", NAPI_AUTO_LENGTH, constructor, nullptr,
                                                 sizeof(desc) / sizeof(desc[0]), desc, nullptr, &sendableClass));

    napi_value value = nullptr;
    char* str = new char[TEST_STR_LENGTH];
    size_t length;

    ASSERT_CHECK_CALL(napi_get_property(env, sendableClass, name_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("SendableClass", str);
    ASSERT_CHECK_VALUE_TYPE(env, sendableClass, napi_function);
}


HWTEST_F(NapiSendableTest, SendableClassTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    auto constructor = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };

    napi_value static_str;
    napi_value non_static_str;
    napi_value invalid_str;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "static", NAPI_AUTO_LENGTH, &static_str));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "nonStatic", NAPI_AUTO_LENGTH, &non_static_str));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "invalid", NAPI_AUTO_LENGTH, &invalid_str));

    napi_property_attributes napi_writable_static = static_cast<napi_property_attributes>(napi_static | napi_writable);
    napi_property_descriptor desc[] = {
        { "static", nullptr, nullptr, nullptr, nullptr, static_str, napi_writable_static, nullptr },
        DECLARE_NAPI_DEFAULT_PROPERTY("nonStatic", non_static_str),
    };

    napi_value sendableClass = nullptr;
    ASSERT_CHECK_CALL(napi_define_sendable_class(env, "SendableClass", NAPI_AUTO_LENGTH, constructor, nullptr,
                                                 sizeof(desc) / sizeof(desc[0]), desc, nullptr, &sendableClass));

    napi_value value = nullptr;
    char* str = new char[TEST_STR_LENGTH];
    size_t length;

    // get static property from constructor
    ASSERT_CHECK_CALL(napi_get_property(env, sendableClass, static_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("static", str);
    // get invalid property from constructor
    ASSERT_CHECK_CALL(napi_get_property(env, sendableClass, invalid_str, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_undefined);

    napi_value static_value;
    napi_value non_static_value;
    napi_value invalid_value;
    napi_value exception = nullptr;
    bool isExceptionPending = false;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "static0", NAPI_AUTO_LENGTH, &static_value));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "nonStatic0", NAPI_AUTO_LENGTH, &non_static_value));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "invalid", NAPI_AUTO_LENGTH, &invalid_value));

    // set static property on constructor
    napi_set_property(env, sendableClass, static_str, static_value);
    ASSERT_CHECK_CALL(napi_get_property(env, sendableClass, static_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("static0", str);

    // set invalid property on constructor
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_FALSE(isExceptionPending);
    napi_set_property(env, sendableClass, invalid_str, invalid_value);
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_TRUE(isExceptionPending);
    napi_get_and_clear_last_exception(env, &exception);
    ASSERT_CHECK_CALL(napi_get_property(env, sendableClass, invalid_str, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_undefined);
}

HWTEST_F(NapiSendableTest, SendableClassTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value sendableClass = GetSendableClass(env);
    napi_value non_static_str;
    napi_value invalid_str;
    napi_value instance_str;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "nonStatic", NAPI_AUTO_LENGTH, &non_static_str));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "invalid", NAPI_AUTO_LENGTH, &invalid_str));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "instance", NAPI_AUTO_LENGTH, &instance_str));

    // new instance
    napi_value sendableInstance = nullptr;
    ASSERT_CHECK_CALL(napi_new_instance(env, sendableClass, 0, nullptr, &sendableInstance));
    bool isInstanceOf = false;
    ASSERT_CHECK_CALL(napi_instanceof(env, sendableInstance, sendableClass, &isInstanceOf));
    ASSERT_TRUE(isInstanceOf);

    // get prototype
    napi_value prototype = nullptr;
    ASSERT_CHECK_CALL(napi_get_prototype(env, sendableInstance, &prototype));
    ASSERT_CHECK_VALUE_TYPE(env, prototype, napi_object);

    napi_value value = nullptr;
    char* str = new char[TEST_STR_LENGTH];
    size_t length;
    napi_value instance_value;
    napi_value non_static_value;
    napi_value invalid_value;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "instance", NAPI_AUTO_LENGTH, &instance_value));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "nonStatic0", NAPI_AUTO_LENGTH, &non_static_value));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "invalid", NAPI_AUTO_LENGTH, &invalid_value));

    // get initial instance property
    ASSERT_CHECK_CALL(napi_get_property(env, sendableInstance, instance_str, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_undefined);
    // set instance property
    napi_set_property(env, sendableInstance, instance_str, instance_value);
    ASSERT_CHECK_CALL(napi_get_property(env, sendableInstance, instance_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("instance", str);

    // get non static property from instance
    ASSERT_CHECK_CALL(napi_get_property(env, sendableInstance, non_static_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("nonStatic", str);
    // set non static property on prototype and get from instance
    napi_set_property(env, prototype, non_static_str, non_static_value);
    ASSERT_CHECK_CALL(napi_get_property(env, sendableInstance, non_static_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("nonStatic0", str);
}

HWTEST_F(NapiSendableTest, SendableClassTest004, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value sendableClass = GetSendableClass(env);
    napi_value non_static_str;
    napi_value invalid_str;
    napi_value instance_str;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "nonStatic", NAPI_AUTO_LENGTH, &non_static_str));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "invalid", NAPI_AUTO_LENGTH, &invalid_str));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "instance", NAPI_AUTO_LENGTH, &instance_str));

    // new instance
    napi_value sendableInstance = nullptr;
    ASSERT_CHECK_CALL(napi_new_instance(env, sendableClass, 0, nullptr, &sendableInstance));
    bool isInstanceOf = false;
    ASSERT_CHECK_CALL(napi_instanceof(env, sendableInstance, sendableClass, &isInstanceOf));
    ASSERT_TRUE(isInstanceOf);

    napi_value value = nullptr;
    char* str = new char[TEST_STR_LENGTH];
    size_t length;
    napi_value non_static_value;
    napi_value invalid_value;
    napi_value exception = nullptr;
    bool isExceptionPending = false;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "nonStatic0", NAPI_AUTO_LENGTH, &non_static_value));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "invalid", NAPI_AUTO_LENGTH, &invalid_value));

    // set non static property on instance
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_FALSE(isExceptionPending);
    napi_set_property(env, sendableInstance, non_static_str, non_static_value);
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_TRUE(isExceptionPending);
    napi_get_and_clear_last_exception(env, &exception);
    ASSERT_CHECK_CALL(napi_get_property(env, sendableInstance, non_static_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("nonStatic", str);

    // set invalid property on instance
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_FALSE(isExceptionPending);
    napi_set_property(env, sendableInstance, invalid_str, invalid_value);
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_TRUE(isExceptionPending);
    napi_get_and_clear_last_exception(env, &exception);
    ASSERT_CHECK_CALL(napi_get_property(env, sendableInstance, invalid_str, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_undefined);
}


HWTEST_F(NapiSendableTest, SendableClassTest005, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value parentClass = GetSendableParentClass(env);
    napi_value sendableClass = GetSendableChildClass(env, parentClass);
    napi_value parent_static_str;
    napi_value parent_non_static_str;
    napi_value parent_instance_str;
    napi_create_string_utf8(env, "parentStatic", NAPI_AUTO_LENGTH, &parent_static_str);
    napi_create_string_utf8(env, "parentNonStatic", NAPI_AUTO_LENGTH, &parent_non_static_str);
    napi_create_string_utf8(env, "parentInstance", NAPI_AUTO_LENGTH, &parent_instance_str);

    // new instance
    napi_value sendableInstance = nullptr;
    ASSERT_CHECK_CALL(napi_new_instance(env, sendableClass, 0, nullptr, &sendableInstance));
    bool isInstanceOf = false;
    bool isInstanceOfParent = false;
    ASSERT_CHECK_CALL(napi_instanceof(env, sendableInstance, sendableClass, &isInstanceOf));
    ASSERT_CHECK_CALL(napi_instanceof(env, sendableInstance, parentClass, &isInstanceOfParent));
    ASSERT_TRUE(isInstanceOf);
    ASSERT_TRUE(isInstanceOfParent);

    napi_value value = nullptr;
    char* str = new char[TEST_STR_LENGTH];
    size_t length;
    napi_value parent_instance_value;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "parentInstance", NAPI_AUTO_LENGTH, &parent_instance_value));

    // get parent instance property on instance
    ASSERT_CHECK_CALL(napi_get_property(env, sendableInstance, parent_instance_str, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_undefined);
    // set parent instance property on instance
    napi_set_property(env, sendableInstance, parent_instance_str, parent_instance_value);
    ASSERT_CHECK_CALL(napi_get_property(env, sendableInstance, parent_instance_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("parentInstance", str);

    // get parent static property from constructor
    ASSERT_CHECK_CALL(napi_get_property(env, sendableClass, parent_static_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("parentStatic", str);

    // get parent non static property from instance
    ASSERT_CHECK_CALL(napi_get_property(env, sendableInstance, parent_non_static_str, &value));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, value, str, TEST_STR_LENGTH, &length));
    ASSERT_STREQ("parentNonStatic", str);
}

HWTEST_F(NapiSendableTest, SendableClassTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value parentClass = nullptr;
    napi_status parentStatus = napi_define_sendable_class(
        env, "ParentClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 0, nullptr, nullptr, &parentClass);
    ASSERT_EQ(parentStatus, napi_ok);
    napi_value childClass = nullptr;
    napi_status childStatus = napi_define_sendable_class(
        env, "ChildClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 0, nullptr, parentClass, &childClass);
    ASSERT_EQ(childStatus, napi_ok);

    napi_value childInstance = nullptr;
    ASSERT_CHECK_CALL(napi_new_instance(env, childClass, 0, nullptr, &childInstance));

    bool isInstanceOfParent = false;
    ASSERT_CHECK_CALL(napi_instanceof(env, childInstance, parentClass, &isInstanceOfParent));
    ASSERT_TRUE(isInstanceOfParent);

    bool isInstanceOfChild = false;
    ASSERT_CHECK_CALL(napi_instanceof(env, childInstance, childClass, &isInstanceOfChild));
    ASSERT_TRUE(isInstanceOfChild);
}

/**
 * @tc.name: CreateSendableMap001
 * @tc.desc: Test napi_create_sendable_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableMap001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    res = napi_create_sendable_map(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_sendable_map(env, &result));

    bool isShared = false;
    ASSERT_CHECK_CALL(napi_is_sendable(env, result, &isShared));
    ASSERT_EQ(isShared, true);

    bool isMap = false;
    ASSERT_CHECK_CALL(napi_is_map(env, result, &isMap));
    ASSERT_EQ(isMap, true);
}

/**
 * @tc.name: CreateSendableMap002
 * @tc.desc: Test napi_create_sendable_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableMap002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_sendable_map(env, &result));

    uint32_t length = 0;
    napi_value value = nullptr;
    bool hasKey = false;

    napi_value key = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "null", NAPI_AUTO_LENGTH, &key));
    napi_value null = nullptr;
    ASSERT_CHECK_CALL(napi_get_null(env, &null));
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));

    ASSERT_CHECK_CALL(napi_map_set_property(env, result, key, null));
    ASSERT_CHECK_CALL(napi_map_get_size(env, result, &length));
    ASSERT_EQ(length, 1);
    ASSERT_CHECK_CALL(napi_map_has_property(env, result, key, &hasKey));
    ASSERT_TRUE(hasKey);
    ASSERT_CHECK_CALL(napi_map_get_property(env, result, key, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_null);

    ASSERT_CHECK_CALL(napi_map_delete_property(env, result, key));
    ASSERT_CHECK_CALL(napi_map_get_size(env, result, &length));
    ASSERT_EQ(length, 0);
    ASSERT_CHECK_CALL(napi_map_has_property(env, result, key, &hasKey));
    ASSERT_FALSE(hasKey);
    ASSERT_CHECK_CALL(napi_map_get_property(env, result, key, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_undefined);
}

/**
 * @tc.name: CreateSendableMap003
 * @tc.desc: Test napi_create_sendable_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableMap003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_sendable_map(env, &result));

    uint32_t length = 0;
    napi_value value = nullptr;
    bool hasKey = false;

    const char* key = "null";
    napi_value null = nullptr;
    ASSERT_CHECK_CALL(napi_get_null(env, &null));
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));

    ASSERT_CHECK_CALL(napi_map_set_named_property(env, result, key, null));
    ASSERT_CHECK_CALL(napi_map_get_size(env, result, &length));
    ASSERT_EQ(length, 1);
    ASSERT_CHECK_CALL(napi_map_has_named_property(env, result, key, &hasKey));
    ASSERT_TRUE(hasKey);
    ASSERT_CHECK_CALL(napi_map_get_named_property(env, result, key, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_null);

    ASSERT_CHECK_CALL(napi_map_clear(env, result));
    ASSERT_CHECK_CALL(napi_map_get_size(env, result, &length));
    ASSERT_EQ(length, 0);
    ASSERT_CHECK_CALL(napi_map_has_named_property(env, result, key, &hasKey));
    ASSERT_FALSE(hasKey);
    ASSERT_CHECK_CALL(napi_map_get_named_property(env, result, key, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_undefined);

    bool isExceptionPending = false;
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_FALSE(isExceptionPending);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    napi_status res = napi_map_set_named_property(env, result, key, object);
    ASSERT_TRUE(res == napi_pending_exception);
}

/**
 * @tc.name: CreateSendableMap004
 * @tc.desc: Test napi_create_sendable_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableMap004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_sendable_map(env, &map));

    napi_value zero = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 0, &zero));
    napi_value one = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 1, &one));

    ASSERT_CHECK_CALL(napi_map_set_property(env, map, zero, one));

    napi_value entries;
    ASSERT_CHECK_CALL(napi_map_get_entries(env, map, &entries));

    napi_value entries0;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, entries, &entries0));
    napi_value entries0Value = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, entries0, "value", &entries0Value));
    napi_value key = nullptr;
    ASSERT_CHECK_CALL(napi_get_element(env, entries0Value, 0, &key));
    int32_t nativeKey;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, key, &nativeKey));
    ASSERT_EQ(nativeKey, 0);
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_element(env, entries0Value, 1, &value));
    int32_t nativeValue;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, value, &nativeValue));
    ASSERT_EQ(nativeValue, 1);

    napi_value end;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, entries, &end));
    napi_value done = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, end, "done", &done));
    bool isDone;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, done, &isDone));
    ASSERT_TRUE(isDone);
}

/**
 * @tc.name: CreateSendableMap005
 * @tc.desc: Test napi_create_sendable_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableMap005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_sendable_map(env, &map));

    napi_value zero = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 0, &zero));
    napi_value one = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 1, &one));

    ASSERT_CHECK_CALL(napi_map_set_property(env, map, zero, one));

    napi_value keys;
    ASSERT_CHECK_CALL(napi_map_get_keys(env, map, &keys));

    napi_value keys0;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, keys, &keys0));
    napi_value key = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, keys0, "value", &key));
    int32_t nativeKey;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, key, &nativeKey));
    ASSERT_EQ(nativeKey, 0);

    napi_value end;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, keys, &end));
    napi_value done = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, end, "done", &done));
    bool isDone;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, done, &isDone));
    ASSERT_TRUE(isDone);
}

/**
 * @tc.name: CreateSendableMap006
 * @tc.desc: Test napi_create_sendable_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableMap006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_sendable_map(env, &map));

    napi_value zero = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 0, &zero));
    napi_value one = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 1, &one));

    ASSERT_CHECK_CALL(napi_map_set_property(env, map, zero, one));

    napi_value values;
    ASSERT_CHECK_CALL(napi_map_get_values(env, map, &values));

    napi_value values0;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, values, &values0));
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, values0, "value", &value));
    int32_t nativeValue;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, value, &nativeValue));
    ASSERT_EQ(nativeValue, 1);

    napi_value end;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, values, &end));
    napi_value done = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, end, "done", &done));
    bool isDone;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, done, &isDone));
    ASSERT_TRUE(isDone);
}

HWTEST_F(NapiSendableTest, WrapSendableTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_status res = napi_ok;

    napi_value testClass = nullptr;
    res = napi_define_sendable_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 0, nullptr, nullptr, &testClass);
    ASSERT_EQ(res, napi_ok);

    napi_value instanceValue = nullptr;
    res = napi_new_instance(env, testClass, 0, nullptr, &instanceValue);
    ASSERT_EQ(res, napi_ok);

    const char* testStr = "test";
    res = napi_wrap_sendable(
        env, instanceValue, (void*)testStr, [](napi_env env, void* data, void* hint) {}, nullptr);
    ASSERT_EQ(res, napi_ok);

    char* tmpTestStr = nullptr;
    res = napi_unwrap_sendable(env, instanceValue, (void**)&tmpTestStr);
    ASSERT_EQ(res, napi_ok);
    ASSERT_STREQ(testStr, tmpTestStr);

    char* tmpTestStr1 = nullptr;
    res = napi_remove_wrap_sendable(env, instanceValue, (void**)&tmpTestStr1);
    ASSERT_EQ(res, napi_ok);
    ASSERT_STREQ(testStr, tmpTestStr1);
}

HWTEST_F(NapiSendableTest, WrapSendableWithSizeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testWrapClass = nullptr;
    napi_define_sendable_class(
        env, "TestWrapClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 0, nullptr, nullptr, &testWrapClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testWrapClass, 0, nullptr, &instanceValue);

    const char* testWrapStr = "testWrapStr";
    size_t size = sizeof(*testWrapStr) / sizeof(char);
    napi_wrap_sendable_with_size(
        env, instanceValue, (void*)testWrapStr, [](napi_env env, void* data, void* hint) {}, nullptr, size);

    char* tempTestStr = nullptr;
    napi_unwrap_sendable(env, instanceValue, (void**)&tempTestStr);
    ASSERT_STREQ(testWrapStr, tempTestStr);

    char* tempTestStr1 = nullptr;
    napi_remove_wrap_sendable(env, instanceValue, (void**)&tempTestStr1);
    ASSERT_STREQ(testWrapStr, tempTestStr1);
}

/**
 * @tc.name: SharedArrayBufferTest001
 * @tc.desc: Test is shared array buffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, SharedArrayBufferTest001, testing::ext::TestSize.Level1) {
    napi_env env = (napi_env) engine_;

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    bool isSharedArrayBuffer = true;
    napi_is_shared_array_buffer(env, arrayBuffer, &isSharedArrayBuffer);
    ASSERT_EQ(isSharedArrayBuffer, false);
}


/**
 * @tc.name: CreateSendableArray001
 * @tc.desc: Test napi_create_sendable_array abnormal argument.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableArray001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    res = napi_create_sendable_array(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: CreateSendableArray002
 * @tc.desc: Test napi_create_sendable_array.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableArray002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    napi_value result = nullptr;
    res = napi_create_sendable_array(env, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isArray = false;
    res = napi_is_array(env, result, &isArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isArray, true);

    uint32_t length = 0;
    res = napi_get_array_length(env, result, &length);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(length, 0);

    napi_value undefined = nullptr;
    res = napi_get_undefined(env, &undefined);
    ASSERT_EQ(res, napi_ok);

    res = napi_set_element(env, result, 0, undefined);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: CreateSendableArrayWithLength001
 * @tc.desc: Test napi_create_sendable_array_with_length.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableArrayWithLength001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    res = napi_create_sendable_array_with_length(env, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: CreateSendableArrayWithLength002
 * @tc.desc: Test napi_create_sendable_array_with_length.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableArrayWithLength002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    napi_value result = nullptr;
    res = napi_create_sendable_array_with_length(env, 0, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isArray = false;
    res = napi_is_array(env, result, &isArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isArray, true);

    uint32_t length = 0;
    res = napi_get_array_length(env, result, &length);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(length, 0);
}

/**
 * @tc.name: CreateSendableArrayWithLength003
 * @tc.desc: Test napi_create_sendable_array_with_length.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableArrayWithLength003, testing::ext::TestSize.Level1)
{
    static uint32_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    napi_value result = nullptr;
    res = napi_create_sendable_array_with_length(env, LENGTH, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isArray = false;
    res = napi_is_array(env, result, &isArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isArray, true);

    uint32_t length = 0;
    res = napi_get_array_length(env, result, &length);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(length, LENGTH);

    napi_value value = nullptr;
    napi_value boolTrue = nullptr;
    bool ret = false;
    res = napi_get_boolean(env, true, &boolTrue);
    ASSERT_EQ(res, napi_ok);

    res = napi_set_element(env, result, 0, boolTrue);
    ASSERT_EQ(res, napi_ok);
    res = napi_get_element(env, result, 0, &value);
    ASSERT_EQ(res, napi_ok);
    res = napi_get_value_bool(env, value, &ret);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(ret, true);

    res = napi_set_element(env, result, LENGTH - 1, boolTrue);
    ASSERT_EQ(res, napi_ok);
    res = napi_get_element(env, result, LENGTH - 1, &value);
    ASSERT_EQ(res, napi_ok);
    res = napi_get_value_bool(env, value, &ret);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: CreateSendableArrayWithLength004
 * @tc.desc: Test napi_create_sendable_array_with_length.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableArrayWithLength004, testing::ext::TestSize.Level1)
{
    static uint32_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    napi_value result = nullptr;
    res = napi_create_sendable_array_with_length(env, LENGTH, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isArray = false;
    res = napi_is_array(env, result, &isArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isArray, true);

    uint32_t length = 0;
    res = napi_get_array_length(env, result, &length);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(length, LENGTH);

    napi_value value = nullptr;
    napi_value boolTrue = nullptr;
    bool ret = false;
    res = napi_get_boolean(env, true, &boolTrue);
    ASSERT_EQ(res, napi_ok);

    res = napi_get_element(env, result, 1, &value);
    ASSERT_EQ(res, napi_ok);
    napi_valuetype type = napi_undefined;
    res = napi_typeof(env, value, &type);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(napi_undefined, napi_undefined);

    res = napi_set_element(env, result, LENGTH, boolTrue);
    ASSERT_EQ(res, napi_ok);
    res = napi_get_element(env, result, LENGTH, &value);
    ASSERT_EQ(res, napi_ok);
    res = napi_get_value_bool(env, value, &ret);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(ret, true);

    res = napi_get_element(env, result, LENGTH + 1, &value);
    ASSERT_EQ(res, napi_ok);
    napi_valuetype getType = napi_undefined;
    res = napi_typeof(env, value, &getType);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(getType, napi_undefined);
}

/**
 * @tc.name: CreateSendableArrayBuffer001
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableArrayBuffer001, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    napi_value result = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, nullptr, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    void *data;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_arraybuffer(env, LENGTH, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: CreateSendableArrayBuffer002
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableArrayBuffer002, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value result = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isArrayBuffer = false;
    res = napi_is_arraybuffer(env, result, &isArrayBuffer);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isArrayBuffer, true);

    void *getData = nullptr;
    size_t length = 0;
    res = napi_get_arraybuffer_info(env, result, &getData, &length);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(length, LENGTH);
    ASSERT_EQ(getData, data);
}

/**
 * @tc.name: CreateSendableTypedArray001
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray001, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    res = napi_create_sendable_typedarray(env, napi_uint8_clamped_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_ok);

    res = napi_create_sendable_typedarray(env, napi_uint8_array, LENGTH / 2, arraybuffer, 1, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_uint8_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_int8_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_int8_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_uint16_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_uint16_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_int32_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_int32_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_uint32_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_uint32_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_float32_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);

    res = napi_create_sendable_typedarray(env, napi_float32_array, LENGTH / 2, nullptr, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: CreateSendableTypedArray002
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray002, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 128;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    napi_value result = nullptr;
    res = napi_create_sendable_typedarray(env, napi_uint8_array, LENGTH / 2, arraybuffer, OFFSET, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isTypedArray = false;
    res = napi_is_typedarray(env, result, &isTypedArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isTypedArray, true);

    napi_typedarray_type type = napi_int8_array;
    size_t length = 0;
    void *getData = nullptr;
    napi_value getArrayBuffer = nullptr;
    size_t byteOffset = 0;
    res = napi_get_typedarray_info(env, result, &type, &length, &getData, &getArrayBuffer, &byteOffset);
    ASSERT_EQ(res, napi_ok);

    bool isEqual = false;
    res = napi_strict_equals(env, arraybuffer, getArrayBuffer, &isEqual);
    ASSERT_EQ(res, napi_ok);

    ASSERT_EQ(type, napi_uint8_array);
    ASSERT_EQ(length, LENGTH / 2);
    ASSERT_EQ(reinterpret_cast<size_t>(getData), reinterpret_cast<size_t>(data) + OFFSET);
    ASSERT_EQ(isEqual, true);
    ASSERT_EQ(byteOffset, OFFSET);
}

/**
 * @tc.name: CreateSendableTypedArray003
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray003, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 128;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    napi_value result = nullptr;
    res = napi_create_sendable_typedarray(env, napi_int8_array, LENGTH / 2, arraybuffer, OFFSET, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isTypedArray = false;
    res = napi_is_typedarray(env, result, &isTypedArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isTypedArray, true);

    napi_typedarray_type type = napi_uint8_array;
    size_t length = 0;
    void *getData = nullptr;
    napi_value getArrayBuffer = nullptr;
    size_t byteOffset = 0;
    res = napi_get_typedarray_info(env, result, &type, &length, &getData, &getArrayBuffer, &byteOffset);
    ASSERT_EQ(res, napi_ok);

    bool isEqual = false;
    res = napi_strict_equals(env, arraybuffer, getArrayBuffer, &isEqual);
    ASSERT_EQ(res, napi_ok);

    ASSERT_EQ(type, napi_int8_array);
    ASSERT_EQ(length, LENGTH / 2);
    ASSERT_EQ(reinterpret_cast<size_t>(getData), reinterpret_cast<size_t>(data) + OFFSET);
    ASSERT_EQ(isEqual, true);
    ASSERT_EQ(byteOffset, OFFSET);
}

/**
 * @tc.name: CreateSendableTypedArray004
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray004, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 0;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    napi_value result = nullptr;
    res = napi_create_sendable_typedarray(env, napi_int16_array, LENGTH / 2, arraybuffer, OFFSET, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isTypedArray = false;
    res = napi_is_typedarray(env, result, &isTypedArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isTypedArray, true);

    napi_typedarray_type type = napi_uint8_array;
    size_t length = 0;
    void *getData = nullptr;
    napi_value getArrayBuffer = nullptr;
    size_t byteOffset = 0;
    res = napi_get_typedarray_info(env, result, &type, &length, &getData, &getArrayBuffer, &byteOffset);
    ASSERT_EQ(res, napi_ok);

    bool isEqual = false;
    res = napi_strict_equals(env, arraybuffer, getArrayBuffer, &isEqual);
    ASSERT_EQ(res, napi_ok);

    ASSERT_EQ(type, napi_int16_array);
    ASSERT_EQ(length, LENGTH);
    ASSERT_EQ(reinterpret_cast<size_t>(getData), reinterpret_cast<size_t>(data) + OFFSET);
    ASSERT_EQ(isEqual, true);
    ASSERT_EQ(byteOffset, OFFSET);
}

/**
 * @tc.name: CreateSendableTypedArray005
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray005, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 0;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    napi_value result = nullptr;
    res = napi_create_sendable_typedarray(env, napi_uint16_array, LENGTH / 2, arraybuffer, OFFSET, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isTypedArray = false;
    res = napi_is_typedarray(env, result, &isTypedArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isTypedArray, true);

    napi_typedarray_type type = napi_uint8_array;
    size_t length = 0;
    void *getData = nullptr;
    napi_value getArrayBuffer = nullptr;
    size_t byteOffset = 0;
    res = napi_get_typedarray_info(env, result, &type, &length, &getData, &getArrayBuffer, &byteOffset);
    ASSERT_EQ(res, napi_ok);

    bool isEqual = false;
    res = napi_strict_equals(env, arraybuffer, getArrayBuffer, &isEqual);
    ASSERT_EQ(res, napi_ok);

    ASSERT_EQ(type, napi_uint16_array);
    ASSERT_EQ(length, LENGTH);
    ASSERT_EQ(reinterpret_cast<size_t>(getData), reinterpret_cast<size_t>(data) + OFFSET);
    ASSERT_EQ(isEqual, true);
    ASSERT_EQ(byteOffset, OFFSET);
}

/**
 * @tc.name: CreateSendableTypedArray006
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray006, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 0;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH * 2, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    napi_value result = nullptr;
    res = napi_create_sendable_typedarray(env, napi_uint32_array, LENGTH / 2, arraybuffer, OFFSET, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isTypedArray = false;
    res = napi_is_typedarray(env, result, &isTypedArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isTypedArray, true);

    napi_typedarray_type type = napi_int8_array;
    size_t length = 0;
    void *getData = nullptr;
    napi_value getArrayBuffer = nullptr;
    size_t byteOffset = 0;
    res = napi_get_typedarray_info(env, result, &type, &length, &getData, &getArrayBuffer, &byteOffset);
    ASSERT_EQ(res, napi_ok);

    bool isEqual = false;
    res = napi_strict_equals(env, arraybuffer, getArrayBuffer, &isEqual);
    ASSERT_EQ(res, napi_ok);

    ASSERT_EQ(type, napi_uint32_array);
    ASSERT_EQ(length, LENGTH * 2);
    ASSERT_EQ(reinterpret_cast<size_t>(getData), reinterpret_cast<size_t>(data) + OFFSET);
    ASSERT_EQ(isEqual, true);
    ASSERT_EQ(byteOffset, OFFSET);
}

/**
 * @tc.name: CreateSendableTypedArray007
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray007, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 0;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH * 2, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    napi_value result = nullptr;
    res = napi_create_sendable_typedarray(env, napi_int32_array, LENGTH / 2, arraybuffer, OFFSET, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isTypedArray = false;
    res = napi_is_typedarray(env, result, &isTypedArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isTypedArray, true);

    napi_typedarray_type type = napi_int8_array;
    size_t length = 0;
    void *getData = nullptr;
    napi_value getArrayBuffer = nullptr;
    size_t byteOffset = 0;
    res = napi_get_typedarray_info(env, result, &type, &length, &getData, &getArrayBuffer, &byteOffset);
    ASSERT_EQ(res, napi_ok);

    bool isEqual = false;
    res = napi_strict_equals(env, arraybuffer, getArrayBuffer, &isEqual);
    ASSERT_EQ(res, napi_ok);

    ASSERT_EQ(type, napi_int32_array);
    ASSERT_EQ(length, LENGTH * 2);
    ASSERT_EQ(reinterpret_cast<size_t>(getData), reinterpret_cast<size_t>(data) + OFFSET);
    ASSERT_EQ(isEqual, true);
    ASSERT_EQ(byteOffset, OFFSET);
}

/**
 * @tc.name: CreateSendableTypedArray008
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray008, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 0;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH * 2, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    napi_value result = nullptr;
    res = napi_create_sendable_typedarray(env, napi_float32_array, LENGTH / 2, arraybuffer, OFFSET, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isTypedArray = false;
    res = napi_is_typedarray(env, result, &isTypedArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isTypedArray, true);

    napi_typedarray_type type = napi_int8_array;
    size_t length = 0;
    void *getData = nullptr;
    napi_value getArrayBuffer = nullptr;
    size_t byteOffset = 0;
    res = napi_get_typedarray_info(env, result, &type, &length, &getData, &getArrayBuffer, &byteOffset);
    ASSERT_EQ(res, napi_ok);

    bool isEqual = false;
    res = napi_strict_equals(env, arraybuffer, getArrayBuffer, &isEqual);
    ASSERT_EQ(res, napi_ok);

    ASSERT_EQ(type, napi_float32_array);
    ASSERT_EQ(length, LENGTH * 2);
    ASSERT_EQ(reinterpret_cast<size_t>(getData), reinterpret_cast<size_t>(data) + OFFSET);
    ASSERT_EQ(isEqual, true);
    ASSERT_EQ(byteOffset, OFFSET);
}

/**
 * @tc.name: CreateSendableTypedArray009
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray009, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 128;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    napi_value result = nullptr;
    res = napi_create_sendable_typedarray(env, napi_uint8_clamped_array, LENGTH / 2, arraybuffer, OFFSET, &result);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isTypedArray = false;
    res = napi_is_typedarray(env, result, &isTypedArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isTypedArray, true);

    napi_typedarray_type type = napi_uint8_array;
    size_t length = 0;
    void *getData = nullptr;
    napi_value getArrayBuffer = nullptr;
    size_t byteOffset = 0;
    res = napi_get_typedarray_info(env, result, &type, &length, &getData, &getArrayBuffer, &byteOffset);
    ASSERT_EQ(res, napi_ok);

    bool isEqual = false;
    res = napi_strict_equals(env, arraybuffer, getArrayBuffer, &isEqual);
    ASSERT_EQ(res, napi_ok);

    ASSERT_EQ(type, napi_uint8_clamped_array);
    ASSERT_EQ(length, LENGTH / 2);
    ASSERT_EQ(reinterpret_cast<size_t>(getData), reinterpret_cast<size_t>(data) + OFFSET);
    ASSERT_EQ(isEqual, true);
    ASSERT_EQ(byteOffset, OFFSET);
}

/**
 * @tc.name: CreateSendableTypedArray010
 * @tc.desc: Test napi_create_sendable_typedarray with non-sendable arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray010, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, false);

    res = napi_create_sendable_typedarray(env, napi_uint8_clamped_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: CreateSendableTypedArray011
 * @tc.desc: Test napi_create_sendable_typedarray with non-sendable arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray011, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, false);

    res = napi_create_sendable_typedarray(env, napi_uint8_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: CreateSendableTypedArray012
 * @tc.desc: Test napi_create_sendable_typedarray with non-sendable arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray012, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, false);

    res = napi_create_sendable_typedarray(env, napi_int8_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: CreateSendableTypedArray013
 * @tc.desc: Test napi_create_sendable_typedarray with non-sendable arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray013, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, false);

    res = napi_create_sendable_typedarray(env, napi_uint16_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: CreateSendableTypedArray014
 * @tc.desc: Test napi_create_sendable_typedarray with non-sendable arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray014, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, false);

    res = napi_create_sendable_typedarray(env, napi_int16_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: CreateSendableTypedArray015
 * @tc.desc: Test napi_create_sendable_typedarray with non-sendable arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray015, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, false);

    res = napi_create_sendable_typedarray(env, napi_uint32_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: CreateSendableTypedArray016
 * @tc.desc: Test napi_create_sendable_typedarray with non-sendable arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray016, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, false);

    res = napi_create_sendable_typedarray(env, napi_int32_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: CreateSendableTypedArray017
 * @tc.desc: Test napi_create_sendable_typedarray with non-sendable arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray017, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, false);

    res = napi_create_sendable_typedarray(env, napi_float32_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: CreateSendableTypedArray018
 * @tc.desc: Test napi_create_sendable_arraybuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, CreateSendableTypedArray018, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 0;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    void* data;
    napi_value arraybuffer = nullptr;
    ASSERT_CHECK_CALL(napi_create_sendable_arraybuffer(env, LENGTH, &data, &arraybuffer));

    for (int type = napi_int8_array; type <= napi_float32_array; ++type) {
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_sendable_typedarray(env, static_cast<napi_typedarray_type>(type), LENGTH / 4,
                                                          arraybuffer, OFFSET, &result));
        bool isTypedArray = false;
        ASSERT_CHECK_CALL(napi_is_typedarray(env, result, &isTypedArray));
        ASSERT_EQ(isTypedArray, true);
    }
}

/**
 * @tc.name: IsDetachedSendableArrayBufferTest001
 * @tc.desc: Test is DetachedSendableArrayBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, IsDetachedSendableArrayBufferTest001, testing::ext::TestSize.Level1)
{
    static constexpr size_t arrayBufferSize = 1024;
    napi_env env = (napi_env)engine_;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    napi_create_sendable_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    auto out = napi_detach_arraybuffer(env, arrayBuffer);
    if (out == napi_ok) {
        arrayBufferPtr = nullptr;
    }
    ASSERT_EQ(out, napi_ok);
}

static napi_value SendableGetter(napi_env env, napi_callback_info info)
{
    napi_value res;
    napi_get_boolean(env, false, &res);
    return res;
}

static napi_value SendableSetter(napi_env env, napi_callback_info info)
{
    napi_value res;
    napi_get_boolean(env, true, &res);
    return res;
}

HWTEST_F(NapiSendableTest, CreateSendableObjectWithPropertiesTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value excep;
    ASSERT_CHECK_CALL(napi_get_and_clear_last_exception(env, &excep));
    napi_value val_false;
    napi_value val_true;
    ASSERT_CHECK_CALL(napi_get_boolean(env, false, &val_false));
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &val_true));
    napi_property_descriptor desc1[] = {
        DECLARE_NAPI_DEFAULT_PROPERTY("x", val_true),
    };
    napi_value obj1;
    ASSERT_CHECK_CALL(napi_create_sendable_object_with_properties(env, 1, desc1, &obj1));
    napi_value obj2;
    napi_property_descriptor desc2[] = {
        DECLARE_NAPI_DEFAULT_PROPERTY("a", val_false),
        DECLARE_NAPI_GETTER_SETTER("b", SendableGetter, SendableSetter),
        DECLARE_NAPI_DEFAULT_PROPERTY("c", obj1),
    };
    ASSERT_CHECK_CALL(napi_create_sendable_object_with_properties(env, 3, desc2, &obj2));
    ASSERT_CHECK_VALUE_TYPE(env, obj1, napi_object);
    ASSERT_CHECK_VALUE_TYPE(env, obj2, napi_object);
    auto checkPropertyEqualsTo = [env](napi_value obj, const char* keyStr, napi_value expect) -> bool {
        napi_value result;
        napi_get_named_property(env, obj, keyStr, &result);
        bool equal = false;
        napi_strict_equals(env, result, expect, &equal);
        return equal;
    };
    // get obj1.x == true
    ASSERT_TRUE(checkPropertyEqualsTo(obj1, "x", val_true));
    // set obj1.x = false
    ASSERT_CHECK_CALL(napi_set_named_property(env, obj1, "x", val_false));
    // get obj1.x == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj1, "x", val_false));
    // get obj2.a == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "a", val_false));
    // get obj2.b == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "b", val_false));
    // set obj2.b = true (useless)
    ASSERT_CHECK_CALL(napi_set_named_property(env, obj2, "b", val_true));
    // get obj2.b == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "b", val_false));
    // get obj2.c == obj1
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "c", obj1));
    // get obj2.c.x == false
    napi_value val_res;
    ASSERT_CHECK_CALL(napi_get_named_property(env, obj2, "c", &val_res));
    ASSERT_TRUE(checkPropertyEqualsTo(val_res, "x", val_false));
}

bool g_funcCalled = false;
static napi_value Func(napi_env env, napi_callback_info info)
{
    napi_value boolTrue = nullptr;
    napi_value boolFalse = nullptr;
    napi_get_boolean(env, true, &boolTrue);
    napi_get_boolean(env, false, &boolFalse);

    if (g_funcCalled) {
        return boolFalse;
    }

    char *data = nullptr;
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    napi_unwrap_sendable(env, thisVar, reinterpret_cast<void**>(&data));
    if (*data == 'a') {
        g_funcCalled = true;
        delete [] data;
    }

    return boolTrue;
}

HWTEST_F(NapiSendableTest, CreateSendableObjectWithPropertiesTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value excep;
    ASSERT_CHECK_CALL(napi_get_and_clear_last_exception(env, &excep));

    napi_value obj2;
    napi_property_descriptor desc2[] = {
        DECLARE_NAPI_FUNCTION("func", Func),
    };
    ASSERT_CHECK_CALL(napi_create_sendable_object_with_properties(env, 1, desc2, &obj2));
    bool isShared = false;
    napi_is_sendable(env, obj2, &isShared);
    ASSERT_EQ(isShared, true);

    auto data = new char[2];
    *data = 'a';
    *(data + 1) = 0;
    napi_wrap_sendable(env, obj2, data, nullptr, nullptr);
    ASSERT_CHECK_VALUE_TYPE(env, obj2, napi_object);
    napi_value func = nullptr;
    napi_value ret = nullptr;
    napi_get_named_property(env, obj2, "func", &func);
    ASSERT_EQ(g_funcCalled, false);
    napi_call_function(env, obj2, func, 0, nullptr, &ret);
    bool resultBool = false;
    napi_get_value_bool(env, ret, &resultBool);
    ASSERT_EQ(resultBool, true);
    ASSERT_EQ(g_funcCalled, true);
}

/**
 * @tc.name: SerializeDeSerializeSendableDataTest001
 * @tc.desc: Test serialize & deserialize sendableArray.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, SerializeDeSerializeSendableDataTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_status res = napi_ok;
    napi_value excep;
    ASSERT_CHECK_CALL(napi_get_and_clear_last_exception(env, &excep));
    napi_value num = nullptr;
    uint32_t value = 1000;
    res = napi_create_uint32(env, value, &num);
    ASSERT_EQ(res, napi_ok);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_DEFAULT_PROPERTY("a", num),
    };
    napi_value obj;
    ASSERT_CHECK_CALL(napi_create_sendable_object_with_properties(env, 1, desc, &obj));
    ASSERT_CHECK_VALUE_TYPE(env, obj, napi_object);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    res = napi_serialize(env, obj, undefined, undefined, &data);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(res, napi_ok);

    napi_value result1 = nullptr;
    res = napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_object);

    napi_value number1 = nullptr;;
    napi_get_named_property(env, result1, "a", &number1);
    ASSERT_CHECK_VALUE_TYPE(env, number1, napi_number);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeSendableDataTest002
 * @tc.desc: Test serialize & deserialize sendableArray.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, SerializeDeSerializeSendableDataTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env) engine_;

    napi_status res = napi_ok;
    napi_value sendableArray = nullptr;
    napi_create_sendable_array(env, &sendableArray);
    ASSERT_NE(sendableArray, nullptr);
    bool isShared = false;
    res = napi_is_sendable(env, sendableArray, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);
    bool isArray = false;
    napi_is_array(env, sendableArray, &isArray);
    ASSERT_TRUE(isArray);
    for (size_t i = 0; i < 10; i++) {
        napi_value num = nullptr;
        napi_create_uint32(env, i, &num);
        napi_set_element(env, sendableArray, i, num);
    }

    uint32_t length = 0;
    res = napi_get_array_length(env, sendableArray, &length);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(length, 10u);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    res = napi_serialize(env, sendableArray, undefined, undefined, &data);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(res, napi_ok);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_object);

    res = napi_get_array_length(env, result1, &length);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(length, 10u);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeSendableDataTest003
 * @tc.desc: Test serialize & deserialize sendableArrayWithLength.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, SerializeDeSerializeSendableDataTest003, testing::ext::TestSize.Level1)
{
    static uint32_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    napi_value sendableArray = nullptr;
    res = napi_create_sendable_array_with_length(env, LENGTH, &sendableArray);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, sendableArray, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);
    bool isArray = false;
    napi_is_array(env, sendableArray, &isArray);
    ASSERT_TRUE(isArray);
    for (size_t i = 0; i < 100; i++) {
        napi_value num = nullptr;
        napi_create_uint32(env, i, &num);
        napi_set_element(env, sendableArray, i, num);
    }

    uint32_t length = 0;
    res = napi_get_array_length(env, sendableArray, &length);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(length, 1024);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    res = napi_serialize(env, sendableArray, undefined, undefined, &data);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(res, napi_ok);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_object);

    res = napi_get_array_length(env, result1, &length);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(length, 1024);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeSendableDataTest004
 * @tc.desc: Test serialize & deserialize sendableTypedArray.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, SerializeDeSerializeSendableDataTest004, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    static size_t OFFSET = 128;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *tmpData;
    napi_value sendableArraybuffer = nullptr;
    napi_value sendableTypedarray = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &tmpData, &sendableArraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, sendableArraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    res = napi_create_sendable_typedarray(env, napi_int8_array, LENGTH / 2,
                                          sendableArraybuffer, OFFSET, &sendableTypedarray);
    ASSERT_EQ(res, napi_ok);

    isShared = false;
    res = napi_is_sendable(env, sendableArraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isTypedArray = false;
    res = napi_is_typedarray(env, sendableTypedarray, &isTypedArray);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isTypedArray, true);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    res = napi_serialize(env, sendableTypedarray, undefined, undefined, &data);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(res, napi_ok);

    napi_value result = nullptr;
    res = napi_deserialize(env, data, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);

    napi_typedarray_type type = napi_uint8_array;
    size_t length = 0;
    void *getData = nullptr;
    napi_value getArrayBuffer = nullptr;
    size_t byteOffset = 0;
    res = napi_get_typedarray_info(env, result, &type, &length, &getData, &getArrayBuffer, &byteOffset);
    ASSERT_EQ(res, napi_ok);

    ASSERT_EQ(type, napi_int8_array);
    ASSERT_EQ(length, LENGTH / 2);
    ASSERT_EQ(reinterpret_cast<size_t>(getData), reinterpret_cast<size_t>(tmpData) + OFFSET);
    ASSERT_EQ(byteOffset, OFFSET);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeSendableDataTest005
 * @tc.desc: Test serialize & deserialize sendableArrayBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiSendableTest, SerializeDeSerializeSendableDataTest005, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *tmpData;
    napi_value sendableBuffer = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &tmpData, &sendableBuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, sendableBuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);

    bool isArrayBuffer = false;
    res = napi_is_arraybuffer(env, sendableBuffer, &isArrayBuffer);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isArrayBuffer, true);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    res = napi_serialize(env, sendableBuffer, undefined, undefined, &data);
    ASSERT_EQ(res, napi_ok);
    ASSERT_NE(data, nullptr);

    napi_delete_serialization_data(env, data);
}