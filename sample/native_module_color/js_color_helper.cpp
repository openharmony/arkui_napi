/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_color_helper.h"

#include "securec.h"

/***********************************************
 * ColorHelper Implementation
 ***********************************************/
int ColorHelper::ClampChannel(int value)
{
    if (value < 0) {
        return 0;
    }
    if (value > ColorConst::RGB_MAX) {
        return ColorConst::RGB_MAX;
    }
    return value;
}

double ColorHelper::ClampNorm(double value)
{
    if (value < 0.0) {
        return 0.0;
    }
    if (value > 1.0) {
        return 1.0;
    }
    return value;
}

static double HueToRGB(double p, double q, double t)
{
    if (t < 0.0) {
        t += 1.0;
    }
    if (t > 1.0) {
        t -= 1.0;
    }
    if (t < ColorConst::HUE_ONE_SIXTH) {
        return p + (q - p) * ColorConst::HUE_NORMALIZE * t;
    }
    if (t < ColorConst::HUE_ONE_HALF) {
        return q;
    }
    if (t < ColorConst::HUE_TWO_THIRDS) {
        return p + (q - p) * (ColorConst::HUE_TWO_THIRDS - t) *
            ColorConst::HUE_NORMALIZE;
    }
    return p;
}

HSLColor ColorHelper::RGBToHSL(const RGBColor& rgb)
{
    double r = rgb.r / ColorConst::RGB_MAX_FLOAT;
    double g = rgb.g / ColorConst::RGB_MAX_FLOAT;
    double b = rgb.b / ColorConst::RGB_MAX_FLOAT;
    double maxVal = std::max({r, g, b});
    double minVal = std::min({r, g, b});
    double h = 0.0;
    double s = 0.0;
    double l = (maxVal + minVal) / ColorConst::DIVISOR_TWO;
    if (std::abs(maxVal - minVal) > 1e-10) {
        double d = maxVal - minVal;
        s = l > ColorConst::LIGHTNESS_MID ? d / (ColorConst::DIVISOR_TWO - maxVal - minVal) : d / (maxVal + minVal);
        if (std::abs(maxVal - r) < 1e-10) {
            h = (g - b) / d + (g < b ? ColorConst::HUE_NORMALIZE : 0.0);
        } else if (std::abs(maxVal - g) < 1e-10) {
            h = (b - r) / d + ColorConst::HUE_OFFSET_GREEN;
        } else {
            h = (r - g) / d + ColorConst::HUE_OFFSET_BLUE;
        }
        h /= ColorConst::HUE_NORMALIZE;
    }
    return HSLColor(h * ColorConst::HUE_MAX, s, l);
}

RGBColor ColorHelper::HSLToRGB(const HSLColor& hsl)
{
    double h = hsl.h / ColorConst::HUE_MAX;
    double s = hsl.s;
    double l = hsl.l;
    if (std::abs(s) < 1e-10) {
        int val = static_cast<int>(l * ColorConst::RGB_MAX_FLOAT);
        return RGBColor(val, val, val);
    }
    double q = l < ColorConst::LIGHTNESS_MID ? l * (1.0 + s) : l + s - l * s;
    double p = ColorConst::DIVISOR_TWO * l - q;
    int r = static_cast<int>(HueToRGB(p, q, h + ColorConst::HUE_ONE_THIRD) * ColorConst::RGB_MAX_FLOAT);
    int g = static_cast<int>(HueToRGB(p, q, h) * ColorConst::RGB_MAX_FLOAT);
    int b = static_cast<int>(HueToRGB(p, q, h - ColorConst::HUE_ONE_THIRD) * ColorConst::RGB_MAX_FLOAT);
    return RGBColor(ClampChannel(r), ClampChannel(g), ClampChannel(b));
}

HSVColor ColorHelper::RGBToHSV(const RGBColor& rgb)
{
    double r = rgb.r / ColorConst::RGB_MAX_FLOAT;
    double g = rgb.g / ColorConst::RGB_MAX_FLOAT;
    double b = rgb.b / ColorConst::RGB_MAX_FLOAT;
    double maxVal = std::max({r, g, b});
    double minVal = std::min({r, g, b});
    double h = 0.0;
    double v = maxVal;
    double d = maxVal - minVal;
    double s = (maxVal != 0.0) ? d * std::pow(maxVal, ColorConst::NEG_ONE) : 0.0;
    if (d > 0.0 || d < 0.0) {
        if (std::abs(maxVal - r) < 1e-10) {
            h = (g - b) / d + (g < b ? ColorConst::HUE_NORMALIZE : 0.0);
        } else if (std::abs(maxVal - g) < 1e-10) {
            h = (b - r) / d + ColorConst::HUE_OFFSET_GREEN;
        } else {
            h = (r - g) / d + ColorConst::HUE_OFFSET_BLUE;
        }
        h /= ColorConst::HUE_NORMALIZE;
    }
    return HSVColor(h * ColorConst::HUE_MAX, s, v);
}

