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

#include <map>
#include <string>
#include <vector>
#include <ctime>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr uint32_t MODULE_VERSION = 1;
constexpr uint32_t NO_MODULE_FLAGS = 0;
constexpr int HTTP_OK = 200;
constexpr int HTTP_NOT_FOUND = 404;
constexpr int HTTP_SERVER_ERROR = 500;
constexpr int HTTP_SUCCESS_MIN = 200;
constexpr int HTTP_SUCCESS_MAX = 299;
constexpr size_t GET_ARG_COUNT = 2;
constexpr size_t POST_ARG_COUNT = 3;
constexpr size_t REQUEST_ARG_COUNT = 2;
constexpr size_t CALLBACK_ARG_COUNT = 2;
constexpr size_t URL_ARG_INDEX = 0;
constexpr size_t OPTIONS_ARG_INDEX = 0;
constexpr size_t CALLBACK_ARG_INDEX = 1;
constexpr size_t BODY_ARG_INDEX = 1;
constexpr size_t HEADERS_ARG_INDEX = 2;

struct HttpRequestContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
    std::string url;
    std::string method;
    std::map<std::string, std::string> headers;
    std::string body;
    int statusCode = 0;
    std::string responseData;
    std::string errorMessage;
    bool useCallback = false;
};

struct HttpResult {
    int statusCode;
    std::string data;
    std::map<std::string, std::string> headers;
};

std::string Trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

bool ParseHeaders(napi_env env, napi_value headersObj, std::map<std::string, std::string>& headers)
{
    napi_valuetype type;
    if (napi_typeof(env, headersObj, &type) != napi_ok || type != napi_object) {
        return false;
    }

    napi_value keys;
    if (napi_get_property_names(env, headersObj, &keys) != napi_ok) {
        return false;
    }

    uint32_t keyCount = 0;
    if (napi_get_array_length(env, keys, &keyCount) != napi_ok) {
        return false;
    }

    for (uint32_t i = 0; i < keyCount; i++) {
        napi_value key;
        if (napi_get_element(env, keys, i, &key) != napi_ok) {
            continue;
        }

        size_t keyLength = 0;
        napi_status status = napi_get_value_string_utf8(env, key, nullptr, 0, &keyLength);
        if (status != napi_ok || keyLength == 0) {
            continue;
        }

        std::vector<char> keyBuffer(keyLength + 1);
        if (napi_get_value_string_utf8(env, key, keyBuffer.data(), keyLength + 1, &keyLength) != napi_ok) {
            continue;
        }

        napi_value value;
        if (napi_get_property(env, headersObj, key, &value) != napi_ok) {
            continue;
        }

        size_t valueLength = 0;
        status = napi_get_value_string_utf8(env, value, nullptr, 0, &valueLength);
        if (status != napi_ok) {
            continue;
        }

        std::vector<char> valueBuffer(valueLength + 1);
        if (napi_get_value_string_utf8(env, value, valueBuffer.data(), valueLength + 1, &valueLength) == napi_ok) {
            headers[std::string(keyBuffer.data())] = std::string(valueBuffer.data());
        }
    }

    return true;
}

bool IsSuccessStatus(int statusCode)
{
    return statusCode >= HTTP_SUCCESS_MIN && statusCode <= HTTP_SUCCESS_MAX;
}

bool GetStringFromValue(napi_env env, napi_value value, std::string& out)
{
    size_t length = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &length) != napi_ok) {
        return false;
    }

    std::vector<char> buffer(length + 1);
    if (napi_get_value_string_utf8(env, value, buffer.data(), length + 1, &length) != napi_ok) {
        return false;
    }

    out.assign(buffer.data(), length);
    return true;
}

void GetStringProperty(napi_env env, napi_value obj, const char* name, std::string& out)
{
    napi_value value;
    if (napi_get_named_property(env, obj, name, &value) != napi_ok) {
        return;
    }

    std::string parsed;
    if (GetStringFromValue(env, value, parsed)) {
        out = parsed;
    }
}

