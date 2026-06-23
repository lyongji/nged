# MessageHub

`MessageHub` 是一个单例，管理应用程序的消息、日志和错误。提供统一的报告和状态信息查询接口。

## 头文件

`#include "nged/nged.h"`

## 核心职责

- **分类日志**：支持三种消息类别（`Log`、`Notice`、`Output`）。
- **分级日志**：支持六种详细级别（`Trace` → `Fatal`）。
- **线程安全**：使用共享互斥锁保护消息队列。
- **数量限制**：可设置消息队列最大条目数。

## 类别与级别

```cpp
enum class Category { Log = 0, Notice, Output, Count };
enum class Verbosity { Trace = 0, Debug, Info, Warning, Error, Fatal, Text, Count };
```

## 使用

```cpp
// 日志消息
MessageHub::trace("vertex count: {}", 1234);
MessageHub::debugf("entering subgraph {}", ptr);
MessageHub::infof("shortcut for command {} triggered", name);
MessageHub::warnf("cannot rename node to {}", name);
MessageHub::error("failed to deserialize");

// 通知（用户可见）
MessageHub::notice("file saved successfully");

// 输出（结果展示）
MessageHub::outputf("result: {:.2f}", value);
```

## 公开方法

- `static MessageHub& instance()`：获取单例。
- `void addMessage(String, Category, Verbosity)`：添加消息。
- `void clear(Category)`：清空指定类别消息。
- `void clearAll()`：清空全部消息。
- `void setCountLimit(size_t)`：设置最大消息数（默认 4096）。
- `template<class F> void foreach(Category, F&&)`：遍历消息。
- `size_t count(Category) const`：返回消息数量。
