# operating-systems-2
# Dining Philosophers Problem and Resource Hierarchy Solution

## Introduction

The **Dining Philosophers Problem** is a classic synchronization problem in computer science, illustrating challenges in resource sharing and deadlock prevention. The scenario involves multiple philosophers sitting around a table, each requiring two chopsticks or forks (shared resources) to eat. Since each philosopher shares a chopstick with their neighbor, improper resource allocation can lead to **deadlock** (where no one can proceed) or **starvation** (where some philosophers never get to eat).

## Solution
This C++ implementation of the **Dining Philosophers Problem** ensures safe resource sharing using **mutexes** and **deadlock prevention** via a **resource hierarchy** (lower-numbered fork first). Each philosopher runs in a separate thread, alternating between **thinking** and **eating**, with `std::lock()` preventing circular wait conditions. A **console or table view** provides real-time status updates. The simulation runs for **x seconds**, then stops and displays eating statistics.

## Compiling
```
g++ -o output_file .\hungry_plilosophers.cpp --std=c++17
```

## Output
Console Table View:
```
Philosopher ID | State     | Eat Count
---------------|-----------|----------
Philosopher 0 |      EATING | 1
Philosopher 1 |    THINKING | 1
Philosopher 2 |      EATING | 2
Philosopher 3 |    THINKING | 2
Philosopher 4 |    THINKING | 1
Philosopher ID | State     | Eat Count
---------------|-----------|----------
```
or Console View:
```
Philosopher 2 is thinking.
Philosopher 3 is thinking.
Philosopher 1 is thinking.
Philosopher 4 is thinking.
Philosopher 0 is eating.
Philosopher 3 is eating.
Philosopher 0 has finished eating.
Philosopher 1 is eating.
Philosopher 0 is thinking.
Philosopher 1 has finished eating.
Philosopher 1 is thinking.
Philosopher 3 has finished eating.
Philosopher 3 is thinking.
Philosopher 2 is eating.
Philosopher 4 is eating.
Philosopher 4 has finished eating.
Philosopher 4 is thinking.
```

---
---

# Simple Chat Server

A basic console-based chat server implemented in C++ for Operating Systems class. This simplified version provides the core functionality with a single chat room.

## Features

- Multithreaded server (one thread per client)
- Message synchronization (new clients see message history)
- Single chat room for all connected clients

## Building

To build the server and client on Windows using MinGW:

```
mingw32-make
```

This will create two executables: `server.exe` and `client.exe`.

## Running the Server

To start the server:

```
server.exe [port]
```

If no port is specified, the server will use the default port 8080.

## Running the Client

To start the client:

```
client.exe [server_ip] [port]
```

If no server IP is specified, it will connect to localhost (127.0.0.1).
If no port is specified, it will use the default port 8080.

## Client Commands

- `/help` - Display help information
- `/exit` - Exit the client

Any other text is sent to all connected users.

## How It Works

1. The server creates a new thread for each client connection
2. Messages are synchronized between clients
3. All messages are stored in history and sent to new clients

## Next Steps

You can expand this basic chat server by:

1. Re-implementing multiple chat rooms functionality
2. Adding private messaging between clients
3. Adding user authentication
4. Implementing file transfer capability
5. Adding a graphical user interface 