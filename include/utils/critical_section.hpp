/**
 * @file critical_section.hpp
 * @brief RAII wrapper for thread/interrupt safety.
 * @details Handles mutexes on ROS2 and interrupt disabling on STM32.
 */

#ifndef PUTM_EV_CAN_CRITICAL_SECTION_HPP
#define PUTM_EV_CAN_CRITICAL_SECTION_HPP

#if defined(PUTM_CAN_BACKEND_ROS2)
    #include <mutex>
#elif defined(PUTM_CAN_BACKEND_STM32)
    #include "stm32_hal_selector.hpp"
#endif

namespace putm_ev_can {

class CriticalSection {
public:
    void lock() {
#if defined(PUTM_CAN_BACKEND_ROS2)
        mutex_.lock();
#elif defined(PUTM_CAN_BACKEND_STM32)
        primask_ = __get_PRIMASK();
        __disable_irq();
#endif
    }

    void unlock() {
#if defined(PUTM_CAN_BACKEND_ROS2)
        mutex_.unlock();
#elif defined(PUTM_CAN_BACKEND_STM32)
        __set_PRIMASK(primask_);
#endif
    }

private:
#if defined(PUTM_CAN_BACKEND_ROS2)
    std::mutex mutex_;
#elif defined(PUTM_CAN_BACKEND_STM32)
    uint32_t primask_ = 0;
#endif
};

/**
 * @brief RAII Guard. Locks on creation, unlocks on destruction.
 */
class LockGuard {
public:
    explicit LockGuard(CriticalSection& cs) : cs_(cs) { cs_.lock(); }
    ~LockGuard() { cs_.unlock(); }
    
    // Disable copy/move to prevent logic errors
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

private:
    CriticalSection& cs_;
};

} // namespace putm_ev_can

#endif