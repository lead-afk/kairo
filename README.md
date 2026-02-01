# Kairo

A high-performance C++ library of reusable, optimized components designed to reduce boilerplate and provide out-of-the-box solutions for common tasks.

## Requirements

- C++20 or later
- CMake 3.15+

## Other libraries

## Features

- **🚀 High Performance:** Heavily optimized for speed.
- **✨ Low Boilerplate:** Clean, easy-to-use interfaces.
- **🛠️ Batteries Included:** Ready-to-use classes for specific tasks.

## Modules

### `kairo::ds` (Data Structures)

Concurrency primitives and data structures.

- **ThreadSafeQueue**: High-performance, thread-safe bounded queue.
- **ThreadPool**: Efficient worker pool for parallel task execution.
- **Counter_guard**: RAII wrapper for managing atomic counters.

### `kairo::ip` (Networking)

Networking utilities and implementations.

- **WebsocketClient**: Simple, robust WebSocket client.
- **WebsocketServer**: Lightweight WebSocket server implementation.

## File Structure

- [kairo/](./include/kairo)
  - [ds/](./include/kairo/ds)
    - [CounterGuard.hpp](./include/kairo/ds/CounterGuard.hpp)
    - [ThreadPool.hpp](./include/kairo/ds/ThreadPool.hpp)
    - [ThreadSafeQueue.hpp](./include/kairo/ds/ThreadSafeQueue.hpp)
  - [ip/](./include/kairo/ip)
    - [WebsocketClient.hpp](./include/kairo/ip/WebsocketClient.hpp)
    - [WebsocketServer.hpp](./include/kairo/ip/WebsocketServer.hpp)
