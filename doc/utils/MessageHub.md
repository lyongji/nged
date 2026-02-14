# MessageHub

The `MessageHub` class is a singleton that manages application messages, logs, and errors. It provides a central place to report and retrieve status information.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class MessageHub
{
public:
  enum class Category { Log, Notice, Output, Count };
  enum class Verbosity { Trace, Debug, Info, Warning, Error, Fatal, Text, Count };
  // ...
  static MessageHub& instance();
};
```

## Key Responsibilities

-   **Logging**: Records messages with different severity levels (Trace, Debug, Info, Warning, Error, Fatal).
-   **Categorization**: Groups messages into categories (Log, Notice, Output).
-   **Storage**: Stores a history of messages.
-   **Thread Safety**: Uses a mutex to ensure safe concurrent access.

## Public Methods

### Logging

-   `void addMessage(String message, Category category, Verbosity verbose)`: Adds a message to the hub.
-   `static void trace(String msg)`: Logs a trace message.
-   `static void debug(String msg)`: Logs a debug message.
-   `static void info(String msg)`: Logs an info message.
-   `static void warn(String msg)`: Logs a warning message.
-   `static void error(String msg)`: Logs an error message.
-   `static void fatal(String msg)`: Logs a fatal error message.
-   `static void notice(String msg)`: Logs a user notice.
-   `static void output(String msg)`: Logs program output.

### Formatting Helpers

There are also formatting variants of the logging functions (e.g., `tracef`, `debugf`, etc.) that accept format strings and arguments (similar to `fmt::format` or `printf`).

### Access & Management

-   `void clear(Category category)`: Clears messages of a specific category.
-   `void clearAll()`: Clears all messages.
-   `void setCountLimit(size_t count)`: Sets the maximum number of messages to store.
-   `size_t count(Category category) const`: Returns the number of messages in a category.

### Iteration

-   `template<class F> void foreach(Category category, F&& func) const`: Iterates over messages in a category.
-   `template<class F> void forrange(Category category, F&& func, size_t offset, size_t count) const`: Iterates over a range of messages.
