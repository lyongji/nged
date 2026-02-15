#pragma once
#include <atomic>
#include <functional>
#include <vector>
#include <algorithm>
#include <memory>

namespace nged {

using EventHandle = int;

template<typename... Args>
class Signal {
public:
    using Slot = std::function<void(Args...)>;
    
    EventHandle connect(Slot slot) {
        static std::atomic<EventHandle> nextHandle{0};
        auto handle = ++nextHandle;
        slots_.emplace_back(handle, std::move(slot));
        return handle;
    }

    void disconnect(EventHandle handle) {
        slots_.erase(std::remove_if(slots_.begin(), slots_.end(),
            [handle](const auto& pair) { return pair.first == handle; }), slots_.end());
    }

    void emit(Args... args) const {
        // Copy to allow modification during emit
        auto currentSlots = slots_; 
        for (const auto& pair : currentSlots) {
            pair.second(args...);
        }
    }
    
    void operator()(Args... args) const {
        emit(args...);
    }

    void clear() {
        slots_.clear();
    }

    bool empty() const {
        return slots_.empty();
    }

private:
    std::vector<std::pair<EventHandle, Slot>> slots_;
};

template<typename... Args>
class Request {
public:
    using Handler = std::function<bool(Args...)>;

    EventHandle connect(Handler handler) {
        static std::atomic<EventHandle> nextHandle{0};
        auto handle = ++nextHandle;
        handlers_.emplace_back(handle, std::move(handler));
        return handle;
    }

    void disconnect(EventHandle handle) {
        handlers_.erase(std::remove_if(handlers_.begin(), handlers_.end(),
            [handle](const auto& pair) { return pair.first == handle; }), handlers_.end());
    }

    // Returns true if all handlers return true
    bool invoke(Args... args) const {
        auto currentHandlers = handlers_;
        for (const auto& pair : currentHandlers) {
            if (!pair.second(args...)) {
                return false;
            }
        }
        return true;
    }
    
    bool operator()(Args... args) const {
        return invoke(args...);
    }

    void clear() {
        handlers_.clear();
    }

    bool empty() const {
        return handlers_.empty();
    }

private:
    std::vector<std::pair<EventHandle, Handler>> handlers_;
};

} // namespace nged