HttpResult SimulateHttpRequest(const std::string& url, const std::string& method,
                               const std::map<std::string, std::string>& headers,
                               const std::string& body)
{
    HttpResult result;

    if (url.empty()) {
        result.statusCode = HTTP_SERVER_ERROR;
        result.data = "{\"error\": \"Invalid URL\"}";
        return result;
    }

    if (method != "GET" && method != "POST" && method != "PUT" && method != "DELETE") {
        result.statusCode = HTTP_SERVER_ERROR;
        result.data = "{\"error\": \"Unsupported method\"}";
        return result;
    }

    result.statusCode = HTTP_OK;
    result.headers["Content-Type"] = "application/json";
    result.headers["Server"] = "Simulated-NAPI-Server/1.0";

    std::string jsonResponse = "{";
    jsonResponse += "\"url\": \"" + url + "\",";
    jsonResponse += "\"method\": \"" + method + "\",";
    jsonResponse += "\"status\": " + std::to_string(HTTP_OK) + ",";
    time_t now = time(nullptr);
    if (now == static_cast<time_t>(-1)) {
        now = 0;
    }
    jsonResponse += "\"timestamp\": " + std::to_string(static_cast<long>(now));

    if (!body.empty()) {
        jsonResponse += ",\"requestBody\": \"" + body + "\"";
    }

    jsonResponse += ",\"message\": \"Request processed successfully\"}";

    result.data = jsonResponse;

    return result;
}

void ExecuteHttpRequest(napi_env env, void* data)
{
    auto context = static_cast<HttpRequestContext*>(data);

    HttpResult result = SimulateHttpRequest(context->url, context->method,
                                            context->headers, context->body);

    context->statusCode = result.statusCode;
    context->responseData = result.data;
}

void CompleteHttpRequest(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<HttpRequestContext*>(data);

    napi_value undefined;
    napi_get_undefined(env, &undefined);

    napi_value resultObj;
    napi_create_object(env, &resultObj);

    napi_value statusCodeValue;
    napi_create_int32(env, context->statusCode, &statusCodeValue);
    napi_set_named_property(env, resultObj, "statusCode", statusCodeValue);

    napi_value dataValue;
    napi_create_string_utf8(env, context->responseData.c_str(),
                           context->responseData.length(), &dataValue);
    napi_set_named_property(env, resultObj, "data", dataValue);

    napi_value successValue;
    napi_get_boolean(env, IsSuccessStatus(context->statusCode), &successValue);
    napi_set_named_property(env, resultObj, "success", successValue);

    if (context->useCallback && context->callback != nullptr) {
        napi_value callback;
        napi_get_reference_value(env, context->callback, &callback);

        napi_value argv[CALLBACK_ARG_COUNT];
        if (IsSuccessStatus(context->statusCode)) {
            napi_get_null(env, &argv[0]);
            argv[1] = resultObj;
        } else {
            napi_value error;
            napi_create_string_utf8(env, context->responseData.c_str(),
                context->responseData.length(), &argv[0]);
            argv[1] = undefined;
        }

        napi_call_function(env, undefined, callback, CALLBACK_ARG_COUNT, argv, nullptr);
        napi_delete_reference(env, context->callback);
    } else if (context->deferred != nullptr) {
        if (IsSuccessStatus(context->statusCode)) {
            napi_resolve_deferred(env, context->deferred, resultObj);
        } else {
            napi_value error;
            napi_create_string_utf8(env, context->responseData.c_str(),
                context->responseData.length(), &error);
            napi_reject_deferred(env, context->deferred, error);
        }
    }

    napi_delete_async_work(env, context->work);
    delete context;
}

napi_value HttpGet(napi_env env, napi_callback_info info)
{
    size_t argc = GET_ARG_COUNT;
    napi_value argv[GET_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "URL is required");
        return nullptr;
    }

    size_t urlLength = 0;
    if (napi_get_value_string_utf8(env, argv[URL_ARG_INDEX], nullptr, 0, &urlLength) != napi_ok || urlLength == 0) {
        napi_throw_error(env, nullptr, "Invalid URL");
        return nullptr;
    }

    auto context = new HttpRequestContext();
    context->method = "GET";

    std::vector<char> urlBuffer(urlLength + 1);
    napi_get_value_string_utf8(env, argv[URL_ARG_INDEX], urlBuffer.data(), urlLength + 1, &urlLength);
    context->url = std::string(urlBuffer.data());

    napi_value promise;
    napi_value resourceName;
    napi_create_string_utf8(env, "HttpGet", NAPI_AUTO_LENGTH, &resourceName);

    if (argc >= GET_ARG_COUNT) {
        napi_valuetype type;
        if (napi_typeof(env, argv[CALLBACK_ARG_INDEX], &type) == napi_ok && type == napi_function) {
            context->useCallback = true;
            napi_create_reference(env, argv[CALLBACK_ARG_INDEX], 1, &context->callback);
            napi_get_undefined(env, &promise);
        } else if (type == napi_object) {
            ParseHeaders(env, argv[CALLBACK_ARG_INDEX], context->headers);
            napi_create_promise(env, &context->deferred, &promise);
        } else {
            napi_create_promise(env, &context->deferred, &promise);
        }
    } else {
        napi_create_promise(env, &context->deferred, &promise);
    }

    napi_create_async_work(env, nullptr, resourceName, ExecuteHttpRequest,
        CompleteHttpRequest, context, &context->work);
    napi_queue_async_work(env, context->work);

    return promise;
}

