# Image Processor NAPI Module

## Overview

The Image Processor module provides comprehensive image processing capabilities for NAPI applications. This module demonstrates real-world business scenarios including image manipulation, format conversion, and async image processing operations.

## Features

- **Image Creation**: Create new images with specified dimensions and formats
- **Image Resizing**: Scale images to different dimensions
- **Format Conversion**: Convert between RGB, RGBA, and Grayscale formats
- **Image Filters**: Apply various image processing filters
  - Grayscale conversion
  - Brightness adjustment
  - Contrast adjustment
  - Color inversion
- **Async Processing**: All operations are non-blocking and Promise-based
- **Memory Management**: Efficient memory handling for large images
- **Thread Safety**: Safe for concurrent image processing operations

## Installation

Add the module to your BUILD.gn file:

```gn
ohos_shared_library("your_module") {
  deps = [
    "//foundation/arkui/napi/sample/native_module_image_processor:image_processor"
  ]
}
```

## API Reference

### Image Format Constants

```javascript
const ImageFormat = {
  RGB: 0,        // 3 bytes per pixel (Red, Green, Blue)
  RGBA: 1,       // 4 bytes per pixel (Red, Green, Blue, Alpha)
  GRAYSCALE: 2   // 1 byte per pixel (Grayscale)
};
```

### `create(width, height, format)`

Creates a new image with specified dimensions and format.

**Parameters:**
- `width` (number): Image width in pixels (1-8192)
- `height` (number): Image height in pixels (1-8192)
- `format` (number): Image format (RGB, RGBA, or GRAYSCALE)

**Returns:**
- `object`: Image object with properties:
  - `width` (number): Image width
  - `height` (number): Image height
  - `format` (number): Image format
  - `pixels` (ArrayBuffer): Raw pixel data

**Example:**
```javascript
import imageProcessor from 'libimage_processor.so';

// Create a 100x100 RGB image
const image = imageProcessor.create(100, 100, ImageFormat.RGB);
console.log('Image created:', image.width, 'x', image.height);
```

### `resize(image, newWidth, newHeight)`

Resizes an image to new dimensions using nearest-neighbor interpolation.

**Parameters:**
- `image` (object): Source image object
- `newWidth` (number): New width in pixels
- `newHeight` (number): New height in pixels

**Returns:**
- `Promise<object>`: Promise resolving to resized image object

**Example:**
```javascript
const resizedImage = await imageProcessor.resize(image, 200, 200);
console.log('Resized to:', resizedImage.width, 'x', resizedImage.height);
```

### `convertFormat(image, targetFormat)`

Converts an image to a different format.

**Parameters:**
- `image` (object): Source image object
- `targetFormat` (number): Target format (RGB, RGBA, or GRAYSCALE)

**Returns:**
- `Promise<object>`: Promise resolving to converted image object

**Example:**
```javascript
// Convert RGB to RGBA
const rgbaImage = await imageProcessor.convertFormat(image, ImageFormat.RGBA);

// Convert to grayscale
const grayImage = await imageProcessor.convertFormat(image, ImageFormat.GRAYSCALE);
```

### `grayscale(image)`

Converts an image to grayscale.

**Parameters:**
- `image` (object): Source image object

**Returns:**
- `Promise<object>`: Promise resolving to grayscale image object

**Example:**
```javascript
const grayImage = await imageProcessor.grayscale(image);
console.log('Converted to grayscale');
```

### `brightness(image, adjustment)`

Adjusts the brightness of an image.

**Parameters:**
- `image` (object): Source image object
- `adjustment` (number): Brightness adjustment (-255 to 255)
  - Positive values increase brightness
  - Negative values decrease brightness

**Returns:**
- `Promise<object>`: Promise resolving to adjusted image object

**Example:**
```javascript
// Increase brightness by 50
const brightImage = await imageProcessor.brightness(image, 50);

// Decrease brightness by 30
const darkImage = await imageProcessor.brightness(image, -30);
```

