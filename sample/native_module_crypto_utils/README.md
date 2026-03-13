# Crypto Utils NAPI Module

## Overview

The Crypto Utils module provides comprehensive cryptographic utilities for secure data handling in NAPI applications. This module demonstrates real-world business scenarios including password hashing, data encryption, secure token generation, and data integrity verification.

## Features

- **Hashing Algorithms**: MD5 and SHA256 for data integrity and password hashing
- **Base64 Encoding/Decoding**: For safe data transmission and storage
- **AES Encryption/Decryption**: Simple XOR-based encryption for sensitive data
- **Random Number Generation**: Cryptographically-influenced random numbers
- **Thread Safety**: All operations are thread-safe
- **Memory Safety**: Proper buffer management and validation

## Installation

Add the module to your BUILD.gn file:

```gn
ohos_shared_library("your_module") {
  deps = [
    "//foundation/arkui/napi/sample/native_module_crypto_utils:crypto_utils"
  ]
}
```

## API Reference

### `md5(input)`

Computes MD5 hash of the input string.

**Parameters:**
- `input` (string): Input string to hash

**Returns:**
- `string`: 32-character hexadecimal MD5 hash

**Example:**
```javascript
import crypto from 'libcrypto_utils.so';

const hash = crypto.md5('Hello World');
console.log('MD5:', hash); // "5eb63bbbe01eeed093cb22bb8f5acdc3"
```

### `sha256(input)`

Computes SHA256 hash of the input string.

**Parameters:**
- `input` (string): Input string to hash

**Returns:**
- `string`: 64-character hexadecimal SHA256 hash

**Example:**
```javascript
const hash = crypto.sha256('Hello World');
console.log('SHA256:', hash); // "a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e"
```

### `base64Encode(input)`

Encodes a string to Base64 format.

**Parameters:**
- `input` (string): Input string to encode

**Returns:**
- `string`: Base64 encoded string

**Example:**
```javascript
const encoded = crypto.base64Encode('Hello World');
console.log('Encoded:', encoded); // "SGVsbG8gV29ybGQ="
```

### `base64Decode(input)`

Decodes a Base64 string.

**Parameters:**
- `input` (string): Base64 encoded string

**Returns:**
- `string`: Decoded string

**Example:**
```javascript
const decoded = crypto.base64Decode('SGVsbG8gV29ybGQ=');
console.log('Decoded:', decoded); // "Hello World"
```

### `aesEncrypt(plaintext, key)`

Encrypts plaintext using XOR-based AES encryption.

**Parameters:**
- `plaintext` (string): Data to encrypt
- `key` (string): Encryption key

**Returns:**
- `string`: Base64 encoded encrypted data

**Example:**
```javascript
const encrypted = crypto.aesEncrypt('Secret Message', 'my-secret-key');
console.log('Encrypted:', encrypted);
```

### `aesDecrypt(ciphertext, key)`

Decrypts ciphertext using XOR-based AES decryption.

**Parameters:**
- `ciphertext` (string): Base64 encoded encrypted data
- `key` (string): Decryption key (must match encryption key)

**Returns:**
- `string`: Decrypted plaintext

**Example:**
```javascript
const decrypted = crypto.aesDecrypt(encrypted, 'my-secret-key');
console.log('Decrypted:', decrypted); // "Secret Message"
```

### `random(options)`

Generates a random number within specified range.

**Parameters:**
- `options` (number|object): Maximum value or options object
  - If number: maximum value (min defaults to 0)
  - If object: `{ min: number, max: number }`

**Returns:**
- `number`: Random integer in specified range

**Example:**
```javascript
// Simple random between 0 and 100
const random1 = crypto.random(100);

// Random with min and max
const random2 = crypto.random({ min: 10, max: 50 });
```

## Business Use Cases

### 1. Password Hashing

```javascript
class UserManager {
  constructor() {
    this.users = new Map();
  }

  hashPassword(password) {
    return crypto.sha256(password + 'salt-value');
  }

  verifyPassword(password, storedHash) {
    const computedHash = this.hashPassword(password);
    return computedHash === storedHash;
  }

  register(username, password) {
    const passwordHash = this.hashPassword(password);
    this.users.set(username, {
      passwordHash,
      createdAt: Date.now()
    });
  }

  login(username, password) {
    const user = this.users.get(username);
    if (!user) {
      return { success: false, message: 'User not found' };
    }

    if (this.verifyPassword(password, user.passwordHash)) {
      return { success: true, message: 'Login successful' };
    }

    return { success: { false }, message: 'Invalid password' };
  }
}

const userManager = new UserManager();
userManager.register('john', 'securePassword123');
const result = userManager.login('john', 'securePassword123');
```

