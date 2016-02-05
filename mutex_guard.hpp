#pragma once
#include <mutex>
#include <memory>


namespace fcl {
template <typename Ty>
class mutex_guard {
public:
    using mutex_t = std::recursive_mutex;

    template <typename InnerTy>
    class lock_guard_base {
        friend class mutex_guard;

    public:
        lock_guard_base(const lock_guard_base &) = delete;
        lock_guard_base &operator =(const lock_guard_base &) = delete;

        lock_guard_base(lock_guard_base &&) = default;
        lock_guard_base &operator =(lock_guard_base &&) = default;

        const InnerTy *operator->() const {
            assert(owns_lock());
            return unsafe_ptr();
        }

        ///\note You can use it wrong. At your own risk of course.
        const InnerTy *unsafe_ptr() const {
            assert(owns_lock());
            return &m_data;
        }

        ///\note You can use it wrong. At your own risk of course.
        const InnerTy &unsafe_ref() const {
            assert(owns_lock());
            return m_data;
        }

        /// Use it if you have tried to lock.
        bool owns_lock() const {
            return m_lock.owns_lock();
        }

    protected:
        lock_guard_base(mutex_t &mutex, InnerTy &data)
            : m_lock(mutex)
            , m_data(data) {}

        lock_guard_base(mutex_t &mutex, std::try_to_lock_t, InnerTy &data)
            : m_lock(mutex, std::try_to_lock)
            , m_data(data) {}

        std::unique_lock<mutex_t> m_lock;
        InnerTy &m_data;
    };

    class lock_guard : public lock_guard_base<Ty> {
        friend class mutex_guard;

    public:
        lock_guard(const lock_guard &) = delete;
        lock_guard &operator =(const lock_guard &) = delete;

        lock_guard(lock_guard &&) = default;
        lock_guard &operator =(lock_guard &&) = default;

        Ty *operator->() {
            assert(this->owns_lock());
            return unsafe_ptr();
        }

        Ty &unsafe_ref() {
            assert(this->owns_lock());
            return this->m_data;
        }

        Ty *unsafe_ptr() {
            assert(this->owns_lock());
            return &this->m_data;
        }

    private:
        lock_guard(mutex_t &mutex, Ty &data)
            : lock_guard_base<Ty>(mutex, data) {}

        lock_guard(mutex_t &mutex, std::try_to_lock_t, Ty &data)
            : lock_guard_base<Ty>(mutex, std::try_to_lock, data) {}
    };

    class clock_guard : public lock_guard_base<const Ty> {
        friend class mutex_guard;

    public:
        clock_guard(const clock_guard &) = delete;
        clock_guard &operator =(const clock_guard &) = delete;

        clock_guard(clock_guard &&) = default;
        clock_guard &operator =(clock_guard &&) = default;

    private:
        clock_guard(mutex_t &mutex, const Ty &data)
            : lock_guard_base<const Ty>(mutex, data) {}

        clock_guard(mutex_t &mutex, std::try_to_lock_t, const Ty &data)
            : lock_guard_base<const Ty>(mutex, std::try_to_lock, data) {}
    };

    ///\todo Replace std::unique_ptr with {std,boost}::optional in the name of obviousness
    using opt_lock_guard = std::unique_ptr<lock_guard>;
    using opt_clock_guard = std::unique_ptr<clock_guard>;

public:
    mutex_guard() = default;

    mutex_guard(Ty &&data)
        : m_data(data) {}

    mutex_guard(const Ty &data)
        : m_data(data) {}

    template <typename ...Args>
    mutex_guard(Args &&...args)
        : m_data(std::forward<Args>(args)...) {}

    mutex_guard(const mutex_guard &) = delete;
    mutex_guard &operator=(const mutex_guard &) = delete;

    lock_guard lock() {
        return { m_mutex, m_data };
    }

    opt_lock_guard try_to_lock() {
        auto guard = lock_guard{ m_mutex, std::try_to_lock, m_data };
        if (guard.owns_lock()) {
            return { guard };
        }
        return nullptr;
    }

    clock_guard lock() const {
        return { m_mutex, m_data };
    }

    opt_clock_guard try_to_lock() const {
        auto guard = clock_guard{ m_mutex, std::try_to_lock, m_data };
        if (guard.owns_lock()) {
            return { guard };
        }
        return nullptr;
    }

    Ty &nolock_unsafe_ref() {
        return m_data;
    }

    const Ty &nolock_unsafe_ref() const {
        return m_data;
    }

    const Ty *nolock_unsafe_ptr() const {
        return &m_data;
    }

    Ty *nolock_unsafe_ptr() {
        return &m_data;
    }

private:
    Ty m_data;
    mutable mutex_t m_mutex;
};
}