napi_value HttpPost(napi_env env, napi_callback_info info)
{
    size_t argc = POST_ARG_COUNT;
    napi_value argv[POST_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "URL and body are required");
        return nullptr;
    }

    size_t urlLength = 0;
    if (napi_get_value_string_utf8(env, argv[URL_ARG_INDEX], nullptr, 0, &urlLength) != napi_ok || urlLength == 0) {
        napi_throw_error(env, nullptr, "Invalid URL");
        return nullptr;
    }

    auto context = new HttpRequestContext();
    context->method = "POST";

    std::vector<char> urlBuffer(urlLength + 1);
    napi_get_value_string_utf8(env, argv[URL_ARG_INDEX], urlBuffer.data(), urlLength + 1, &urlLength);
    context->url = std::string(urlBuffer.data());

    size_t bodyLength = 0;
    napi_get_value_string_utf8(env, argv[BODY_ARG_INDEX], nullptr, 0, &bodyLength);
    std::vector<char> bodyBuffer(bodyLength + 1);
    napi_get_value_string_utf8(env, argv[BODY_ARG_INDEX], bodyBuffer.data(), bodyLength + 1, &bodyLength);
    context->body = std::string(bodyBuffer.data());

    if (argc >= POST_ARG_COUNT) {
        napi_valuetype type;
        if (napi_typeof(env, argv[HEADERS_ARG_INDEX], &type) == napi_ok && type == napi_object) {
            ParseHeaders(env, argv[HEADERS_ARG_INDEX], context->headers);
        }
    }

    napi_value promise;
    napi_value resourceName;
    napi_create_string_utf8(env, "HttpPost", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_promise(env, &context->deferred, &promise);

    napi_create_async_work(env, nullptr, resourceName, ExecuteHttpRequest,
        CompleteHttpRequest, context, &context->work);
    napi_queue_async_work(env, context->work);

    return promise;
}

napi_value HttpRequest(napi_env env, napi_callback_info info)
{
    size_t argc = REQUEST_ARG_COUNT;
    napi_value argv[REQUEST_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "Options object is required");
        return nullptr;
    }

    napi_valuetype type;
    if (napi_typeof(env, argv[OPTIONS_ARG_INDEX], &type) != napi_ok || type != napi_object) {
        napi_throw_error(env, nullptr, "First argument must be an object");
        return nullptr;
    }

    auto context = new HttpRequestContext();

    GetStringProperty(env, argv[OPTIONS_ARG_INDEX], "url", context->url);
    GetStringProperty(env, argv[OPTIONS_ARG_INDEX], "method", context->method);
    if (context->method.empty()) {
        context->method = "GET";
    }
    GetStringProperty(env, argv[OPTIONS_ARG_INDEX], "body", context->body);

    napi_value headersValue;
    if (napi_get_named_property(env, argv[OPTIONS_ARG_INDEX], "headers", &headersValue) == napi_ok) {
        ParseHeaders(env, headersValue, context->headers);
    }

    napi_value promise;
    napi_value resourceName;
    napi_create_string_utf8(env, "HttpRequest", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_promise(env, &context->deferred, &promise);

    napi_create_async_work(env, nullptr, resourceName, ExecuteHttpRequest,
        CompleteHttpRequest, context, &context->work);
    napi_queue_async_work(env, context->work);

    return promise;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("get", HttpGet),
        DECLARE_NAPI_FUNCTION("post", HttpPost),
        DECLARE_NAPI_FUNCTION("request", HttpRequest),
    };

    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

}

extern "C" __attribute__((visibility("default"))) napi_value NAPI_Register(napi_env env, napi_value exports)
{
    return Init(env, exports);
}
