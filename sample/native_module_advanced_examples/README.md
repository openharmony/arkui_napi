# NAPI Advanced Examples

This directory contains advanced NAPI module examples demonstrating various native module features and best practices.

## Examples

### Event Notification
Demonstrates event-driven architecture with callback support and thread-safe event queuing.

**Location:** `event_notification/`

**Key Features:**
- Event-driven communication between native and JavaScript
- Worker thread for event processing
- Thread-safe event queue with mutex and condition variable
- Support for multiple event types

**Build:** `libevent_notification.so`

### Memory Management
Shows proper memory management techniques including object lifecycle, reference counting, and external memory handling.

**Location:** `memory_management/`

**Key Features:**
- Object lifecycle management with smart pointers
- Reference counting for JavaScript objects
- External ArrayBuffer creation with custom cleanup
- Memory leak prevention patterns

**Build:** `libmemory_management.so`

### Thread-Safe Async
Demonstrates thread-safe asynchronous operations including async work queues and parallel computation.

**Location:** `threadsafe_async/`

**Key Features:**
- Asynchronous work execution with napi_async_work
- Thread-safe function calls with napi_threadsafe_function
- Parallel computation with multiple threads
- Proper synchronization and resource cleanup

**Build:** `libthreadsafe_async.so`

## Building the Examples

Each example has its own BUILD.gn file. To build all examples:

```bash
# From the project root
hb build napi -t
```

Or build individual examples:

```bash
# Build specific example
hb build napi_packages --build-target event_notification
hb build napi_packages --build-target memory_management
hb build napi_packages --build-target threadsafe_async
```

## Using the Examples

After building, the shared libraries can be imported in JavaScript:

```javascript
import eventNotification from 'libevent_notification.so';
import memoryManagement from 'libmemory_management.so';
import threadsafeAsync from 'libthreadsafe_async.so';
```

See individual README.md files in each example directory for detailed API documentation and usage examples.

## Common Patterns

### Error Handling

All examples follow consistent error handling patterns:

```cpp
if (argc < expected) {
    napi_throw_error(env, nullptr, "Expected N arguments");
    return nullptr;
}

napi_valuetype valuetype;
napi_typeof(env, args[0], &valuetype);
if (valuetype != napi_function) {
    napi_throw_error(env, nullptr, "Argument must be a function");
    return nullptr;
}
```

### Resource Cleanup

Proper cleanup of resources:

```cpp
if (asyncData->work) {
    napi_delete_async_work(env, asyncData->work);
}
if (asyncData->callbackRef) {
    napi_delete_reference(env, asyncData->callbackRef);
}
if (asyncData->tsfn) {
    napi_release_threadsafe_function(asyncData->tsfn, napi_tsfn_release);
}
```

### Thread Safety

Thread-safe operations with proper synchronization:

```cpp
std::mutex mutex_;
std::lock_guard<std::mutex> lock(mutex_);
```

## Best Practices

1. **Always validate input parameters** before processing
2. **Use smart pointers** (std::unique_ptr, std::shared_ptr) for automatic memory management
3. **Clean up resources** in reverse order of acquisition
4. **Use thread-safe functions** when calling JavaScript from worker threads
5. **Implement proper error handling** and propagate errors to JavaScript
6. **Document all APIs** with clear parameter and return value descriptions
7. **Follow NAPI conventions** for callback patterns (error-first callbacks)
8. **Test thoroughly** especially for edge cases and error conditions

## Additional Resources

- [NAPI Documentation](https://nodejs.org/api/n-api.html)
- [NAPI Reference](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/napi-guidelines-V5)
- [OpenHarmony Native Development](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-V5/ndk-development-guidelines-V5)

## License

Copyright (c) 2026 Huawei Device Co., Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
