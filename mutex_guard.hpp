#pragma once
#include <mutex>


namespace fcl {
template <typename Ty>
class mutex_guard {
public:
    using mutex_t = std::mutex;

    class lock_guard {
        friend class mutex_guard;

    public:
        lock_guard(const lock_guard &) = delete;
        lock_guard &operator =(const lock_guard &) = delete;

        lock_guard(lock_guard &&) = delete;
        lock_guard &operator =(lock_guard &&) = delete;

        Ty *operator->() {
            return m_ptr.get();
        }

        const Ty *operator->() const {
            return m_ptr.get();
        }

        Ty &operator*() {
            return ref();
        }

        const Ty &operator*() const {
            return ref();
        }

        ///\note You can use it wrong. At your own risk of course.
        Ty *get() const {
            return m_ptr.get();
        }

        Ty &ref() {
            return *m_ptr;
        }

        const Ty &ref() const {
            return *m_ptr;
        }

    private:
        lock_guard(mutex_t &mutex, Ty &data)
            : m_lock(mutex)
            , m_data(data) {}

    private:
        std::lock_guard<mutex_t> m_lock;
        Ty &m_data;
    };

public:
    mutex_guard() = default;

    mutex_guard(Ty &&data)
        : m_data(data) {}

    mutex_guard(const Ty &data)
        : m_data(data) {}

    mutex_guard(const mutex_guard) = delete;
    mutex_guard &operator=(const mutex_guard &) = delete;

    lock_guard lock() const {
        return { *m_mutex, m_data };
    }

private:
    Ty m_data;
    mutex_t m_mutex;
};
}
