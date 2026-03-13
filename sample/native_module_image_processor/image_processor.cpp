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

#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr uint32_t MODULE_VERSION = 1;
constexpr uint32_t NO_MODULE_FLAGS = 0;
constexpr int MAX_IMAGE_DIMENSION = 8192;
constexpr int MIN_IMAGE_DIMENSION = 1;

enum class ImageFormat {
    RGB,
    RGBA,
    GRAYSCALE
};

struct ImageData {
    int width;
    int height;
    ImageFormat format;
    std::vector<uint8_t> pixels;
};

struct ImageProcessContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
    std::unique_ptr<ImageData> inputImage;
    std::unique_ptr<ImageData> outputImage;
    std::string errorMessage;
    bool useCallback = false;
    int operationType = 0;
    int param1 = 0;
    int param2 = 0;
};

bool ValidateImageDimensions(int width, int height)
{
    return width >= MIN_IMAGE_DIMENSION && width <= MAX_IMAGE_DIMENSION &&
           height >= MIN_IMAGE_DIMENSION && height <= MAX_IMAGE_DIMENSION;
}

int GetBytesPerPixel(ImageFormat format)
{
    switch (format) {
        case ImageFormat::RGB:
            return 3;
        case ImageFormat::RGBA:
            return 4;
        case ImageFormat::GRAYSCALE:
            return 1;
        default:
            return 3;
    }
}

std::unique_ptr<ImageData> CreateImage(int width, int height, ImageFormat format)
{
    if (!ValidateImageDimensions(width, height)) {
        return nullptr;
    }

    auto image = std::make_unique<ImageData>();
    image->width = width;
    image->height = height;
    image->format = format;

    int bytesPerPixel = GetBytesPerPixel(format);
    size_t pixelCount = static_cast<size_t>(width) * height * bytesPerPixel;
    image->pixels.resize(pixelCount, 0);

    return image;
}

std::unique_ptr<ImageData> CloneImage(const ImageData* source)
{
    if (!source) {
        return nullptr;
    }

    auto clone = std::make_unique<ImageData>();
    clone->width = source->width;
    clone->height = source->height;
    clone->format = source->format;
    clone->pixels = source->pixels;

    return clone;
}

std::unique_ptr<ImageData> ResizeImage(const ImageData* source, int newWidth, int newHeight)
{
    if (!source || !ValidateImageDimensions(newWidth, newHeight)) {
        return nullptr;
    }

    auto result = CreateImage(newWidth, newHeight, source->format);
    if (!result) {
        return nullptr;
    }

    int bytesPerPixel = GetBytesPerPixel(source->format);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            int srcX = (x * source->width) / newWidth;
            int srcY = (y * source->height) / newHeight;

            for (int c = 0; c < bytesPerPixel; c++) {
                size_t srcIndex = (srcY * source->width + srcX) * bytesPerPixel + c;
                size_t dstIndex = (y * newWidth + x) * bytesPerPixel + c;
                result->pixels[dstIndex] = source->pixels[srcIndex];
            }
        }
    }

    return result;
}

std::unique_ptr<ImageData> ConvertFormat(const ImageData* source, ImageFormat targetFormat)
{
    if (!source || source->format == targetFormat) {
        return CloneImage(source);
    }

    auto result = CreateImage(source->width, source->height, targetFormat);
    if (!result) {
        return nullptr;
    }

    for (int y = 0; y < source->height; y++) {
        for (int x = 0; x < source->width; x++) {
            size_t srcIndex = (y * source->width + x) * GetBytesPerPixel(source->format);
            size_t dstIndex = (y * result->width + x) * GetBytesPerPixel(targetFormat);

            if (source->format == ImageFormat::RGB && targetFormat == ImageFormat::RGBA) {
                result->pixels[dstIndex] = source->pixels[srcIndex];
                result->pixels[dstIndex + 1] = source->pixels[srcIndex + 1];
                result->pixels[dstIndex + 2] = source->pixels[srcIndex + 2];
                result->pixels[dstIndex + 3] = 255;
            } else if (source->format == ImageFormat::RGBA && targetFormat == ImageFormat::RGB) {
                result->pixels[dstIndex] = source->pixels[srcIndex];
                result->pixels[dstIndex + 1] = source->pixels[srcIndex + 1];
                result->pixels[dstIndex + 2] = source->pixels[srcIndex + 2];
            } else if (source->format == ImageFormat::RGB && targetFormat == ImageFormat::GRAYSCALE) {
                uint8_t r = source->pixels[srcIndex];
                uint8_t g = source->pixels[srcIndex + 1];
                uint8_t b = source->pixels[srcIndex + 2];
                result->pixels[dstIndex] = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
            } else if (source->format == ImageFormat::RGBA && targetFormat == ImageFormat::GRAYSCALE) {
                uint8_t r = source->pixels[srcIndex];
                uint8_t g = source->pixels[srcIndex + 1];
                uint8_t b = source->pixels[srcIndex + 2];
                result->pixels[dstIndex] = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
            } else if (source->format == ImageFormat::GRAYSCALE && targetFormat == ImageFormat::RGB) {
                uint8_t gray = source->pixels[srcIndex];
                result->pixels[dstIndex] = gray;
                result->pixels[dstIndex + 1] = gray;
                result->pixels[dstIndex + 2] = gray;
            } else if (source->format == ImageFormat::GRAYSCALE && targetFormat == ImageFormat::RGBA) {
                uint8_t gray = source->pixels[srcIndex];
                result->pixels[dstIndex] = gray;
                result->pixels[dstIndex + 1] = gray;
                result->pixels[dstIndex + 2] = gray;
                result->pixels[dstIndex + 3] = 255;
            }
        }
    }

    return result;
}

