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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_COLOR_JS_COLOR_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_COLOR_JS_COLOR_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

constexpr size_t HEX_BUFFER_SIZE = 16;

enum ColorArgIndex {
    COLOR_FIRST_ARG = 0,
    COLOR_SECOND_ARG,
    COLOR_THIRD_ARG,
    COLOR_FOURTH_ARG,
    COLOR_FIFTH_ARG,
    COLOR_SIXTH_ARG,
    COLOR_SEVENTH_ARG,
};

namespace ColorArgCount {
    constexpr size_t THREE = 3;
    constexpr size_t FOUR = 4;
    constexpr size_t SIX = 6;
    constexpr size_t SEVEN = 7;
};

namespace ColorConst {
    constexpr int RGB_MAX = 255;
    constexpr double RGB_MAX_FLOAT = 255.0;
    constexpr double HUE_MAX = 360.0;
    constexpr double HUE_SEGMENT = 60.0;
    constexpr int HSV_MOD = 6;
    constexpr double HUE_NORMALIZE = 6.0;
    constexpr double HUE_OFFSET_GREEN = 2.0;
    constexpr double HUE_OFFSET_BLUE = 4.0;
    constexpr double DIVISOR_TWO = 2.0;
    constexpr double LIGHTNESS_MID = 0.5;
    constexpr double HUE_ONE_SIXTH = 1.0 / 6.0;
    constexpr double HUE_ONE_HALF = 1.0 / 2.0;
    constexpr double HUE_TWO_THIRDS = 2.0 / 3.0;
    constexpr double HUE_ONE_THIRD = 1.0 / 3.0;
    constexpr double COMPLEMENT_OFFSET = 180.0;
    constexpr double GRAYSCALE_R = 0.299;
    constexpr double GRAYSCALE_G = 0.587;
    constexpr double GRAYSCALE_B = 0.114;
    constexpr double LUMINANCE_R = 0.2126;
    constexpr double LUMINANCE_G = 0.7152;
    constexpr double LUMINANCE_B = 0.0722;
    constexpr double SRGB_THRESHOLD = 0.03928;
    constexpr double SRGB_LOW_DIVISOR = 12.92;
    constexpr double SRGB_OFFSET = 0.055;
    constexpr double SRGB_DIVISOR = 1.055;
    constexpr double SRGB_EXPONENT = 2.4;
    constexpr double CONTRAST_OFFSET = 0.05;
    constexpr int HEX_BASE = 16;
    constexpr int HEX_ALPHA_OFFSET = 10;
    constexpr size_t HEX_SHORT_LEN = 3;
    constexpr int HEX_PAIR_SIZE = 2;
    constexpr size_t HEX_FULL_LEN = 6;
    constexpr int HEX_GREEN_START = 2;
    constexpr int HEX_BLUE_START = 4;
    constexpr int SHIFT_ALPHA = 24;
    constexpr int SHIFT_RED = 16;
    constexpr int SHIFT_GREEN = 8;
    constexpr int HSV_CASE_GREEN = 2;
    constexpr int HSV_CASE_CYAN = 3;
    constexpr int HSV_CASE_BLUE = 4;
    constexpr double NEG_ONE = -1.0;
};

struct RGBColor {
    int r;
    int g;
    int b;
    int a;

    RGBColor() : r(0), g(0), b(0), a(ColorConst::RGB_MAX) {}
    RGBColor(int red, int green, int blue) : r(red), g(green), b(blue), a(ColorConst::RGB_MAX) {}
    RGBColor(int red, int green, int blue, int alpha) : r(red), g(green), b(blue), a(alpha) {}
};

struct HSLColor {
    double h;
    double s;
    double l;

    HSLColor() : h(0.0), s(0.0), l(0.0) {}
    HSLColor(double hue, double saturation, double lightness) : h(hue), s(saturation), l(lightness) {}
};

struct HSVColor {
    double h;
    double s;
    double v;

    HSVColor() : h(0.0), s(0.0), v(0.0) {}
    HSVColor(double hue, double saturation, double value) : h(hue), s(saturation), v(value) {}
};

struct CMYKColor {
    double c;
    double m;
    double y;
    double k;

    CMYKColor() : c(0.0), m(0.0), y(0.0), k(0.0) {}
    CMYKColor(double cyan, double magenta, double yellow, double key)
        : c(cyan), m(magenta), y(yellow), k(key) {}
};

class ColorHelper {
public:
    static HSLColor RGBToHSL(const RGBColor& rgb);
    static RGBColor HSLToRGB(const HSLColor& hsl);
    static HSVColor RGBToHSV(const RGBColor& rgb);
    static RGBColor HSVToRGB(const HSVColor& hsv);
    static CMYKColor RGBToCMYK(const RGBColor& rgb);
    static RGBColor CMYKToRGB(const CMYKColor& cmyk);
    static std::string RGBToHex(const RGBColor& rgb);
    static RGBColor HexToRGB(const std::string& hex);
    static uint32_t RGBToInt(const RGBColor& rgb);
    static RGBColor IntToRGB(uint32_t color);
    static RGBColor Blend(const RGBColor& c1, const RGBColor& c2, double factor);
    static RGBColor Lighten(const RGBColor& rgb, double amount);
    static RGBColor Darken(const RGBColor& rgb, double amount);
    static RGBColor Saturate(const RGBColor& rgb, double amount);
    static RGBColor Desaturate(const RGBColor& rgb, double amount);
    static RGBColor Grayscale(const RGBColor& rgb);
    static RGBColor Invert(const RGBColor& rgb);
    static RGBColor Complement(const RGBColor& rgb);
    static double Luminance(const RGBColor& rgb);
    static double ContrastRatio(const RGBColor& c1, const RGBColor& c2);
    static int ClampChannel(int value);
    static double ClampNorm(double value);
};

#endif
