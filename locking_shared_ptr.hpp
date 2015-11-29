#pragma once
#include <memory>
#include <mutex>


namespace fcl {
    template <typename Ty>
    class locking_shared_ptr;

    struct use_own_lock_policy {};

    template <typename Ty>
    struct use_same_lock_policy {
        const locking_shared_ptr<Ty> &m_other;
        use_same_lock_policy(locking_shared_ptr<Ty> &other)
            : m_other(other) {}
    };

    template <typename Ty>
    auto use_same_lock(locking_shared_ptr<Ty> &other)
        -> use_same_lock_policy<Ty> {
        return use_same_lock_policy<Ty>(other);
    }

    template <typename Ty>
    class locking_shared_ptr {
        template <typename OtherTy>
        friend class locking_shared_ptr;

    public:
        using ptr_t = std::shared_ptr<Ty>;
        using mutex_t = std::recursive_mutex;

        class locked_ptr {
            friend class locking_shared_ptr;

        public:
            locked_ptr(const locked_ptr &) = delete;
            locked_ptr &operator =(const locked_ptr &) = delete;

            locked_ptr(locked_ptr &&) = default;
            locked_ptr &operator =(locked_ptr &&) = default;

            Ty *operator->() {
                return m_ptr.get();
            }

            const Ty *operator->() const {
                return m_ptr.get();
            }

            Ty &operator*() {
                return *m_ptr;
            }

            const Ty &operator*() const {
                return *m_ptr;
            }

            ///\note You can use it wrong. At your own risk of course.
            Ty *get() const {
                return m_ptr.get();
            }

        private:
            locked_ptr(mutex_t &mutex, ptr_t ptr)
                : m_lock(mutex)
                , m_ptr(ptr) {}

        private:
            std::unique_lock<mutex_t> m_lock;
            ptr_t m_ptr;
        };

    public:
        locking_shared_ptr()
            : m_mutex(make_mutex(use_own_lock_policy())) {}

        template <typename MutexPolicy = use_own_lock_policy>
        locking_shared_ptr(Ty *ptr, MutexPolicy policy = MutexPolicy())
            : m_ptr(ptr)
            , m_mutex(make_mutex(policy)) {}

        template <typename MutexPolicy = use_own_lock_policy>
        locking_shared_ptr(ptr_t ptr, MutexPolicy policy = MutexPolicy())
            : m_ptr(ptr)
            , m_mutex(make_mutex(policy)) {}

        template <typename MutexPolicy = use_own_lock_policy>
        locking_shared_ptr(Ty &&data, MutexPolicy policy = MutexPolicy())
            : m_ptr(std::make_shared<Ty>(std::move(data)))
            , m_mutex(make_mutex(policy)) {}

        template <typename MutexPolicy = use_own_lock_policy>
        locking_shared_ptr(const Ty &data, MutexPolicy policy = MutexPolicy())
            : m_ptr(std::make_shared<Ty>(data))
            , m_mutex(make_mutex(policy)) {}

        template <typename ...Args, typename MutexPolicy = use_own_lock_policy>
        locking_shared_ptr(Args &&...args, MutexPolicy policy = MutexPolicy())
            : m_ptr(std::make_shared<Ty>(std::forward<Args>(args)...))
            , m_mutex(make_mutex(policy)) {}

        locking_shared_ptr(const locking_shared_ptr &) = default;
        locking_shared_ptr &operator =(const locking_shared_ptr &) = default;

        locking_shared_ptr(locking_shared_ptr &&) = default;
        locking_shared_ptr &operator =(locking_shared_ptr &&) = default;

        locked_ptr lock() const {
            return { *m_mutex, m_ptr };
        }

        const Ty &unsafe_ref() const {
            return *m_ptr;
        }

        Ty &unsafe_ref() {
            return *m_ptr;
        }

        Ty *unsafe_ptr() const {
            return m_ptr.get();
        }

    private:
        template<typename OtherTy>
        static auto make_mutex(use_same_lock_policy<OtherTy> &other)
            -> std::shared_ptr<mutex_t> {
            return other.m_other.m_mutex;
        }

        static auto make_mutex(use_own_lock_policy)
            -> std::shared_ptr<mutex_t> {
            return std::make_shared<mutex_t>();
        }

        ptr_t m_ptr;
        std::shared_ptr<mutex_t> m_mutex;
    };


    template <typename Ty, typename ...Args>
    auto make_locking_shared(Args &&...args) -> locking_shared_ptr<Ty> {
        return locking_shared_ptr<Ty>(std::forward<Args>(args)...);
    }
} // namespace fcl