std::unique_ptr<ImageData> ApplyGrayscale(const ImageData* source)
{
    return ConvertFormat(source, ImageFormat::GRAYSCALE);
}

std::unique_ptr<ImageData> AdjustBrightness(const ImageData* source, int adjustment)
{
    if (!source) {
        return nullptr;
    }

    auto result = CloneImage(source);
    if (!result) {
        return nullptr;
    }

    int bytesPerPixel = GetBytesPerPixel(source->format);

    for (size_t i = 0; i < result->pixels.size(); i++) {
        int newValue = static_cast<int>(result->pixels[i]) + adjustment;
        result->pixels[i] = static_cast<uint8_t>(std::max(0, std::min(255, newValue)));
    }

    return result;
}

std::unique_ptr<ImageData> AdjustContrast(const ImageData* source, float factor)
{
    if (!source) {
        return nullptr;
    }

    auto result = CloneImage(source);
    if (!result) {
        return nullptr;
    }

    for (size_t i = 0; i < result->pixels.size(); i++) {
        int newValue = static_cast<int>((result->pixels[i] - 128) * factor + 128);
        result->pixels[i] = static_cast<uint8_t>(std::max(0, std::min(255, newValue)));
    }

    return result;
}

std::unique_ptr<ImageData> ApplyInvert(const ImageData* source)
{
    if (!source) {
        return nullptr;
    }

    auto result = CloneImage(source);
    if (!result) {
        return nullptr;
    }

    for (size_t i& : result->pixels) {
        i = 255 - i;
    }

    return result;
}

void ExecuteImageProcess(napi_env env, void* data)
{
    auto context = static_cast<ImageProcessContext*>(data);

    if (!context->inputImage) {
        context->errorMessage = "Invalid input image";
        return;
    }

    switch (context->operationType) {
        case 1:
            context->outputImage = ResizeImage(context->inputImage.get(), context->param1, context->param2);
            break;
        case 2:
            context->outputImage = ConvertFormat(context->inputImage.get(), static_cast<ImageFormat>(context->param1));
            break;
        case 3:
            context->outputImage = ApplyGrayscale(context->inputImage.get());
            break;
        case 4:
            context->outputImage = AdjustBrightness(context->inputImage.get(), context->param1);
            break;
        case 5:
            context->outputImage = AdjustContrast(context->inputImage.get(), context->param1 / 100.0f);
            break;
        case 6:
            context->outputImage = ApplyInvert(context->inputImage.get());
            break;
        default:
            context->errorMessage = "Unknown operation";
            break;
    }

    if (!context->outputImage) {
        context->errorMessage = "Image processing failed";
    }
}

void CompleteImageProcess(napi_env env, napi_status status, void* data)
{
    auto context = = static_cast<ImageProcessContext*>(data);

    napi_value undefined;
    napi_get_undefined(env, &undefined);

    napi_value resultObj = nullptr;

    if (context->outputImage) {
        napi_create_object(env, &resultObj);

        napi_value widthValue;
        napi_create_int32(env, context->outputImage->width, &widthValue);
        napi_set_named_property(env, resultObj, "width", widthValue);

        napi_value heightValue;
        napi_create_int32(env, context->outputImage->height, &heightValue);
        napi_set_named_property(env, resultObj, "height", heightValue);

        napi_value formatValue;
        napi_create_int32(env, static_cast<int>(context->outputImage->format), &formatValue);
        napi_set_named_property(env, resultObj, "format", formatValue);

        napi_value pixelsArray;
        void* arrayData = nullptr;
        napi_create_arraybuffer(env, context->outputImage->pixels.size(), &arrayData, &pixelsArray);
        memcpy(arrayData, context->outputImage->pixels.data(), context->outputImage->pixels.size());
        napi_set_named_property(env, resultObj, "pixels", pixelsArray);
    }

    if (context->useCallback && context->callback != nullptr) {
        napi_value callback;
        napi_get_reference_value(env, context->callback, &callback);

        napi_value argv[2];
        if (context->outputImage) {
            napi_get_null(env, &argv[0]);
            argv[1] = resultObj;
        } else {
            napi_value error;
            napi_create_string_utf8(env, context->errorMessage.c_str(),
                                   context->errorMessage.length(), &argv[0]);
            argv[1] = undefined;
        }

        napi_call_function(env, undefined, callback, 2, argv, nullptr);
        napi_delete_reference(env, context->callback);
    } else if (context->deferred != nullptr) {
        if (context->outputImage) {
            napi_resolve_deferred(env, context->deferred, resultObj);
        } else {
            napi_value error;
            napi_create_string_utf8(env, context->errorMessage.c_str(),
                                   context->errorMessage.length(), &error);
            napi_reject_deferred(env, context->deferred, error);
        }
    }

    napi_delete_async_work(env, context->work);
    delete context;
}

