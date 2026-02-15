#pragma once

#include "types.h"
#include <chrono>
#include <deque>
#include <shared_mutex>

namespace nged {

// MessageHub {{{
class MessageHub
{
public:
  ~MessageHub() = default;
  enum class Category : int
  {
    Log = 0,
    Notice,
    Output,

    Count
  };
  enum class Verbosity
  {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,

    Text,

    Count
  };
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
  struct Message
  {
    Message(String s, Verbosity v, TimePoint t) : content(std::move(s)), verbosity(v), timestamp(t)
    {
    }
    Message(Message&& m) = default;

    String    content;
    Verbosity verbosity;
    TimePoint timestamp;
  };

  void addMessage(String message, Category category, Verbosity verbose);
  void clear(Category category);
  void clearAll();
  void setCountLimit(size_t count);

  template<class F>
  void foreach (Category category, F && func) const
  {
    std::shared_lock lock(mutex_);
    for (auto&& s : messageCategories_[static_cast<int>(category)]) {
      func(s);
    }
  }
  template<class F>
  void forrange(Category category, F&& func, size_t offset, size_t count = std::numeric_limits<size_t>::max()) const
  {
    std::shared_lock lock(mutex_);
    auto const&      queue = messageCategories_[static_cast<int>(category)];
    for (size_t i = offset,
                n = count == std::numeric_limits<size_t>::max() ? queue.size() : std::min(offset + count, queue.size());
         i < n;
         ++i) {
      func(queue[i]);
    }
  }
  size_t count(Category category) const
  {
    std::shared_lock lock(mutex_);
    return messageCategories_[static_cast<int>(category)].size();
  }

  static MessageHub& instance() { return instance_; }

protected:
  std::deque<Message>       messageCategories_[static_cast<int>(Category::Count)];
  mutable std::shared_mutex mutex_;
  size_t                    countLimit_ = 4096;

  static MessageHub instance_;

private:
  MessageHub()                       = default;
  MessageHub(MessageHub const& that) = delete;

public: // helpers
#define EMIT_MESSAGE_(msg, cat, verb) \
  MessageHub::instance().addMessage((msg), Category::cat, Verbosity::verb)
#define DEFINE_MSG_VARIANT_(func, cat, verb)                                        \
  static inline void func(String msg) { EMIT_MESSAGE_(std::move(msg), cat, verb); } \
  template<class... T>                                                              \
  static inline void func##f(T... args)                                             \
  {                                                                                 \
    EMIT_MESSAGE_(fmt::format(std::forward<T>(args)...), cat, verb);                \
  }
  DEFINE_MSG_VARIANT_(trace, Log, Trace)
  DEFINE_MSG_VARIANT_(debug, Log, Debug)
  DEFINE_MSG_VARIANT_(info, Log, Info)
  DEFINE_MSG_VARIANT_(warn, Log, Warning)
  DEFINE_MSG_VARIANT_(error, Log, Error)
  DEFINE_MSG_VARIANT_(fatal, Log, Fatal)
  DEFINE_MSG_VARIANT_(notice, Notice, Text)
  DEFINE_MSG_VARIANT_(output, Output, Text)
#undef DEFINE_MSG_VARIANT_
#undef EMIT_MESSAGE_
};
// }}} MessageHub

} // namespace nged