RGBColor ColorHelper::HSVToRGB(const HSVColor& hsv)
{
    double h = hsv.h / ColorConst::HUE_SEGMENT;
    double s = hsv.s;
    double v = hsv.v;
    int i = static_cast<int>(h) % ColorConst::HSV_MOD;
    double f = h - static_cast<int>(h);
    double p = v * (1.0 - s);
    double q = v * (1.0 - f * s);
    double t = v * (1.0 - (1.0 - f) * s);
    double r;
    double g;
    double b;
    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case ColorConst::HSV_CASE_GREEN: r = p; g = v; b = t; break;
        case ColorConst::HSV_CASE_CYAN: r = p; g = q; b = v; break;
        case ColorConst::HSV_CASE_BLUE: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    return RGBColor(ClampChannel(static_cast<int>(r * ColorConst::RGB_MAX_FLOAT)),
                    ClampChannel(static_cast<int>(g * ColorConst::RGB_MAX_FLOAT)),
                    ClampChannel(static_cast<int>(b * ColorConst::RGB_MAX_FLOAT)));
}

CMYKColor ColorHelper::RGBToCMYK(const RGBColor& rgb)
{
    double r = rgb.r / ColorConst::RGB_MAX_FLOAT;
    double g = rgb.g / ColorConst::RGB_MAX_FLOAT;
    double b = rgb.b / ColorConst::RGB_MAX_FLOAT;
    double k = 1.0 - std::max({r, g, b});
    if (std::abs(k - 1.0) < 1e-10) {
        return CMYKColor(0.0, 0.0, 0.0, 1.0);
    }
    double c = (1.0 - r - k) / (1.0 - k);
    double m = (1.0 - g - k) / (1.0 - k);
    double y = (1.0 - b - k) / (1.0 - k);
    return CMYKColor(c, m, y, k);
}

RGBColor ColorHelper::CMYKToRGB(const CMYKColor& cmyk)
{
    int r = static_cast<int>((1.0 - cmyk.c) * (1.0 - cmyk.k) * ColorConst::RGB_MAX_FLOAT);
    int g = static_cast<int>((1.0 - cmyk.m) * (1.0 - cmyk.k) * ColorConst::RGB_MAX_FLOAT);
    int b = static_cast<int>((1.0 - cmyk.y) * (1.0 - cmyk.k) * ColorConst::RGB_MAX_FLOAT);
    return RGBColor(ClampChannel(r), ClampChannel(g), ClampChannel(b));
}

std::string ColorHelper::RGBToHex(const RGBColor& rgb)
{
    char buf[HEX_BUFFER_SIZE] = { 0 };
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "#%02X%02X%02X", rgb.r, rgb.g, rgb.b) < 0) {
        return "#000000";
    }
    return std::string(buf);
}

RGBColor ColorHelper::HexToRGB(const std::string& hex)
{
    std::string h = hex;
    if (!h.empty() && h[0] == '#') {
        h = h.substr(1);
    }
    if (h.length() == ColorConst::HEX_SHORT_LEN) {
        h = std::string(ColorConst::HEX_PAIR_SIZE, h[0]) + std::string(ColorConst::HEX_PAIR_SIZE, h[1]) +
            std::string(ColorConst::HEX_PAIR_SIZE, h[ColorConst::HEX_PAIR_SIZE]);
    }
    if (h.length() < ColorConst::HEX_FULL_LEN) {
        return RGBColor(0, 0, 0);
    }
    unsigned int r = 0;
    unsigned int g = 0;
    unsigned int b = 0;
    for (int i = 0; i < ColorConst::HEX_PAIR_SIZE; i++) {
        char c = h[i];
        r = r * ColorConst::HEX_BASE + (c >= '0' && c <= '9' ? c - '0' :
            (c >= 'a' && c <= 'f' ? c - 'a' + ColorConst::HEX_ALPHA_OFFSET :
            (c >= 'A' && c <= 'F' ? c - 'A' + ColorConst::HEX_ALPHA_OFFSET : 0)));
    }
    for (int i = ColorConst::HEX_GREEN_START; i < ColorConst::HEX_BLUE_START; i++) {
        char c = h[i];
        g = g * ColorConst::HEX_BASE + (c >= '0' && c <= '9' ? c - '0' :
            (c >= 'a' && c <= 'f' ? c - 'a' + ColorConst::HEX_ALPHA_OFFSET :
            (c >= 'A' && c <= 'F' ? c - 'A' + ColorConst::HEX_ALPHA_OFFSET : 0)));
    }
    for (int i = ColorConst::HEX_BLUE_START; i < static_cast<int>(ColorConst::HEX_FULL_LEN); i++) {
        char c = h[i];
        b = b * ColorConst::HEX_BASE + (c >= '0' && c <= '9' ? c - '0' :
            (c >= 'a' && c <= 'f' ? c - 'a' + ColorConst::HEX_ALPHA_OFFSET :
            (c >= 'A' && c <= 'F' ? c - 'A' + ColorConst::HEX_ALPHA_OFFSET : 0)));
    }
    return RGBColor(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b));
}