std::unique_ptr<ImageData> ParseImageFromNAPI(napi_env env, napi_value imageObj)
{
    napi_value widthValue;
    if (napi_get_named_property(env, imageObj, "width", &widthValue) != napi_ok) {
        return nullptr;
    }

    int32_t width = 0;
    if (napi_get_value_int32(env, widthValue, &width) != napi_ok) {
        return nullptr;
    }

    napi_value heightValue;
    if (napi_get_named_property(env, imageObj, "height", &heightValue) != napi_ok) {
        return nullptr;
    }

    int32_t height = 0;
    if (napi_get_value_int32(env, heightValue, &height) != napi_ok) {
        return nullptr;
    }

    napi_value formatValue;
    if (napi_get_named_property(env, imageObj, "format", &formatValue) != napi_ok) {
        return nullptr;
    }

    int32_t format = 0;
    if (napi_get_value_int32(env, formatValue, &format) != napi) {
        return nullptr;
    }

    napi_value pixelsValue;
    if (napi_get_named_property(env, imageObj, "pixels", &pixelsValue) != napi_ok) {
        return nullptr;
    }

    bool isArrayBuffer = false;
    if (napi_is_arraybuffer(env, pixelsValue, &isArrayBuffer) != napi_ok || !isArrayBuffer) {
        return nullptr;
    }

    void* arrayData = nullptr;
    size_t arrayLength = 0;
    if (napi_get_arraybuffer_info(env, pixelsValue, &arrayData, &arrayLength) != napi_ok) {
        return nullptr;
    }

    auto image = CreateImageariwidth, height, static_cast<ImageFormat>(format));
    if (!image) {
        return nullptr;
    }

    if (arrayLength == image->pixels.size()) {
        memcpy(image->pixels.data(), arrayData, arrayLength);
    }

    return image;
}

napi_value Resize(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 3) {
        napi_throw_error(env, nullptr, "Image, width, and height are required");
        return nullptr;
    }

    auto context = new ImageProcessContext();
    context->inputImage = ParseImageFromNAPI(env, argv[0]);

    if (!context->inputImage) {
        delete context;
        napi_throw_error(env, nullptr, "Invalid image object");
        return nullptr;
    }

    napi_get_value_int32(env, argv[1], &context->param1);
    napi_get_value_int32(env, argv[2], &context->param2);
    context->operationType = 1;

    napi_value promise;
    napi_value resourceName;
    napi_create_string_utf8(env, "ImageResize", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_promise(env, &context->deferred, &promise);

    napi_create_async_work(env, nullptr, resourceName, ExecuteImageProcess,
                          CompleteImageProcess, context, &context->work);
    napi_queue_async_work(env, context->work);

    return promise;
}

napi_value ConvertFormat(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "Image and format are required");
        return nullptr;
    }

    auto context = new ImageProcessContext();
    context->inputImage = ParseImageFromNAPI(env, argv[0]);

    if (!context->inputImage) {
        delete context;
        napi_throw_error(env, nullptr, "Invalid image object");
        return nullptr;
    }

    napi_get_value_int32(env, argv[1], &context->param1);
    context->operationType = 2;

    napi_value promise;
    napi_value resourceName;
    napi_create_string_utf8(env, "ImageConvertFormat", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_promise(env, &context->deferred, &promise);

    napi_create_async_work(env, nullptr, resourceName, ExecuteImageProcess,
                          CompleteImageProcess, context, &context->work);
    napi_queue_async_work(env, context->work);

    return promise;
}

