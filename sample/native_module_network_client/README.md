# Network Client NAPI Module

## Overview

The Network Client module provides a comprehensive HTTP client implementation for making network requests in NAPI applications. This module demonstrates real-world business scenarios including REST API integration, data fetching, and async network operations.

## Features

- **HTTP Methods Support**: GET, POST, PUT, DELETE
- **Async/Promise Support**: Both Promise-based and callback-based async operations
- **Headers Management**: Custom request headers support
- **JSON Handling**: Automatic JSON request/response handling
- **Error Handling**: Comprehensive error reporting with status codes
- **Type Safety**: Strong typing for request/response data

## Installation

Add the module to your BUILD.gn file:

```gn
ohos_shared_library("your_module") {
  deps = [
    "//foundation/arkui/napi/sample/native_module_network_client:network_client"
  ]
}
```

## API Reference

### `get(url, optionsOrCallback)`

Performs an HTTP GET request.

**Parameters:**
- `url` (string): The URL to request
- `optionsOrCallback` (object|function, optional): Options object or callback function

**Returns:**
- `Promise<object>`: Promise resolving to response object

**Example (Promise):**
```javascript
import networkClient from 'libnetwork_client.so';

try {
  const response = await networkClient.get('https://api.example.com/users');
  {
    "success": true,
    "statusCode": 200,
    "data": "{\"url\":\"https://api.example.com/users\",\"method\":\"GET\",\"status\":200,\"timestamp\":1234567890,\"message\":\"Request processed successfully\"}"
  }
  
  const parsedData = JSON.parse(response.data);
  console.log('User data:', parsedData);
} catch (error) {
  console.error('Request failed:', error);
}
```

**Example (with headers):**
```javascript
const response = await networkClient.get('https://api.example.com/data', {
  'Authorization': 'Bearer token123',
  'Accept': 'application/json'
});
```

**Example (Callback):**
```javascript
networkClient.get('https://api.example.com/users', (error, response) => {
  if (error) {
    console.error('Request failed:', error);
    return;
  }
  console.log('Response:', response);
});
```

### `post(url, body, headers)`

Performs an HTTP POST request.

**Parameters:**
- `url` (string): The URL to request
- `body` (string): Request body data (typically JSON string)
- `headers` (object, optional): Custom headers

**Returns:**
- `Promise<object>`: Promise resolving to response object

**Example:**
```javascript
const userData = {
  name: 'John Doe',
  email: 'john@example.com'
};

const response = await networkClient.post(
  'https://api.example.com/users',
  JSON.stringify(userData),
  {
    'Content-Type': 'application/json',
    'Authorization': 'Bearer token123'
  }
);

if (response.success) {
  const result = JSON.parse(response.data);
  console.log('User created:', result);
}
```

### `request(options)`

Performs a custom HTTP request with full options.

**Parameters:**
- `options` (object): Request options
  - `url` (string): Request URL
  - `method` (string): HTTP method (GET, POST, PUT, DELETE)
  - `body` (string, optional): Request body
  - `headers` (object, optional): Request headers

**Returns:**
- `Promise<object>`: Promise resolving to response object

**Example:**
```javascript
const response = await networkClient.request({
  url: 'https://api.example.com/users/123',
  method: 'PUT',
  body: JSON.stringify({ name: 'Updated Name' }),
  headers: {
    'Content-Type': 'application/json',
    'Authorization': 'Bearer token123'
  }
});

if (response.success) {
  console.log('Update successful');
}
```

## Response Object Structure

```typescript
interface HttpResponse {
  success: boolean;           // true if status code is 2xx
  statusCode: number;          // HTTP status code
  data: string;               // Response data (typically JSON string)
}
```

## Business Use Cases

### 1. REST API Integration

```javascript
class ApiService {
  constructor(baseUrl, authToken) {
    this.baseUrl = baseUrl;
    this.authToken = authToken;
  }

  async getUsers() {
    const response = await networkClient.get(`${this.baseUrl}/users`, {
      'Authorization': `Bearer ${this.authToken}`
    });
    return JSON.parse(response.data);
  }

  async createUser(userData) {
    const response = await networkClient.post(
      `${this.baseUrl}/users`,
      JSON.stringify(userData),
      {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${this.authToken}`
      }
    );
    return JSON.parse(response.data);
  }
}

const api = new ApiService('https://api.example.com', 'your-token');
const users = await api.getUsers();
```

### 2. Data Synchronization

```javascript
async function syncDataWithServer(localData) {
  try {
    const response = await networkClient.post(
      'https://sync.example.com/upload',
      JSON.stringify(localData),
      {
        'Content-Type': 'application/json',
        'X-Sync-Version': '1.0'
      }
    );

    if (response.success) {
      console.log('Data synchronized successfully');
      return true;
    }
  } catch (error) {
    console.error('Sync failed:', error);
    return false;
  }
}
```

### 3. Batch Operations

```javascript
async function fetchMultipleUrls(urls) {
  const promises = urls.map(url => 
    networkClient.get(url).catch(error => ({ error, url }))
  );

  const results = await Promise.all(promises);
  
  const successful = results.filter(r => r.success);
  const failed = results.filter(r => !r.success);

  console.log(`Successful: ${successful.length}, Failed: ${failed.length}`);
  return { successful, failed };
}
```

## Error Handling

The module provides comprehensive error handling:

```javascript
try {
  const response = await networkClient.get('https://api.example.com/data');
  
  if (!response.success) {
    // Handle HTTP errors (4xx, 5xx)
    switch (response.statusCode) {
      case 404:
        console.error('Resource not found');
        break;
      case 500:
        console.error('Server error');
        break;
      default:
        console.error(`HTTP error: ${response.statusCode}`);
    }
  }
} catch (error) {
  // Handle network errors, invalid URLs, etc.
  console.error('Network error:', error);
}
```

## Performance Considerations

1. **Async Operations**: All network operations are non-blocking
2. **Memory Management**: Proper cleanup of async work and callbacks
3. **Thread Safety**: Safe for concurrent requests

## Security Best Practices

1. Always use HTTPS in production
2. Implement proper authentication headers
3. Validate and sanitize request data
4. Handle sensitive data securely
5. Implement proper error handling to avoid information leakage

## License

Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0
