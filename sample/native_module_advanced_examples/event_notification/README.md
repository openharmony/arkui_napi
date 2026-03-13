# Event Notification Example

This example demonstrates how to implement event notification mechanisms in NAPI modules, allowing native code to asynchronously notify JavaScript of events.

## Features

- Event-driven architecture with callback support
- Thread-safe event queue
- Worker thread for event processing
- Support for multiple event types

## API

### startEventNotifier(callback: Function): void

Starts the event notifier with a callback function that will be invoked when events are emitted.

**Parameters:**
- `callback` - Function to call when events are emitted. The callback receives three arguments:
  - `type` (number) - Event type identifier
  - `message` (string) - Event message
  - `value` (number) - Event value

**Example:**
```javascript
import eventNotification from 'libevent_notification.so';

eventNotification.startEventNotifier((type, message, value) => {
    console.log(`Event received: type=${type}, message=${message}, value=${value}`);
});
```

### stopEventNotifier(): void

Stops the event notifier and cleans up resources.

**Example:**
```javascript
eventNotification.stopEventNotifier();
```

### emitEvent(type: number, message: string, value: number): void

Emits an event to the registered callback.

**Parameters:**
- `type` - Event type identifier
- `message` - Event message
- `value` - Event value

**Example:**
```javascript
eventNotification.emitEvent(1, "Test event", 42);
```

## Complete Example

```javascript
import eventNotification from 'libevent_notification.so';

eventNotification.startEventNotifier((type, message, value) => {
    console.log(`Event ${type}: ${message} (value: ${value})`);
});

eventNotification.emitEvent(1, "First event", 100);
eventNotification.emitEvent(2, "Second event", 200);

setTimeout(() => {
    eventNotification.stopEventNotifier();
}, 1000);
```

## Implementation Details

The event notification system uses:
- A worker thread that processes events from a queue
- Thread-safe synchronization with mutex and condition variable
- NAPI reference to maintain callback across thread boundaries
- Handle scope management for safe JavaScript interaction

## Thread Safety

The implementation ensures thread safety by:
- Using mutex to protect shared data structures
- Using condition variable for efficient waiting
- Properly managing NAPI handle scopes in the worker thread
- Ensuring proper cleanup on module shutdown