uint32_t ColorHelper::RGBToInt(const RGBColor& rgb)
{
    return (static_cast<uint32_t>(rgb.a) << ColorConst::SHIFT_ALPHA) |
           (static_cast<uint32_t>(rgb.r) << ColorConst::SHIFT_RED) |
           (static_cast<uint32_t>(rgb.g) << ColorConst::SHIFT_GREEN) |
           static_cast<uint32_t>(rgb.b);
}

RGBColor ColorHelper::IntToRGB(uint32_t color)
{
    int a = (color >> ColorConst::SHIFT_ALPHA) & 0xFF;
    int r = (color >> ColorConst::SHIFT_RED) & 0xFF;
    int g = (color >> ColorConst::SHIFT_GREEN) & 0xFF;
    int b = color & 0xFF;
    return RGBColor(r, g, b, a);
}

RGBColor ColorHelper::Blend(const RGBColor& c1, const RGBColor& c2, double factor)
{
    factor = ClampNorm(factor);
    int r = static_cast<int>(c1.r + (c2.r - c1.r) * factor);
    int g = static_cast<int>(c1.g + (c2.g - c1.g) * factor);
    int b = static_cast<int>(c1.b + (c2.b - c1.b) * factor);
    return RGBColor(ClampChannel(r), ClampChannel(g), ClampChannel(b));
}

RGBColor ColorHelper::Lighten(const RGBColor& rgb, double amount)
{
    HSLColor hsl = RGBToHSL(rgb);
    hsl.l = ClampNorm(hsl.l + amount);
    return HSLToRGB(hsl);
}

RGBColor ColorHelper::Darken(const RGBColor& rgb, double amount)
{
    HSLColor hsl = RGBToHSL(rgb);
    hsl.l = ClampNorm(hsl.l - amount);
    return HSLToRGB(hsl);
}

RGBColor ColorHelper::Saturate(const RGBColor& rgb, double amount)
{
    HSLColor hsl = RGBToHSL(rgb);
    hsl.s = ClampNorm(hsl.s + amount);
    return HSLToRGB(hsl);
}

RGBColor ColorHelper::Desaturate(const RGBColor& rgb, double amount)
{
    HSLColor hsl = RGBToHSL(rgb);
    hsl.s = ClampNorm(hsl.s - amount);
    return HSLToRGB(hsl);
}

RGBColor ColorHelper::Grayscale(const RGBColor& rgb)
{
    int gray = static_cast<int>(ColorConst::GRAYSCALE_R * rgb.r + ColorConst::GRAYSCALE_G * rgb.g +
        ColorConst::GRAYSCALE_B * rgb.b);
    return RGBColor(gray, gray, gray);
}

RGBColor ColorHelper::Invert(const RGBColor& rgb)
{
    return RGBColor(ColorConst::RGB_MAX - rgb.r, ColorConst::RGB_MAX - rgb.g, ColorConst::RGB_MAX - rgb.b);
}

RGBColor ColorHelper::Complement(const RGBColor& rgb)
{
    HSLColor hsl = RGBToHSL(rgb);
    hsl.h = std::fmod(hsl.h + ColorConst::COMPLEMENT_OFFSET, ColorConst::HUE_MAX);
    return HSLToRGB(hsl);
}

