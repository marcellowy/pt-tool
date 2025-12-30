#ifndef AV_QUEUE_H_
#define AV_QUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class Queue {
public:
    Queue() = default;
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    // 入队（拷贝）
    void push(const T& value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(value);
        }
        cond_.notify_one();
    }

    // 入队（移动）
    void push(T&& value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cond_.notify_one();
    }

    // 阻塞式出队
    // 如果队列被关闭且为空，返回 false
    bool pop(T& out) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [&] {
            return closed_ || !queue_.empty();
        });

        if (queue_.empty()) {
            return false; // closed
        }

        out = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    // 非阻塞出队
    bool try_pop(T& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }

        out = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    // 关闭队列（唤醒所有等待线程）
    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        cond_.notify_all();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable cond_;
    bool closed_ = false;
};

#endif