### `contrast(image, factor)`

Adjusts the contrast of an image.

**Parameters:**
- `image` (object): Source image object
- `factor` (number): Contrast factor (0-200, where 100 is no change)
  - Values > 100 increase contrast
  - Values < 100 decrease contrast

**Returns:**
- `Promise<object>`: Promise resolving to adjusted image object

**Example:**
```javascript
// Increase contrast
const highContrastImage = await imageProcessor.contrast(image, 150);

// Decrease contrast
const lowContrastImage = await imageProcessor.contrast(image, 50);
```

### `invert(image)`

Inverts the colors of an image.

**Parameters:**
- `image` (object): Source image object

**Returns:**
- `Promise<object>`: Promise resolving to inverted image object

**Example:**
```javascript
const invertedImage = await imageProcessor.invert(image);
console.log('Colors inverted');
```

## Business Use Cases

### 1. Image Thumbnail Generation

```javascript
class ThumbnailGenerator {
  constructor() {
    this.thumbnailSizes = [
      { name: 'small', width: 64, height: 64 },
      { name: 'medium', width: 128, height: 128 },
      { name: 'large', width: 256, height: 256 }
    ];
  }

  async generateThumbnails(originalImage) {
    const thumbnails = {};

    for (const size of this.thumbnailSizes) {
      thumbnails[size.name] = await imageProcessor.resize(
        originalImage,
        size.width,
        size.height
      );
    }

    return thumbnails;
  }
}

const generator = new ThumbnailGenerator();
const thumbnails = await generator.generateThumbnails(originalImage);
```

### 2. Image Preprocessing for Machine Learning

```javascript
class ImagePreprocessor {
  async preprocessForML(image) {
    // Convert to grayscale
    let processed = await imageProcessor.grayscale(image);

    // Resize to standard ML input size
    processed = await imageProcessor.resize(processed, 224, 224);

    // Normalize brightness
    processed = await imageProcessor.brightness(processed, 0);

    // Enhance contrast
    processed = await imageProcessor.contrast(processed, 120);

    return processed;
  }
}

const preprocessor = new ImagePreprocessor();
const mlInput = await preprocessor.preprocessForML(rawImage);
```

### 3. Photo Editing Application

```javascript
class PhotoEditor {
  constructor(originalImage) {
    this.originalImage = originalImage;
    this.currentImage = originalImage;
    this.history = [];
  }

  async applyBrightness(adjustment) {
    this.saveToHistory();
    this.currentImage = await imageProcessor.brightness(
      this.currentImage,
      adjustment
    );
    return this.currentImage;
  }

  async applyContrast(factor) {
    this.saveToHistory();
    this.currentImage = await imageProcessor.contrast(
      this.currentImage,
      factor
    );
    return this.currentImage;
  }

  async applyGrayscale() {
    this.saveToHistory();
    this.currentImage = await imageProcessor.grayscale(this.currentImage);
    return this.currentImage;
  }

  async applyInvert() {
    this.saveToHistory();
    this.currentImage = await imageProcessor.invert(this.currentImage);
    return this.currentImage;
  }

  saveToHistory() {
    this.history.push(this.currentImage);
  }

  undo() {
    if (this.history.length > 0) {
      this.currentImage = this.history.pop();
    }
    return this.currentImage;
  }

  reset() {
    this.currentImage = this.originalImage;
    this.history = [];
    return this.currentImage;
  }
}

const editor = new PhotoEditor(originalImage);
await editor.applyBrightness(20);
await editor.applyContrast(110);
const editedImage = editor.currentImage;
```

### 4. Batch Image Processing