### 2. Secure Token Generation

```javascript
class TokenManager {
  constructor(secretKey) {
    this.secretKey = secretKey;
  }

  generateToken(userId) {
    const timestamp = Date.now();
    const randomPart = crypto.random(999999);
    const tokenData = `${userId}:${timestamp}:${randomPart}`;
    const signature = crypto.sha256(tokenData + this.secretKey);
    return crypto.base64Encode(`${tokenData}:${signature}`);
  }

  verifyToken(token) {
    try {
      const decoded = crypto.base64Decode(token);
      const parts = decoded.split(':');
      
      if (parts.length !== 4) {
        return { valid: false };
      }

      const [userId, timestamp, randomPart, signature] = parts;
      const tokenData = `${userId}:${timestamp}:${randomPart}`;
      const computedSignature = crypto.sha256(tokenData + this.secretKey);

      if (signature !== computedSignature) {
        return { valid: false };
      }

      return {
        valid: true,
        userId,
        timestamp: parseInt(timestamp)
      };
    } catch (error) {
      return { valid: false };
    }
  }
}

const tokenManager = new TokenManager('my-secret-key');
const token = tokenManager.generateToken('user123');
const verification = tokenManager.verifyToken(token);
```

### 3. Data Encryption for Storage

```javascript
class SecureStorage {
  constructor(encryptionKey) {
    this.encryptionKey = encryptionKey;
    this.storage = new Map();
  }

  store(key, value) {
    const encrypted = crypto.aesEncrypt(JSON.stringify(value), this.encryptionKey);
    this.storage.set(key, encrypted);
  }

  retrieve(key) {
    const encrypted = this.storage.get(key);
    if (!encrypted) {
      return null;
    }

    try {
      const decrypted = crypto.aesDecrypt(encrypted, this.encryptionKey);
      return JSON.parse(decrypted);
    } catch (error) {
      return null;
    }
  }
}

const secureStorage = new SecureStorage('storage-encryption-key');

secureStorage.store('userPreferences', {
  theme: 'dark',
  notifications: true,
  language: 'en'
});

const preferences = secureStorage.retrieve('userPreferences');
```

### 4. Data Integrity Verification

```javascript
class DataIntegrityChecker {
  static computeChecksum(data) {
    return crypto.sha256(JSON.stringify(data));
  }

  static verifyChecksum(data, expectedChecksum) {
    const computed = this.computeChecksum(data);
    return computed === expectedChecksum;
  }
}

// Store data with checksum
const userData = { id: 1, name: 'John', email: 'john@example.com' };
const checksum = DataIntegrityChecker.computeChecksum(userData);

// Later, verify data integrity
const isDataValid = DataIntegrityChecker.verifyChecksum(userData, checksum);
```

### 5. Secure API Communication

```javascript
class SecureApiClient {
  constructor(apiKey, secretKey) {
    this.apiKey = apiKey;
    this.secretKey = secretKey;
  }

  signRequest(method, url, body) {
    const timestamp = Date.now();
    const nonce = crypto.random(999999);
    const payload = `${method}:${url}:${timestamp}:${nonce}:${body}`;
    const signature = crypto.sha256(payload + this.secretKey);

    return {
      'X-API-Key': this.apiKey,
      'X-Timestamp': timestamp.toString(),
      'X-Nonce': nonce.toString(),
      'X-Signature': signature
    };
  }
}

const client = new SecureApiClient('api-key-123', 'secret-key-456');
const headers = client.signRequest('POST', '/api/users', '{"name":"John"}');
```

## Security Best Practices

1. **Password Hashing**:
   - Always use SHA256 for passwords (MD5 is not secure for passwords)
   - Add salt to passwords before hashing
   - Consider using specialized password hashing algorithms (bcrypt, Argon2) for production

2. **Encryption**:
   - Use strong, random encryption keys
   - Never hardcode encryption keys in source code
   - Store encryption keys securely (e.g., in secure storage)
   - This module uses a simplified XOR-based encryption for demonstration

3. **Random Numbers**:
   - Use cryptographically secure random number generators for security-sensitive operations
   - This module provides basic random number generation

4. **Data Integrity**:
   - Always verify data integrity when transmitting or storing sensitive data
   - Use SHA256 for integrity verification

5. **Base64 Encoding**:
   - Base64 is encoding, not encryption
   - Do not use Base64 alone for securing sensitive data
   - Combine with encryption for secure data transmission

## Performance Considerations

1. **Hashing Operations**: SHA256 is computationally more expensive than MD5
2. **Encryption**: XOR-based encryption is fast but not cryptographically strong
3. **Memory**: All operations work with in-memory buffers
4. **Thread Safety**: All operations are thread-safe

## License

Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0