napi_value Grayscale(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "Image is required");
        return nullptr;
    }

    auto context = new ImageProcessContext();
    context->inputImage = ParseImageFromNAPI(env, argv[0]);

    if (!context->inputImage) {
        delete context;
        napi_throw_error(env, nullptr, "Invalid image object");
        return nullptr;
    }

    context->operationType = 3;

    napi_value promise;
    napi_value resourceName;
    napi_create_string_utf8(env, "ImageGrayscale", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_promise(env, &context->deferred, &promise);

    napi_create_async_work(env, nullptr, resourceName, ExecuteImageProcess,
                          CompleteImageProcess, context, &context->work);
    napi_queue_async_work(env, context->work);

    return promise;
}

napi_value Brightness(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "Image and adjustment are required");
        return nullptr;
    }

    auto context = new ImageProcessContext();
    context->inputImage = ParseImageFromNAPI(env, argv[0]);

    if (!context->inputImage) {
        delete context;
        napi_throw_error(env, nullptr, "Invalid image object");
        return nullptr;
    }

    napi_get_value_int32(env, argv[1], &context->param1);
    context->operationType = 4;

    napi_value promise;
    napi_value resourceName;
    napi_create_string_utf8(env, "ImageBrightness", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_promise(env, &context->deferred, &promise);

    napi_create_async_work(env, nullptr, resourceName, ExecuteImageProcess,
                          CompleteImageProcess, context, &context->work);
    napi_queue_async_work(env, context->work);

    return promise;
}

napi_value Contrast(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "Image and factor are required");
        return nullptr;
    }

    auto context = new ImageProcessContext();
    context->inputImage = ParseImageFromNAPI(env, argv[0]);

    if (!context->inputImage) {
        delete context;
        napi_throw_error(env, nullptr, "Invalid image object");
        return nullptr;
    }

    napi_get_value_int32(env, argv[1], &context->param1);
    context->operationType = 5;

    napi_value promise;
    napi_value resourceName;
    napi_create_string_utf8(env, "ImageContrast", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_promise(env, &context->deferred, &promise);

    napi_create_async_work(env, nullptr, resourceName, ExecuteImageProcess,
                          CompleteImageProcess, context, &context->work);
    napi_queue_async_work(env, context->work);

    return promise;
}

napi_value Invert(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "Image is required");
        return nullptr;
    }

    auto context = new ImageProcessContext();
    context->inputImage = ParseImageFromNAPI(env, argv[0]);

    if (!context->inputImage) {
        delete context;
        napi_throw_error(env, nullptr, "Invalid image object");
        return nullptr;
    }

    context->operationType = 6;

    napi_value promise;
    napi_value resourceName;
    napi_create_string_utf8(env, "ImageInvert", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_promise(env, &context->deferred, &promise);

    napi_create_async_work(env, nullptr, resourceName, ExecuteImageProcess,
                          CompleteImageProcess, context, &context->work);
    napi_queue_async_work(env, context->work);

    return promise;
}

napi_value Create(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 3) {
        napi_throw_error(env, nullptr, "Width, height, and format are required");
        return nullptr;
    }

    int32_t width = 0;
    napi_get_value_int32(env, argv[0], &width);

    int32_t height = 0;
    napi_get_value_int32(env, argv[1], &height);

    int32_t format = 0;
    napi_get_value_int32(env, argv[2], &format);

    auto image = CreateImage(width, height, static_cast<ImageFormat>(format));
    if (!image) {
        napi_throw_error(env, nullptr, "Failed to create image");
        return nullptr;
    }

    napi_value resultObj;
    napi_create_object(env, &resultObj);

    napi_value widthValue;
    napi_create_int32(env, image->width, &widthValue);
    napi_set_named_property(env, resultObj, "width", widthValue);

    napi_value heightValue;
    napi_create_int32(env, image->height, &heightValue);
    napi_set_named_property(env, resultObj, "height", heightValue);

    napi_value formatValue;
    napi_create_int32(env, static_cast<int>(image->format), &formatValue);
    napi_set_named_property(env, resultObj, "format", formatValue);

    napi_value pixelsArray;
    void* arrayData = nullptr;
    napi_create_arraybuffer(env, image->pixels.size(), &arrayData, &pixelsArray);
    memcpy(arrayData, image->pixels.data(), image->pixels.size());
    napi_set_named_property(env, resultObj, "pixels", pixelsArray);

    return resultObj;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("create", Create),
        DECLARE_NAPI_FUNCTION("resize", Resize),
        DECLARE_NAPI_FUNCTION("convertFormat", ConvertFormat),
        DECLARE_NAPI_FUNCTION("grayscale", Grayscale),
        DECLARE_NAPI_FUNCTION("brightness", Brightness),
        DECLARE_NAPI_FUNCTION("contrast", Contrast),
        DECLARE_NAPI_FUNCTION("invert", Invert),
    };

    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

}

extern "C" __attribute__((visibility("default"))) napi_value
NAPI_Register(napi_env env, napi_value exports)
{
    return Init(env, exports);
}
