#ifndef PUTM_EV_CAN_HAL_ROS2_HPP
#define PUTM_EV_CAN_HAL_ROS2_HPP

#include "can_hal.hpp"
#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

namespace putm_ev_can {

/**
 * @brief Configuration of one SocketCAN bus.
 *
 * Minimal usage (only verifies the interface bitrate, no root needed):
 * @code
 *   SocketCanConfig cfg;
 *   cfg.interface = "can0";
 *   cfg.bitrate   = 1'000'000;
 *   driver.Init(cfg);
 * @endcode
 *
 * With @ref configure_interface = true the library sets the bitrate itself
 * (interface down -> bittiming -> txqueuelen -> up). That needs CAP_NET_ADMIN:
 *   sudo setcap cap_net_admin+ep <your_node_binary>
 */
struct SocketCanConfig {
    std::string interface = "can0";

    /// Nominal (arbitration) bitrate in bit/s. 0 = do not check / do not set.
    uint32_t bitrate = 0;

    /// Sample point in tenths of percent (875 = 87.5 %). 0 = kernel default.
    uint16_t sample_point = 0;

    /// CAN FD: enables FD frames on the socket (and FD ctrlmode when configuring).
    bool fd = false;
    /// CAN FD data-phase bitrate in bit/s. 0 = no bitrate switch (BRS off).
    uint32_t data_bitrate = 0;
    /// CAN FD data-phase sample point (tenths of percent). 0 = kernel default.
    uint16_t data_sample_point = 0;

    /// true  -> set bitrate / restart_ms / txqueuelen via netlink if they differ (needs CAP_NET_ADMIN)
    /// false -> only verify; Init() fails on bitrate mismatch and logs the `ip link` command to run
    bool configure_interface = false;

    /// Automatic restart after bus-off in ms (applied only when configure_interface = true). 0 = off.
    uint32_t restart_ms = 100;

    /// Interface TX queue length (applied only when configure_interface = true).
    /// Kernel default for CAN is 10, which overflows easily (ENOBUFS). 0 = leave unchanged.
    uint32_t txqueuelen = 1000;

    /// Socket receive buffer in bytes. 0 = system default.
    int rx_buffer_bytes = 1 << 20;

    /// How long transmit() keeps retrying when the TX queue is full (ENOBUFS). 0 = no retry.
    uint32_t tx_timeout_us = 5000;
};

class SocketCanHal : public PUTM_CAN::ICanHal {
public:
    SocketCanHal() = default;
    ~SocketCanHal() override;

    SocketCanHal(const SocketCanHal&) = delete;
    SocketCanHal& operator=(const SocketCanHal&) = delete;

    /// Opens and configures the socket. Does NOT start reception yet (see init()).
    bool open(const SocketCanConfig& config);
    /// Backwards compatible: open with default config on given interface.
    bool init(const std::string& interface_name);

    /// Starts the RX thread. Call after set_rx_callback()/configure_filters().
    bool init() override;
    void stop() override;

    bool transmit(const PUTM_CAN::CanFrame& frame) override;
    bool receive(PUTM_CAN::CanFrame& frame) override { (void)frame; return false; }
    bool configure_filters(std::span<const PUTM_CAN::CanFilter> filters) override;

    /// Must be called before init() (the RX thread reads it without locking).
    void set_rx_callback(PUTM_CAN::RxCallback callback) override;

    bool is_initialized() const { return socket_fd_ >= 0; }
    const SocketCanConfig& config() const { return config_; }

    PUTM_CAN::BusDiagnostics get_diagnostics() const override;

    /// Reads the current nominal bitrate of an interface via netlink. Returns 0 if unknown (e.g. vcan).
    static uint32_t read_interface_bitrate(const std::string& interface_name, uint32_t* data_bitrate = nullptr);

private:
    struct AtomicDiagnostics {
        std::atomic<uint32_t> rx_count{0};
        std::atomic<uint32_t> tx_count{0};
        std::atomic<uint32_t> tx_errors{0};
        std::atomic<uint32_t> rx_errors{0};
        std::atomic<uint32_t> last_error_code{0};
        std::atomic<uint32_t> rx_dropped{0};
        std::atomic<uint32_t> tx_buffer_full{0};
        std::atomic<uint32_t> bus_off_count{0};
    };

    SocketCanConfig config_;
    int socket_fd_ = -1;
    int wake_fd_ = -1;   // eventfd used to wake the RX thread on stop()
    int if_index_ = 0;
    bool fd_enabled_ = false;

    PUTM_CAN::RxCallback rx_callback_ = nullptr;
    AtomicDiagnostics diag_;
    uint32_t last_kernel_drops_ = 0;   // only touched by the RX thread

    std::atomic<bool> running_{false};
    std::thread rx_thread_;

    bool setup_interface();
    void close_fds();
    void rx_thread_loop();
    void handle_error_frame(uint32_t can_id, const uint8_t* data);
};

} // namespace putm_ev_can

#endif
