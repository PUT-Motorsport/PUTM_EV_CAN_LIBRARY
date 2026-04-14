#include "can_hal_ros2.hpp"

#ifdef PUTM_CAN_BACKEND_ROS2
    #include <rclcpp/rclcpp.hpp>
#else
    #include <iostream>
#endif

#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <cstring>

namespace putm_ev_can {

SocketCanHal::~SocketCanHal() {
    running_ = false;
    if (socket_fd_ >= 0) {
        shutdown(socket_fd_, SHUT_RDWR);
        close(socket_fd_); 
        socket_fd_ = -1;
    }
    if (rx_thread_.joinable()) {
        rx_thread_.join();
    }
}

bool SocketCanHal::init(const std::string& interface_name) {
    socket_fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd_ < 0) {
        #ifdef PUTM_CAN_BACKEND_ROS2
            RCLCPP_ERROR(rclcpp::get_logger("SocketCanHal"), "Failed to open socket");
        #else
            std::cerr << "Failed to open socket" << std::endl;
        #endif
        return false;
    }
    
    struct ifreq ifr;
    std::strncpy(ifr.ifr_name, interface_name.c_str(), IFNAMSIZ - 1);
    if (ioctl(socket_fd_, SIOCGIFINDEX, &ifr) < 0) return false;

    struct sockaddr_can addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socket_fd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) return false;

    connected_ = true;
    running_ = true;
    rx_thread_ = std::thread(&SocketCanHal::rx_thread_loop, this);
    return true;
}

bool SocketCanHal::transmit(const PUTM_CAN::CanFrame& frame) {
    if (!connected_) return false;

    struct can_frame linux_frame;
    linux_frame.can_id = frame.id;
    if (frame.id > 0x7FF) linux_frame.can_id |= CAN_EFF_FLAG;
    
    linux_frame.can_dlc = frame.dlc;
    std::memcpy(linux_frame.data, frame.data.data(), frame.dlc);

    int nbytes = write(socket_fd_, &linux_frame, sizeof(struct can_frame));
    if (nbytes == sizeof(struct can_frame)) {
        diagnostics_.tx_count++;
        return true;
    } else {
        diagnostics_.tx_errors++;
        return false;
    }
}

void SocketCanHal::rx_thread_loop() {
    while (running_) {
        if (socket_fd_ < 0) break;
        struct can_frame linux_frame;
        int nbytes = read(socket_fd_, &linux_frame, sizeof(struct can_frame));

        if (nbytes < 0) break;
        if (nbytes < (int)sizeof(struct can_frame)) continue;

        PUTM_CAN::CanFrame frame;
        frame.id = linux_frame.can_id & ~CAN_EFF_FLAG;
        frame.dlc = linux_frame.can_dlc;
        std::memcpy(frame.data.data(), linux_frame.data, 8); // Should handle FD here too if needed

        diagnostics_.rx_count++;
        if (rx_callback_) rx_callback_(frame);
    }
}

bool SocketCanHal::receive(PUTM_CAN::CanFrame& frame) { return false; }

} // namespace putm_ev_can