#pragma once
#include <memory>
#include <mutex>


namespace fcl {
    template <typename Ty>
    class locking_shared_ptr {
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
        locking_shared_ptr(Ty *ptr)
            : m_ptr(ptr) {}

        locking_shared_ptr(ptr_t ptr)
            : m_ptr(ptr) {}

        locking_shared_ptr(Ty &&data)
            : m_ptr(std::make_shared<Ty>(std::move(data))) {}

        locking_shared_ptr(const Ty &data)
            : m_ptr(std::make_shared<Ty>(data)) {}

        template <typename ...Args>
        locking_shared_ptr(Args &&...args)
            : m_ptr(std::make_shared<Ty>(std::forward(args)...)) {}

        locked_ptr lock() {
            return { *m_mutex, m_ptr };
        }

    private:
        ptr_t m_ptr;
        std::shared_ptr<mutex_t> m_mutex = std::make_shared<mutex_t>();
    };


    template <typename Ty, typename ...Args>
    auto make_locking_shared(Args &&...args) -> locking_shared_ptr<Ty> {
        return locking_shared_ptr<Ty>(std::forward(args)...);
    }
} // namespace fcl
