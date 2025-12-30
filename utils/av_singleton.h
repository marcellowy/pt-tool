#ifndef AV_SINGLETON_H_
#define AV_SINGLETON_H_

#include <memory>
#include <mutex>

namespace av {
    template <typename T>
    class Singleton {
    public:
        static T& instance() {
            if (!m_instance) {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (!m_instance) {
                    m_instance.reset(new T());
                }
            }
            return *m_instance;
        }

        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

    protected:
        Singleton() = default;
        virtual ~Singleton() = default;

    private:
        static std::unique_ptr<T> m_instance;
        static std::mutex m_mutex;
    };

    template <typename T>
    std::unique_ptr<T> Singleton<T>::m_instance = nullptr;

    template <typename T>
    std::mutex Singleton<T>::m_mutex;
}
#endif