```javascript
class BatchImageProcessor {
  async processBatch(images, operations) {
    const results = [];

    for (const image of images) {
      let processedImage = image;

      for (const operation of operations) {
        switch (operation.type) {
          case 'resize':
            processedImage = await imageProcessor.resize(
              processedImage,
              operation.width,
              operation.height
            );
            break;
          case 'grayscale':
            processedImage = await imageProcessor.grayscale(processedImage);
            break;
          case 'brightness':
            processedImage = await imageProcessor.brightness(
              processedImage,
              operation.value
            );
            break;
          case 'contrast':
            processedImage = await imageProcessor.contrast(
              processedImage,
              operation.value
            );
            break;
        }
      }

      results.push(processedImage);
    }

    return results;
  }
}

const batchProcessor = new BatchImageProcessor();
const processedImages = await batchProcessor.processBatch(
  [image1, image2, image3],
  [
    { type: 'resize', width: 800, height: 600 },
    { type: 'brightness', value: 10 },
    { type: 'contrast', value: 105 }
  ]
);
```

### 5. Image Format Conversion Service

```javascript
class ImageFormatConverter {
  async convertToRGBA(image) {
    if (image.format === ImageFormat.RGBA) {
      return image;
    }
    return await imageProcessor.convertFormat(image, ImageFormat.RGBA);
  }

  async convertToRGB(image) {
    if (image.format === ImageFormat.RGB) {
      return image;
    }
    return await imageProcessor.convertFormat(image, ImageFormat.RGB);
  }

  async convertToGrayscale(image) {
    if (image.format === ImageFormat.GRAYSCALE) {
      return image;
    }
    return await imageProcessor.convertFormat(image, ImageFormat.GRAYSCALE);
  }

  async autoConvert(image, targetFormat) {
    return await imageProcessor.convertFormat(image, targetFormat);
  }
}

const converter = new ImageFormatConverter();
const rgbaImage = await converter.convertToRGBA(rgbImage);
```

## Working with Pixel Data

### Reading Pixel Data

```javascript
function getPixel(image, x, y) {
  const bytesPerPixel = getBytesPerPixel(image.format);
  const index = (y * image.width + x) * bytesPerPixel;
  const pixels = new Uint8Array(image.pixels);

  if (image.format === ImageFormat.RGB) {
    return {
      r: pixels[index],
      g: pixels[index + 1],
      b: pixels[index + 2]
    };
  } else if (image.format === ImageFormat.RGBA) {
    return {
      r: pixels[index],
      g: pixels[index + 1],
      b: pixels[index + 2],
      a: pixels[index + 3]
    };
  } else {
    return { gray: pixels[index] };
  }
}
```

### Writing Pixel Data

```javascript
function setPixel(image, x, y, color) {
  const bytesPerPixel = getBytesPerPixel(image.format);
  const index = (y * image.width + x) * bytesPerPixel;
  const pixels = new Uint8Array(image.pixels);

  if (image.format === ImageFormat.RGB) {
    pixels[index] = color.r;
    pixels[index + 1] = color.g;
    pixels[index + 2] = color.b;
  } else if (image.format === ImageFormat.RGBA) {
    pixels[index] = color.r;
    pixels[index + 1] = color.g;
    pixels[index + 2] = color.b;
    pixels[index + 3] = color.a || 255;
  } else {
    pixels[index] = color.gray;
  }
}
```

## Performance Considerations

1. **Async Operations**: All image processing operations are non-blocking
2. **Memory Management**: Large images are handled efficiently
3. **Batch Processing**: Process multiple images concurrently for better performance
4. **Image Size**: Maximum supported dimension is 8192x8192 pixels

## Error Handling

```javascript
try {
  const resized = await imageProcessor.resize(image, 200, 200);
} catch (error) {
  console.error('Image processing failed:', error);
}
```

## Best Practices

1. **Image Dimensions**: Always validate image dimensions before processing
2. **Memory Management**: Be mindful of memory when processing large images
3. **Format Conversion**: Convert to appropriate format before applying filters
4. **Batch Operations**: Use Promise.all() for concurrent batch processing
5. **Error Handling**: Always wrap async operations in try-catch blocks

## License

Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0
