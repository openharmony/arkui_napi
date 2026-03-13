# Thread-Safe Async Example

This example demonstrates thread-safe asynchronous operations in NAPI modules, including async work queues, thread-safe functions, and parallel computation.

## Features

- Asynchronous work execution with napi_async_work
- Thread-safe function calls with napi_threadsafe_function
- Parallel computation with multiple threads
- Proper synchronization and resource cleanup

## API

### asyncDouble(value: number, callback: (error: Error | null, result: number) => void): void

Performs asynchronous computation (doubling the value) using napi_async_work.

**Parameters:**
- `value` - Number to double
- `callback` - Callback function that receives `(error, result)`

**Example:**
```javascript
import threadsafeAsync from 'libthreadsafe_async.so';

threadsafeAsync.asyncDouble(21, (error, result) => {
    if (error) {
        console.error("Error:", error);
    } else {
        console.log("Result:", result);
    }
});
```

### threadSafeAsyncDouble(value: number, callback: (error: Error | null, result: number) => void): void

Performs asynchronous computation using thread-safe functions, allowing calls from any thread.

**Parameters:**
- `value` - Number to double
- `callback` - Callback function that receives `(error, result)`

**Example:**
```javascript
threadsafeAsync.threadSafeAsyncDouble(15, (error, result) => {
    if (error) {
        console.error("Error:", error);
    } else {
        console.log("Result:", result);
    }
});
```

### parallelCompute(iterations: number, callback: (error: Error | null, results: number[]) => void): void

Performs parallel computation across multiple threads, each computing the square of its index.

**Parameters:**
- `iterations` - Number of parallel computations to perform
- `callback` - Callback function that receives `(error, results)`

**Example:**
```javascript
threadsafeAsync.parallelCompute(5, (error, results) => {
    if (error) {
        console.error("Error:", error);
    } else {
        console.log("Results:", results);
    }
});
```

## Complete Example

```javascript
import threadsafeAsync from 'libthreadsafe_async.so';

console.log("Starting async operations...");

threadsafeAsync.asyncDouble(21, (error, result) => {
    if (error) {
        console.error("asyncDouble error:", error);
    } else {
        console.log("asyncDouble result:", result);
    }
});

threadsafeAsync.threadSafeAsyncDouble(15, (error, result) => {
    if (error) {
        console.error("threadSafeAsyncDouble error:", error);
    } else {
        console.log("threadSafeAsyncDouble result:", result);
    }
});

threadsafeAsync.parallelCompute(5, (error, results) => {
    if (error) {
        console.error("parallelCompute error:", error);
    } else {
        console.log("parallelCompute results:", results);
    }
});
```

## Implementation Details

### Async Work Pattern

The `asyncDouble` function uses the traditional async work pattern:
1. Creates an AsyncWorkData structure to hold input/output
2. Creates a reference to the callback function
3. Creates an async work with execute and complete callbacks
4. Queues the work for execution
5. Cleans up resources in the complete callback

### Thread-Safe Function Pattern

The `threadSafeAsyncDouble` function uses thread-safe functions:
1. Creates a thread-safe function from the JavaScript callback
2. Spawns a worker thread to perform computation
3. Calls `napi_call_threadsafe_function` to invoke the callback
4. The thread-safe function handles JavaScript environment synchronization

### Parallel Computation Pattern

The `parallelCompute` function demonstrates multi-threading:
1. Creates multiple threads for parallel computation
2. Uses atomic counter for thread-safe increments
3. Uses mutex to protect shared results vector
4. Joins all threads before invoking callback

## Thread Safety Considerations

1. **JavaScript Environment**: Only interact with JavaScript objects from the main thread or through thread-safe functions
2. **Shared Data**: Use mutex or atomic operations for shared data
3. **Resource Cleanup**: Ensure all resources are freed properly
4. **Handle Scopes**: Use handle scopes when calling into JavaScript from worker threads

## Error Handling

All async operations follow the Node.js callback pattern:
- First parameter is error (null if successful)
- Second parameter is the result (undefined if error occurred)

## Performance Notes

- Thread-safe functions have overhead but provide safety
- Parallel computation can improve CPU utilization
- Always balance thread count with available cores
- Consider using thread pools for frequent async operations