double ColorHelper::Luminance(const RGBColor& rgb)
{
    auto srgbToLinear = [](double c) -> double {
        c /= ColorConst::RGB_MAX_FLOAT;
        return c <= ColorConst::SRGB_THRESHOLD ? c / ColorConst::SRGB_LOW_DIVISOR :
            std::pow((c + ColorConst::SRGB_OFFSET) / ColorConst::SRGB_DIVISOR, ColorConst::SRGB_EXPONENT);
    };
    return ColorConst::LUMINANCE_R * srgbToLinear(rgb.r) + ColorConst::LUMINANCE_G * srgbToLinear(rgb.g) +
        ColorConst::LUMINANCE_B * srgbToLinear(rgb.b);
}

double ColorHelper::ContrastRatio(const RGBColor& c1, const RGBColor& c2)
{
    double l1 = Luminance(c1);
    double l2 = Luminance(c2);
    double lighter = std::max(l1, l2);
    double darker = std::min(l1, l2);
    return (lighter + ColorConst::CONTRAST_OFFSET) / (darker + ColorConst::CONTRAST_OFFSET);
}

/***********************************************
 * NAPI Helper
 ***********************************************/
static napi_value CreateRGBObject(napi_env env, const RGBColor& rgb)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_value rVal;
    napi_value gVal;
    napi_value bVal;
    napi_value aVal;
    napi_create_int32(env, rgb.r, &rVal);
    napi_create_int32(env, rgb.g, &gVal);
    napi_create_int32(env, rgb.b, &bVal);
    napi_create_int32(env, rgb.a, &aVal);
    napi_set_named_property(env, result, "r", rVal);
    napi_set_named_property(env, result, "g", gVal);
    napi_set_named_property(env, result, "b", bVal);
    napi_set_named_property(env, result, "a", aVal);
    return result;
}

static napi_value CreateHSLObject(napi_env env, const HSLColor& hsl)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_value hVal;
    napi_value sVal;
    napi_value lVal;
    napi_create_double(env, hsl.h, &hVal);
    napi_create_double(env, hsl.s, &sVal);
    napi_create_double(env, hsl.l, &lVal);
    napi_set_named_property(env, result, "h", hVal);
    napi_set_named_property(env, result, "s", sVal);
    napi_set_named_property(env, result, "l", lVal);
    return result;
}

static napi_value CreateHSVObject(napi_env env, const HSVColor& hsv)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_value hVal;
    napi_value sVal;
    napi_value vVal;
    napi_create_double(env, hsv.h, &hVal);
    napi_create_double(env, hsv.s, &sVal);
    napi_create_double(env, hsv.v, &vVal);
    napi_set_named_property(env, result, "h", hVal);
    napi_set_named_property(env, result, "s", sVal);
    napi_set_named_property(env, result, "v", vVal);
    return result;
}

static napi_value CreateCMYKObject(napi_env env, const CMYKColor& cmyk)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_value cVal;
    napi_value mVal;
    napi_value yVal;
    napi_value kVal;
    napi_create_double(env, cmyk.c, &cVal);
    napi_create_double(env, cmyk.m, &mVal);
    napi_create_double(env, cmyk.y, &yVal);
    napi_create_double(env, cmyk.k, &kVal);
    napi_set_named_property(env, result, "c", cVal);
    napi_set_named_property(env, result, "m", mVal);
    napi_set_named_property(env, result, "y", yVal);
    napi_set_named_property(env, result, "k", kVal);
    return result;
}

/***********************************************
 * NAPI Functions
 ***********************************************/
static napi_value JSRGBToHSL(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: r, g, b");
    int32_t r;
    int32_t g;
    int32_t b;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    HSLColor hsl = ColorHelper::RGBToHSL(RGBColor(r, g, b));
    return CreateHSLObject(env, hsl);
}

static napi_value JSHSLToRGB(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: h, s, l");
    double h;
    double s;
    double l;
    napi_get_value_double(env, argv[0], &h);
    napi_get_value_double(env, argv[1], &s);
    napi_get_value_double(env, argv[COLOR_THIRD_ARG], &l);
    RGBColor rgb = ColorHelper::HSLToRGB(HSLColor(h, s, l));
    return CreateRGBObject(env, rgb);
}

static napi_value JSRGBToHSV(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: r, g, b");
    int32_t r;
    int32_t g;
    int32_t b;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    HSVColor hsv = ColorHelper::RGBToHSV(RGBColor(r, g, b));
    return CreateHSVObject(env, hsv);
}

