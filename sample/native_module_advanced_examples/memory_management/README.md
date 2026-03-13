# Memory Management Example

This example demonstrates proper memory management techniques in NAPI modules, including object lifecycle management, reference counting, and external memory handling.

## Features

- Object lifecycle management with smart pointers
- Reference counting for JavaScript objects
- External ArrayBuffer creation with custom cleanup
- Memory leak prevention patterns

## API

### createData(id: number, name: string): number

Creates a new managed data object and returns its index.

**Parameters:**
- `id` - Numeric identifier for the data
- `name` - String name for the data

**Returns:** Index of the created data object

**Example:**
```javascript
import memoryManagement from 'libmemory_management.so';

const index = memoryManagement.createData(1, "Test Data");
console.log(`Created data at index: ${index}`);
```

### getDataInfo(index: number): object

Retrieves information about a managed data object.

**Parameters:**
- `index` - Index of the data object

**Returns:** Object containing `id` and `name` properties

**Example:**
```javascript
const info = memoryManagement.getDataInfo(index);
console.log(`Data info:`, info);
```

### deleteData(index: number): boolean

Deletes a managed data object and frees its memory.

**Parameters:**
- `index` - Index of the data object to delete

**Returns:** `true` if successful, `false` if index is invalid

**Example:**
```javascript
const success = memoryManagement.deleteData(index);
if (success) {
    console.log("Data deleted successfully");
}
```

### clearCache(): void

Clears all managed data objects from the cache.

**Example:**
```javascript
memoryManagement.clearCache();
```

### getCacheSize(): number

Returns the number of data objects currently in the cache.

**Returns:** Number of data objects

**Example:**
```javascript
const size = memoryManagement.getCacheSize();
console.log(`Cache size: ${size}`);
```

### createArrayBuffer(size: number): ArrayBuffer

Creates a new ArrayBuffer with the specified size, managed by the JavaScript engine.

**Parameters:**
- `size` - Size in bytes (1 to 1MB)

**Returns:** ArrayBuffer object

**Example:**
```javascript
const buffer = memoryManagement.createArrayBuffer(1024);
const view = new Uint8Array(buffer);
view[0] = 42;
```

### createExternalArrayBuffer(size: number): ArrayBuffer

Creates an external ArrayBuffer with custom memory management. The memory is allocated with `malloc` and freed with a custom finalize callback.

**Parameters:**
- `size` - Size in bytes (1 to 1MB)

**Returns:** ArrayBuffer object with external memory

**Example:**
```javascript
const externalBuffer = memoryManagement.createExternalArrayBuffer(1024);
const view = new Uint8Array(externalBuffer);
console.log(`First byte: ${view[0]}`);
```

## Complete Example

```javascript
import memoryManagement from 'libmemory_management.so';

const index1 = memoryManagement.createData(1, "First Data");
const index2 = memoryManagement.createData(2, "Second Data");

console.log(`Cache size: ${memoryManagement.getCacheSize()}`);

const info1 = memoryManagement.getDataInfo(index1);
console.log(`Data 1:`, info1);

const buffer = memoryManagement.createArrayBuffer(256);
const externalBuffer = memoryManagement.createExternalArrayBuffer(256);

memoryManagement.deleteData(index1);
console.log(`Cache size after deletion: ${memoryManagement.getCacheSize()}`);

memoryManagement.clearCache();
```

## Memory Management Best Practices

1. **Use smart pointers**: Prefer `std::unique_ptr` and `std::shared_ptr` over raw pointers
2. **Implement cleanup**: Always provide cleanup callbacks for external memory
3. **Validate indices**: Check bounds before accessing indexed data
4. **Clear resources**: Ensure all resources are freed when no longer needed
5. **Limit allocations**: Set reasonable limits on memory allocations

## Implementation Details

The memory management system uses:
- `std::unique_ptr` for automatic memory cleanup
- RAII (Resource Acquisition Is Initialization) pattern
- Custom finalize callbacks for external ArrayBuffers
- Bounds checking for all indexed operations

## Thread Safety

The current implementation is not thread-safe. For multi-threaded use, add:
- Mutex protection for shared data structures
- Atomic operations for counters
- Thread-safe reference management
