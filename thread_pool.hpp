#pragma once
#include <thread>
#include <future>
#include <mutex>
#include <list>
#include <queue>
#include <atomic>
#include <cassert>

namespace fcl {
template <typename Signature>
class thread_pool;

template <typename Res, typename ...TaskArgs>
class thread_pool<Res (TaskArgs...)> {
    using task_mutex_t = std::mutex;
    using thread_mutex_t = std::mutex;

public:
    using task_result_t = Res;
    using signature_t = Res (TaskArgs...);
    using task_t = std::packaged_task<signature_t>;

    thread_pool(const size_t threads_nb = std::thread::hardware_concurrency())
        : m_should_stop(false) {
        assert(threads_nb > 0);
        for (size_t i = 0; i < threads_nb; ++i) {
            add_thread();
        }
    }

    thread_pool(const thread_pool &) = delete;
    thread_pool &operator =(const thread_pool &) = delete;
    thread_pool(thread_pool &&) = delete;
    thread_pool &operator =(thread_pool &&) = delete;
    ~thread_pool() {
        finish_and_stop();
        // stop();
    }

    template <typename Func, typename ...Args>
    std::future<task_result_t> emplace_task(Func &&func, Args &&...args) {
        return push_task(task_t(std::move(func), std::forward<Args>(args)...));
    }

    std::future<task_result_t> push_task(task_t &&task) {
        std::lock_guard<task_mutex_t> lock(m_tasks_mutex);
        m_tasks.push(std::move(task));
        return m_tasks.back().get_future();
    }

    void add_thread() {
        std::lock_guard<thread_mutex_t> thr_lk(m_threads_mutex);
        m_threads.emplace_back([this] {
            while (!m_should_stop) {
                if (m_tasks.empty()) {
                    std::this_thread::yield();
                    continue;
                }
                std::unique_lock<task_mutex_t> lk(m_tasks_mutex);
                task_t task = std::move(m_tasks.front());
                m_tasks.pop();
                lk.unlock();
                task();
            }
        });
    }

    void wait_for_all() const {
        while (has_tasks()) std::this_thread::yield();
    }

    void finish_and_stop() {
        wait_for_all();
        stop();
    }

    void stop() {
        m_should_stop.store(true);
        join_all();
    }

    bool has_tasks() const {
        return !m_tasks.empty();
    }

private:
    void join_all() {
        std::lock_guard<thread_mutex_t> thr_lk(m_threads_mutex);
        for (auto &thread: m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    std::atomic_bool m_should_stop;
    std::list<std::thread> m_threads;
    std::queue<task_t> m_tasks;
    task_mutex_t m_tasks_mutex;
    thread_mutex_t m_threads_mutex;
};
}