static napi_value JSHSVToRGB(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: h, s, v");
    double h;
    double s;
    double v;
    napi_get_value_double(env, argv[0], &h);
    napi_get_value_double(env, argv[1], &s);
    napi_get_value_double(env, argv[COLOR_THIRD_ARG], &v);
    RGBColor rgb = ColorHelper::HSVToRGB(HSVColor(h, s, v));
    return CreateRGBObject(env, rgb);
}

static napi_value JSRGBToCMYK(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: r, g, b");
    int32_t r;
    int32_t g;
    int32_t b;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    CMYKColor cmyk = ColorHelper::RGBToCMYK(RGBColor(r, g, b));
    return CreateCMYKObject(env, cmyk);
}

static napi_value JSCMYKToRGB(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::FOUR;
    napi_value argv[ColorArgCount::FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::FOUR, "requires 4 parameters: c, m, y, k");
    double c;
    double m;
    double y;
    double k;
    napi_get_value_double(env, argv[COLOR_FIRST_ARG], &c);
    napi_get_value_double(env, argv[COLOR_SECOND_ARG], &m);
    napi_get_value_double(env, argv[COLOR_THIRD_ARG], &y);
    napi_get_value_double(env, argv[COLOR_FOURTH_ARG], &k);
    RGBColor rgb = ColorHelper::CMYKToRGB(CMYKColor(c, m, y, k));
    return CreateRGBObject(env, rgb);
}

static napi_value JSRGBToHex(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: r, g, b");
    int32_t r;
    int32_t g;
    int32_t b;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    std::string hex = ColorHelper::RGBToHex(RGBColor(r, g, b));
    napi_value result = nullptr;
    napi_create_string_utf8(env, hex.c_str(), hex.length(), &result);
    return result;
}

static napi_value JSHexToRGB(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char hex[HEX_BUFFER_SIZE] = { 0 };
    size_t hexLen = 0;
    napi_get_value_string_utf8(env, argv[0], hex, HEX_BUFFER_SIZE, &hexLen);
    RGBColor rgb = ColorHelper::HexToRGB(std::string(hex, hexLen));
    return CreateRGBObject(env, rgb);
}

static napi_value JSBlend(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::SEVEN;
    napi_value argv[ColorArgCount::SEVEN] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::SEVEN, "requires 7 parameters: r1,g1,b1, r2,g2,b2, factor");
    int32_t r1;
    int32_t g1;
    int32_t b1;
    int32_t r2;
    int32_t g2;
    int32_t b2;
    double factor;
    napi_get_value_int32(env, argv[COLOR_FIRST_ARG], &r1);
    napi_get_value_int32(env, argv[COLOR_SECOND_ARG], &g1);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b1);
    napi_get_value_int32(env, argv[COLOR_FOURTH_ARG], &r2);
    napi_get_value_int32(env, argv[COLOR_FIFTH_ARG], &g2);
    napi_get_value_int32(env, argv[COLOR_SIXTH_ARG], &b2);
    napi_get_value_double(env, argv[COLOR_SEVENTH_ARG], &factor);
    RGBColor result = ColorHelper::Blend(RGBColor(r1, g1, b1), RGBColor(r2, g2, b2), factor);
    return CreateRGBObject(env, result);
}

static napi_value JSLighten(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::FOUR;
    napi_value argv[ColorArgCount::FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::FOUR, "requires 4 parameters: r, g, b, amount");
    int32_t r;
    int32_t g;
    int32_t b;
    double amount;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    napi_get_value_double(env, argv[COLOR_FOURTH_ARG], &amount);
    return CreateRGBObject(env, ColorHelper::Lighten(RGBColor(r, g, b), amount));
}

static napi_value JSDarken(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::FOUR;
    napi_value argv[ColorArgCount::FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::FOUR, "requires 4 parameters: r, g, b, amount");
    int32_t r;
    int32_t g;
    int32_t b;
    double amount;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    napi_get_value_double(env, argv[COLOR_FOURTH_ARG], &amount);
    return CreateRGBObject(env, ColorHelper::Darken(RGBColor(r, g, b), amount));
}

static napi_value JSGrayscale(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: r, g, b");
    int32_t r;
    int32_t g;
    int32_t b;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    return CreateRGBObject(env, ColorHelper::Grayscale(RGBColor(r, g, b)));
}

static napi_value JSInvert(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: r, g, b");
    int32_t r;
    int32_t g;
    int32_t b;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    return CreateRGBObject(env, ColorHelper::Invert(RGBColor(r, g, b)));
}

static napi_value JSComplement(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: r, g, b");
    int32_t r;
    int32_t g;
    int32_t b;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    return CreateRGBObject(env, ColorHelper::Complement(RGBColor(r, g, b)));
}

static napi_value JSLuminance(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: r, g, b");
    int32_t r;
    int32_t g;
    int32_t b;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    napi_value result = nullptr;
    napi_create_double(env, ColorHelper::Luminance(RGBColor(r, g, b)), &result);
    return result;
}

static napi_value JSSaturate(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::FOUR;
    napi_value argv[ColorArgCount::FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::FOUR, "requires 4 parameters: r, g, b, amount");
    int32_t r;
    int32_t g;
    int32_t b;
    double amount;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    napi_get_value_double(env, argv[COLOR_FOURTH_ARG], &amount);
    return CreateRGBObject(env, ColorHelper::Saturate(RGBColor(r, g, b), amount));
}

static napi_value JSDesaturate(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::FOUR;
    napi_value argv[ColorArgCount::FOUR] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::FOUR, "requires 4 parameters: r, g, b, amount");
    int32_t r;
    int32_t g;
    int32_t b;
    double amount;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    napi_get_value_double(env, argv[COLOR_FOURTH_ARG], &amount);
    return CreateRGBObject(env, ColorHelper::Desaturate(RGBColor(r, g, b), amount));
}

static napi_value JSRGBToInt(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::THREE;
    napi_value argv[ColorArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::THREE, "requires 3 parameters: r, g, b");
    int32_t r;
    int32_t g;
    int32_t b;
    napi_get_value_int32(env, argv[0], &r);
    napi_get_value_int32(env, argv[1], &g);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b);
    napi_value result = nullptr;
    napi_create_uint32(env, ColorHelper::RGBToInt(RGBColor(r, g, b)), &result);
    return result;
}

static napi_value JSContrastRatio(napi_env env, napi_callback_info info)
{
    size_t argc = ColorArgCount::SIX;
    napi_value argv[ColorArgCount::SIX] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ColorArgCount::SIX, "requires 6 parameters: r1,g1,b1, r2,g2,b2");
    int32_t r1;
    int32_t g1;
    int32_t b1;
    int32_t r2;
    int32_t g2;
    int32_t b2;
    napi_get_value_int32(env, argv[0], &r1);
    napi_get_value_int32(env, argv[1], &g1);
    napi_get_value_int32(env, argv[COLOR_THIRD_ARG], &b1);
    napi_get_value_int32(env, argv[COLOR_FOURTH_ARG], &r2);
    napi_get_value_int32(env, argv[COLOR_FIFTH_ARG], &g2);
    napi_get_value_int32(env, argv[COLOR_SIXTH_ARG], &b2);
    napi_value result = nullptr;
    napi_create_double(env, ColorHelper::ContrastRatio(RGBColor(r1, g1, b1), RGBColor(r2, g2, b2)), &result);
    return result;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value ColorExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("rgbToHsl", JSRGBToHSL),
        DECLARE_NAPI_FUNCTION("hslToRgb", JSHSLToRGB),
        DECLARE_NAPI_FUNCTION("rgbToHsv", JSRGBToHSV),
        DECLARE_NAPI_FUNCTION("hsvToRgb", JSHSVToRGB),
        DECLARE_NAPI_FUNCTION("rgbToCmyk", JSRGBToCMYK),
        DECLARE_NAPI_FUNCTION("cmykToRgb", JSCMYKToRGB),
        DECLARE_NAPI_FUNCTION("rgbToHex", JSRGBToHex),
        DECLARE_NAPI_FUNCTION("hexToRgb", JSHexToRGB),
        DECLARE_NAPI_FUNCTION("blend", JSBlend),
        DECLARE_NAPI_FUNCTION("lighten", JSLighten),
        DECLARE_NAPI_FUNCTION("darken", JSDarken),
        DECLARE_NAPI_FUNCTION("grayscale", JSGrayscale),
        DECLARE_NAPI_FUNCTION("invert", JSInvert),
        DECLARE_NAPI_FUNCTION("complement", JSComplement),
        DECLARE_NAPI_FUNCTION("luminance", JSLuminance),
        DECLARE_NAPI_FUNCTION("saturate", JSSaturate),
        DECLARE_NAPI_FUNCTION("desaturate", JSDesaturate),
        DECLARE_NAPI_FUNCTION("rgbToInt", JSRGBToInt),
        DECLARE_NAPI_FUNCTION("contrastRatio", JSContrastRatio),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_colorModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ColorExport,
    .nm_modname = "color",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void ColorRegister()
{
    napi_module_register(&g_colorModule);
}
