/**
 * @file can_hal_ros2.cpp
 * @brief SocketCAN HAL (Linux / ROS2).
 *
 * Design notes:
 *  - One socket + one RX thread per bus. Callbacks of different buses run in
 *    DIFFERENT threads, concurrently.
 *  - RX thread uses poll() on the socket and an eventfd, so stop() wakes it
 *    immediately (close() alone does not wake a blocked read()).
 *  - Transient errors (EINTR, ENETDOWN during bus-off restart, ...) never
 *    terminate the RX thread.
 *  - transmit() retries on ENOBUFS (TX queue full) for tx_timeout_us.
 *  - Kernel-side RX drops are counted via SO_RXQ_OVFL -> diagnostics.rx_dropped.
 */

#include "can_hal_ros2.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <exception>

#include <fcntl.h>
#include <net/if.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <linux/can.h>
#include <linux/can/error.h>
#include <linux/can/netlink.h>
#include <linux/can/raw.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#if defined(PUTM_CAN_BACKEND_ROS2) && !defined(PUTM_CAN_NO_RCLCPP)
    #include <rclcpp/rclcpp.hpp>
    #define PUTM_LOG_ERROR(...) RCLCPP_ERROR(rclcpp::get_logger("putm_ev_can"), __VA_ARGS__)
    #define PUTM_LOG_WARN(...)  RCLCPP_WARN(rclcpp::get_logger("putm_ev_can"), __VA_ARGS__)
    #define PUTM_LOG_INFO(...)  RCLCPP_INFO(rclcpp::get_logger("putm_ev_can"), __VA_ARGS__)
#else
    #define PUTM_LOG_ERROR(...) (std::fprintf(stderr, "[putm_ev_can][ERROR] " __VA_ARGS__), std::fputc('\n', stderr))
    #define PUTM_LOG_WARN(...)  (std::fprintf(stderr, "[putm_ev_can][WARN] "  __VA_ARGS__), std::fputc('\n', stderr))
    #define PUTM_LOG_INFO(...)  (std::fprintf(stderr, "[putm_ev_can][INFO] "  __VA_ARGS__), std::fputc('\n', stderr))
#endif

namespace putm_ev_can {

// ===========================================================================
// Netlink helpers (read / set CAN bittiming, txqueuelen, up/down)
// ===========================================================================
namespace {

constexpr size_t kNlBufSize = 4096;

struct NlRequest {
    nlmsghdr  nh;
    ifinfomsg ifi;
    char      attrs[1024];
};

rtattr* nl_add_attr(nlmsghdr* n, size_t maxlen, unsigned short type, const void* data, size_t len) {
    const size_t rta_len = RTA_LENGTH(len);
    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(rta_len) > maxlen) return nullptr;
    auto* rta = reinterpret_cast<rtattr*>(reinterpret_cast<char*>(n) + NLMSG_ALIGN(n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len  = static_cast<unsigned short>(rta_len);
    if (len) std::memcpy(RTA_DATA(rta), data, len);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(rta_len);
    return rta;
}

void nl_end_nest(nlmsghdr* n, rtattr* nest) {
    nest->rta_len = static_cast<unsigned short>(
        reinterpret_cast<char*>(n) + NLMSG_ALIGN(n->nlmsg_len) - reinterpret_cast<char*>(nest));
}

void nl_init_request(NlRequest& req, uint16_t type, uint16_t flags, int if_index) {
    std::memset(&req, 0, sizeof(req));
    req.nh.nlmsg_len   = NLMSG_LENGTH(sizeof(ifinfomsg));
    req.nh.nlmsg_type  = type;
    req.nh.nlmsg_flags = NLM_F_REQUEST | flags;
    req.ifi.ifi_family = AF_UNSPEC;
    req.ifi.ifi_index  = if_index;
}

/// Sends request, returns 0 on success / -errno. If reply != nullptr, copies first RTM_NEWLINK reply.
int nl_transact(NlRequest& req, std::vector<char>* reply) {
    const int fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (fd < 0) return -errno;

    sockaddr_nl sa{};
    sa.nl_family = AF_NETLINK;
    static std::atomic<uint32_t> seq{1};
    req.nh.nlmsg_seq = seq++;

    if (sendto(fd, &req, req.nh.nlmsg_len, 0, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) < 0) {
        const int e = errno;
        close(fd);
        return -e;
    }

    timeval tv{1, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::vector<char> buf(kNlBufSize * 4);
    int result = -ETIMEDOUT;
    bool done = false;
    while (!done) {
        const ssize_t len = recv(fd, buf.data(), buf.size(), 0);
        if (len < 0) {
            if (errno == EINTR) continue;
            result = -errno;
            break;
        }
        int remaining = static_cast<int>(len);
        for (auto* nh = reinterpret_cast<nlmsghdr*>(buf.data()); NLMSG_OK(nh, remaining);
             nh = NLMSG_NEXT(nh, remaining)) {
            if (nh->nlmsg_seq != req.nh.nlmsg_seq) continue;
            if (nh->nlmsg_type == NLMSG_ERROR) {
                result = reinterpret_cast<nlmsgerr*>(NLMSG_DATA(nh))->error;  // 0 = ACK
                done = true;
                break;
            }
            if (nh->nlmsg_type == RTM_NEWLINK && reply) {
                reply->assign(reinterpret_cast<char*>(nh), reinterpret_cast<char*>(nh) + nh->nlmsg_len);
                result = 0;
                done = true;
                break;
            }
            if (nh->nlmsg_type == NLMSG_DONE) { done = true; break; }
        }
    }
    close(fd);
    return result;
}

struct CanLinkInfo {
    bool     ok = false;
    bool     is_can = false;     // false for vcan / slcan without bittiming
    bool     up = false;
    uint32_t bitrate = 0;
    uint32_t sample_point = 0;
    uint32_t data_bitrate = 0;
    uint32_t ctrlmode = 0;
    uint32_t restart_ms = 0;
    uint32_t txqlen = 0;
};

CanLinkInfo nl_query_link(int if_index) {
    CanLinkInfo info;
    NlRequest req;
    nl_init_request(req, RTM_GETLINK, 0, if_index);
    std::vector<char> reply;
    if (nl_transact(req, &reply) != 0 || reply.empty()) return info;
    info.ok = true;

    auto* nh  = reinterpret_cast<nlmsghdr*>(reply.data());
    auto* ifi = reinterpret_cast<ifinfomsg*>(NLMSG_DATA(nh));
    info.up = (ifi->ifi_flags & IFF_UP) != 0;

    int len = static_cast<int>(nh->nlmsg_len - NLMSG_LENGTH(sizeof(ifinfomsg)));
    for (auto* rta = IFLA_RTA(ifi); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        if (rta->rta_type == IFLA_TXQLEN) {
            std::memcpy(&info.txqlen, RTA_DATA(rta), sizeof(uint32_t));
        } else if (rta->rta_type == IFLA_LINKINFO) {
            int len2 = RTA_PAYLOAD(rta);
            for (auto* li = reinterpret_cast<rtattr*>(RTA_DATA(rta)); RTA_OK(li, len2); li = RTA_NEXT(li, len2)) {
                if (li->rta_type == IFLA_INFO_KIND) {
                    info.is_can = std::strncmp(static_cast<const char*>(RTA_DATA(li)), "can", RTA_PAYLOAD(li)) == 0;
                } else if (li->rta_type == IFLA_INFO_DATA) {
                    int len3 = RTA_PAYLOAD(li);
                    for (auto* c = reinterpret_cast<rtattr*>(RTA_DATA(li)); RTA_OK(c, len3); c = RTA_NEXT(c, len3)) {
                        switch (c->rta_type) {
                            case IFLA_CAN_BITTIMING: {
                                can_bittiming bt{};
                                std::memcpy(&bt, RTA_DATA(c), std::min<size_t>(sizeof(bt), RTA_PAYLOAD(c)));
                                info.bitrate = bt.bitrate;
                                info.sample_point = bt.sample_point;
                                break;
                            }
                            case IFLA_CAN_DATA_BITTIMING: {
                                can_bittiming bt{};
                                std::memcpy(&bt, RTA_DATA(c), std::min<size_t>(sizeof(bt), RTA_PAYLOAD(c)));
                                info.data_bitrate = bt.bitrate;
                                break;
                            }
                            case IFLA_CAN_CTRLMODE: {
                                can_ctrlmode cm{};
                                std::memcpy(&cm, RTA_DATA(c), std::min<size_t>(sizeof(cm), RTA_PAYLOAD(c)));
                                info.ctrlmode = cm.flags;
                                break;
                            }
                            case IFLA_CAN_RESTART_MS:
                                std::memcpy(&info.restart_ms, RTA_DATA(c), sizeof(uint32_t));
                                break;
                            default: break;
                        }
                    }
                }
            }
        }
    }
    return info;
}

int nl_set_up(int if_index, bool up) {
    NlRequest req;
    nl_init_request(req, RTM_NEWLINK, NLM_F_ACK, if_index);
    req.ifi.ifi_change = IFF_UP;
    req.ifi.ifi_flags  = up ? IFF_UP : 0;
    return nl_transact(req, nullptr);
}

int nl_set_txqlen(int if_index, uint32_t txqlen) {
    NlRequest req;
    nl_init_request(req, RTM_NEWLINK, NLM_F_ACK, if_index);
    nl_add_attr(&req.nh, sizeof(req), IFLA_TXQLEN, &txqlen, sizeof(txqlen));
    return nl_transact(req, nullptr);
}

int nl_set_can_params(int if_index, const SocketCanConfig& cfg, bool set_fd_mode) {
    NlRequest req;
    nl_init_request(req, RTM_NEWLINK, NLM_F_ACK, if_index);

    rtattr* linkinfo = nl_add_attr(&req.nh, sizeof(req), IFLA_LINKINFO, nullptr, 0);
    nl_add_attr(&req.nh, sizeof(req), IFLA_INFO_KIND, "can", 3);
    rtattr* data = nl_add_attr(&req.nh, sizeof(req), IFLA_INFO_DATA, nullptr, 0);

    if (cfg.bitrate) {
        can_bittiming bt{};
        bt.bitrate = cfg.bitrate;
        bt.sample_point = cfg.sample_point;
        nl_add_attr(&req.nh, sizeof(req), IFLA_CAN_BITTIMING, &bt, sizeof(bt));
    }
    if (set_fd_mode) {
        can_ctrlmode cm{};
        cm.mask  = CAN_CTRLMODE_FD;
        cm.flags = cfg.fd ? CAN_CTRLMODE_FD : 0;
        nl_add_attr(&req.nh, sizeof(req), IFLA_CAN_CTRLMODE, &cm, sizeof(cm));
    }
    if (cfg.fd && cfg.data_bitrate) {
        can_bittiming dbt{};
        dbt.bitrate = cfg.data_bitrate;
        dbt.sample_point = cfg.data_sample_point;
        nl_add_attr(&req.nh, sizeof(req), IFLA_CAN_DATA_BITTIMING, &dbt, sizeof(dbt));
    }
    uint32_t restart = cfg.restart_ms;
    nl_add_attr(&req.nh, sizeof(req), IFLA_CAN_RESTART_MS, &restart, sizeof(restart));

    nl_end_nest(&req.nh, data);
    nl_end_nest(&req.nh, linkinfo);
    return nl_transact(req, nullptr);
}

std::string ip_link_hint(const SocketCanConfig& c) {
    if (c.bitrate == 0) return "sudo ip link set " + c.interface + " up";
    std::string s = "sudo ip link set " + c.interface + " down && sudo ip link set " + c.interface +
                    " type can bitrate " + std::to_string(c.bitrate);
    if (c.sample_point) {
        char sp[16];
        std::snprintf(sp, sizeof(sp), "%.3f", c.sample_point / 1000.0);
        s += std::string(" sample-point ") + sp;
    }
    if (c.fd) {
        if (c.data_bitrate) s += " dbitrate " + std::to_string(c.data_bitrate);
        s += " fd on";
    }
    s += " restart-ms " + std::to_string(c.restart_ms);
    s += " && sudo ip link set " + c.interface + " txqueuelen " + std::to_string(c.txqueuelen ? c.txqueuelen : 1000) + " up";
    return s;
}

/// Pads a payload length to the next valid CAN FD length.
uint8_t fd_valid_len(uint8_t len) {
    static constexpr uint8_t kLens[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
    for (uint8_t l : kLens) if (len <= l) return l;
    return 64;
}

} // namespace

// ===========================================================================
// SocketCanHal
// ===========================================================================

SocketCanHal::~SocketCanHal() {
    stop();
    close_fds();
}

void SocketCanHal::close_fds() {
    if (socket_fd_ >= 0) { close(socket_fd_); socket_fd_ = -1; }
    if (wake_fd_ >= 0)   { close(wake_fd_);   wake_fd_ = -1; }
}

uint32_t SocketCanHal::read_interface_bitrate(const std::string& interface_name, uint32_t* data_bitrate) {
    const unsigned idx = if_nametoindex(interface_name.c_str());
    if (idx == 0) return 0;
    const CanLinkInfo info = nl_query_link(static_cast<int>(idx));
    if (data_bitrate) *data_bitrate = info.data_bitrate;
    return info.bitrate;
}

bool SocketCanHal::init(const std::string& interface_name) {
    SocketCanConfig cfg;
    cfg.interface = interface_name;
    if (!open(cfg)) return false;
    return init();
}

bool SocketCanHal::setup_interface() {
    const SocketCanConfig& c = config_;
    CanLinkInfo info = nl_query_link(if_index_);

    if (!info.ok) {
        PUTM_LOG_WARN("%s: cannot query interface via netlink - bitrate not verified", c.interface.c_str());
        return true;
    }

    if (!info.is_can) {
        if (c.bitrate) {
            PUTM_LOG_WARN("%s: not a hardware CAN interface (vcan?) - bitrate %u ignored",
                          c.interface.c_str(), c.bitrate);
        }
        if (!info.up) {
            PUTM_LOG_ERROR("%s: interface is DOWN (sudo ip link set %s up)", c.interface.c_str(), c.interface.c_str());
            return false;
        }
        return true;
    }

    const bool fd_now      = (info.ctrlmode & CAN_CTRLMODE_FD) != 0;
    const bool need_bt     = c.bitrate && (info.bitrate != c.bitrate ||
                                           (c.sample_point && info.sample_point != c.sample_point));
    const bool need_fd     = c.fd && !fd_now;
    const bool need_dbt    = c.fd && c.data_bitrate && info.data_bitrate != c.data_bitrate;
    const bool need_params = need_bt || need_fd || need_dbt;

    if (!c.configure_interface) {
        if (need_params) {
            PUTM_LOG_ERROR("%s: bitrate mismatch - interface: %u bit/s (data %u), expected: %u bit/s (data %u). "
                           "Run: %s   or set configure_interface = true",
                           c.interface.c_str(), info.bitrate, info.data_bitrate, c.bitrate, c.data_bitrate,
                           ip_link_hint(c).c_str());
            return false;
        }
        if (!info.up) {
            PUTM_LOG_ERROR("%s: interface is DOWN. Run: %s", c.interface.c_str(), ip_link_hint(c).c_str());
            return false;
        }
        if (info.txqlen < 100) {
            PUTM_LOG_WARN("%s: txqueuelen is %u - frames may be dropped under load (ENOBUFS). "
                          "Recommended: sudo ip link set %s txqueuelen 1000",
                          c.interface.c_str(), info.txqlen, c.interface.c_str());
        }
        if (info.restart_ms == 0) {
            PUTM_LOG_WARN("%s: restart-ms is 0 - after bus-off the bus stays dead until manual restart",
                          c.interface.c_str());
        }
        return true;
    }

    // --- configure_interface = true ---
    const bool need_restart = info.restart_ms != c.restart_ms;
    const bool need_txq     = c.txqueuelen && info.txqlen != c.txqueuelen;

    if (need_params || need_restart) {
        int err;
        if (info.up && (err = nl_set_up(if_index_, false)) != 0) {
            PUTM_LOG_ERROR("%s: cannot set interface down: %s (needs CAP_NET_ADMIN). Run manually: %s",
                           c.interface.c_str(), std::strerror(-err), ip_link_hint(c).c_str());
            return false;
        }
        if ((err = nl_set_can_params(if_index_, c, need_fd)) != 0) {
            PUTM_LOG_ERROR("%s: cannot set bitrate %u: %s", c.interface.c_str(), c.bitrate, std::strerror(-err));
            nl_set_up(if_index_, true);
            return false;
        }
        info.up = false;
    }
    if (need_txq) {
        if (const int err = nl_set_txqlen(if_index_, c.txqueuelen); err != 0) {
            PUTM_LOG_WARN("%s: cannot set txqueuelen %u: %s", c.interface.c_str(), c.txqueuelen, std::strerror(-err));
        }
    }
    if (!info.up) {
        if (const int err = nl_set_up(if_index_, true); err != 0) {
            PUTM_LOG_ERROR("%s: cannot set interface up: %s", c.interface.c_str(), std::strerror(-err));
            return false;
        }
    }

    const CanLinkInfo now = nl_query_link(if_index_);
    PUTM_LOG_INFO("%s: configured - bitrate %u bit/s, sample point %.1f%%, data bitrate %u, restart-ms %u, txqueuelen %u",
                  c.interface.c_str(), now.bitrate, now.sample_point / 10.0, now.data_bitrate, now.restart_ms, now.txqlen);
    return true;
}

bool SocketCanHal::open(const SocketCanConfig& config) {
    stop();
    close_fds();
    config_ = config;

    if (config_.interface.empty() || config_.interface.size() >= IFNAMSIZ) {
        PUTM_LOG_ERROR("Invalid CAN interface name '%s'", config_.interface.c_str());
        return false;
    }

    if_index_ = static_cast<int>(if_nametoindex(config_.interface.c_str()));
    if (if_index_ == 0) {
        PUTM_LOG_ERROR("%s: interface not found (%s)", config_.interface.c_str(), std::strerror(errno));
        return false;
    }

    if (!setup_interface()) return false;

    socket_fd_ = socket(PF_CAN, SOCK_RAW | SOCK_CLOEXEC, CAN_RAW);
    if (socket_fd_ < 0) {
        PUTM_LOG_ERROR("%s: failed to open CAN socket: %s", config_.interface.c_str(), std::strerror(errno));
        return false;
    }

    // --- CAN FD ---
    fd_enabled_ = false;
    if (config_.fd) {
        ifreq ifr{};
        std::strncpy(ifr.ifr_name, config_.interface.c_str(), IFNAMSIZ - 1);
        const int on = 1;
        if (ioctl(socket_fd_, SIOCGIFMTU, &ifr) < 0 || ifr.ifr_mtu != CANFD_MTU) {
            PUTM_LOG_ERROR("%s: CAN FD requested but interface is not in FD mode (MTU != 72)", config_.interface.c_str());
            close_fds();
            return false;
        }
        if (setsockopt(socket_fd_, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &on, sizeof(on)) < 0) {
            PUTM_LOG_ERROR("%s: cannot enable CAN FD frames: %s", config_.interface.c_str(), std::strerror(errno));
            close_fds();
            return false;
        }
        fd_enabled_ = true;
    }

    // --- Error frames: bus-off, controller problems, restarts ---
    const can_err_mask_t err_mask = CAN_ERR_BUSOFF | CAN_ERR_CRTL | CAN_ERR_RESTARTED | CAN_ERR_TX_TIMEOUT;
    setsockopt(socket_fd_, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));

    // --- Count kernel RX drops ---
    const int one = 1;
    setsockopt(socket_fd_, SOL_SOCKET, SO_RXQ_OVFL, &one, sizeof(one));

    // --- Bigger RX buffer (FORCE needs CAP_NET_ADMIN, fallback is capped by net.core.rmem_max) ---
    if (config_.rx_buffer_bytes > 0) {
        const int sz = config_.rx_buffer_bytes;
        if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz)) < 0) {
            setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF, &sz, sizeof(sz));
        }
    }

    sockaddr_can addr{};
    addr.can_family  = AF_CAN;
    addr.can_ifindex = if_index_;
    if (bind(socket_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        PUTM_LOG_ERROR("%s: bind failed: %s", config_.interface.c_str(), std::strerror(errno));
        close_fds();
        return false;
    }

    wake_fd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (wake_fd_ < 0) {
        PUTM_LOG_ERROR("eventfd failed: %s", std::strerror(errno));
        close_fds();
        return false;
    }
    return true;
}

bool SocketCanHal::configure_filters(std::span<const PUTM_CAN::CanFilter> filters) {
    if (socket_fd_ < 0) return false;
    if (filters.empty()) return true;  // keep kernel default: accept all

    std::vector<can_filter> kf;
    kf.reserve(filters.size());
    for (const auto& f : filters) {
        can_filter cf{};
        if (f.mask == 0) {
            cf.can_id = 0;
            cf.can_mask = 0;  // accept everything (std + ext)
        } else {
            cf.can_id   = f.extended ? ((f.id & CAN_EFF_MASK) | CAN_EFF_FLAG) : (f.id & CAN_SFF_MASK);
            cf.can_mask = (f.extended ? (f.mask & CAN_EFF_MASK) : (f.mask & CAN_SFF_MASK)) | CAN_EFF_FLAG | CAN_RTR_FLAG;
        }
        kf.push_back(cf);
    }
    if (setsockopt(socket_fd_, SOL_CAN_RAW, CAN_RAW_FILTER, kf.data(),
                   static_cast<socklen_t>(kf.size() * sizeof(can_filter))) < 0) {
        PUTM_LOG_ERROR("%s: CAN_RAW_FILTER failed: %s", config_.interface.c_str(), std::strerror(errno));
        return false;
    }
    return true;
}

void SocketCanHal::set_rx_callback(PUTM_CAN::RxCallback callback) {
    if (running_) {
        // Swapping a std::function that another thread is calling is a data race.
        PUTM_LOG_WARN("%s: set_rx_callback() while RX thread is running - stopping RX first",
                      config_.interface.c_str());
        stop();
        rx_callback_ = std::move(callback);
        if (rx_callback_) init();
        return;
    }
    rx_callback_ = std::move(callback);
}

bool SocketCanHal::init() {
    if (socket_fd_ < 0 || wake_fd_ < 0) return false;
    if (running_) return true;
    running_ = true;
    rx_thread_ = std::thread(&SocketCanHal::rx_thread_loop, this);
    return true;
}

void SocketCanHal::stop() {
    if (!running_ && !rx_thread_.joinable()) return;
    running_ = false;
    if (wake_fd_ >= 0) {
        const uint64_t v = 1;
        [[maybe_unused]] ssize_t r = write(wake_fd_, &v, sizeof(v));
    }
    if (rx_thread_.joinable() && rx_thread_.get_id() != std::this_thread::get_id()) {
        rx_thread_.join();
    } else if (rx_thread_.joinable()) {
        rx_thread_.detach();  // stop() called from inside a callback
    }
    if (wake_fd_ >= 0) {  // drain
        uint64_t v;
        [[maybe_unused]] ssize_t r = read(wake_fd_, &v, sizeof(v));
    }
}

bool SocketCanHal::transmit(const PUTM_CAN::CanFrame& frame) {
    if (socket_fd_ < 0) return false;

    canfd_frame cf{};
    if (frame.id > CAN_SFF_MASK) {
        cf.can_id = (frame.id & CAN_EFF_MASK) | CAN_EFF_FLAG;
    } else {
        cf.can_id = frame.id;
    }

    size_t mtu;
    const uint8_t dlc = std::min<uint8_t>(frame.dlc, 64);
    if (frame.is_fd || dlc > 8) {
        if (!fd_enabled_) {
            diag_.tx_errors++;
            diag_.last_error_code = EINVAL;
            return false;  // never copy >8 bytes into a classic frame
        }
        cf.len   = fd_valid_len(dlc);
        cf.flags = config_.data_bitrate ? CANFD_BRS : 0;
        mtu = CANFD_MTU;
    } else {
        cf.len = dlc;
        mtu = CAN_MTU;
    }
    std::memcpy(cf.data, frame.data.data(), dlc);

    using clock = std::chrono::steady_clock;
    const auto deadline = clock::now() + std::chrono::microseconds(config_.tx_timeout_us);

    while (true) {
        const ssize_t n = write(socket_fd_, &cf, mtu);
        if (n == static_cast<ssize_t>(mtu)) {
            diag_.tx_count++;
            return true;
        }
        const int err = (n < 0) ? errno : EIO;
        if (err == EINTR) continue;
        if (err == ENOBUFS || err == EAGAIN) {
            diag_.tx_buffer_full++;
            if (clock::now() < deadline) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                continue;
            }
        }
        diag_.tx_errors++;
        diag_.last_error_code = static_cast<uint32_t>(err);
        return false;
    }
}

void SocketCanHal::handle_error_frame(uint32_t can_id, const uint8_t* data) {
    diag_.last_error_code = can_id & CAN_ERR_MASK;
    if (can_id & CAN_ERR_BUSOFF) {
        diag_.bus_off_count++;
        PUTM_LOG_ERROR("%s: BUS-OFF%s", config_.interface.c_str(),
                       config_.restart_ms ? "" : " (restart-ms = 0, bus will not recover automatically!)");
    }
    if (can_id & CAN_ERR_RESTARTED) {
        PUTM_LOG_WARN("%s: controller restarted after bus-off", config_.interface.c_str());
    }
    if (can_id & CAN_ERR_CRTL) {
        if (data[1] & (CAN_ERR_CRTL_RX_OVERFLOW)) {
            diag_.rx_dropped++;  // controller (hardware) FIFO overflow
        }
        if (data[1] & (CAN_ERR_CRTL_RX_PASSIVE | CAN_ERR_CRTL_TX_PASSIVE)) {
            PUTM_LOG_WARN("%s: controller error-passive", config_.interface.c_str());
        }
    }
    if (can_id & CAN_ERR_TX_TIMEOUT) {
        diag_.tx_errors++;
    }
}

void SocketCanHal::rx_thread_loop() {
    pollfd fds[2]{};
    fds[0].fd = socket_fd_;
    fds[0].events = POLLIN;
    fds[1].fd = wake_fd_;
    fds[1].events = POLLIN;

    bool net_down_logged = false;
    alignas(cmsghdr) char ctrl[CMSG_SPACE(sizeof(uint32_t))];

    while (running_) {
        const int pr = poll(fds, 2, -1);
        if (pr < 0) {
            if (errno == EINTR) continue;
            diag_.rx_errors++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        if (fds[1].revents) break;   // stop() requested
        if (!(fds[0].revents & (POLLIN | POLLERR))) continue;

        // Drain everything that is queued
        for (int burst = 0; burst < 256 && running_; ++burst) {
            canfd_frame cf{};
            iovec iov{&cf, sizeof(cf)};
            msghdr msg{};
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_control = ctrl;
            msg.msg_controllen = sizeof(ctrl);

            const ssize_t n = recvmsg(socket_fd_, &msg, MSG_DONTWAIT);
            if (n < 0) {
                const int err = errno;
                if (err == EAGAIN || err == EWOULDBLOCK) break;
                if (err == EINTR) continue;
                // ENETDOWN (interface down / bus-off restart), ENOBUFS, ... -> never kill the thread
                diag_.rx_errors++;
                diag_.last_error_code = static_cast<uint32_t>(err);
                if (!net_down_logged) {
                    PUTM_LOG_WARN("%s: receive error: %s (RX keeps running)", config_.interface.c_str(), std::strerror(err));
                    net_down_logged = true;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(err == ENETDOWN ? 50 : 1));
                break;
            }
            net_down_logged = false;

            // Kernel drop counter (cumulative per socket)
            for (cmsghdr* c = CMSG_FIRSTHDR(&msg); c; c = CMSG_NXTHDR(&msg, c)) {
                if (c->cmsg_level == SOL_SOCKET && c->cmsg_type == SO_RXQ_OVFL) {
                    uint32_t drops;
                    std::memcpy(&drops, CMSG_DATA(c), sizeof(drops));
                    if (drops != last_kernel_drops_) {
                        diag_.rx_dropped += drops - last_kernel_drops_;
                        last_kernel_drops_ = drops;
                    }
                }
            }

            bool is_fd;
            if (n == CAN_MTU)        is_fd = false;
            else if (n == CANFD_MTU) is_fd = true;
            else { diag_.rx_errors++; continue; }

            if (cf.can_id & CAN_ERR_FLAG) { handle_error_frame(cf.can_id, cf.data); continue; }
            if (cf.can_id & CAN_RTR_FLAG) continue;

            PUTM_CAN::CanFrame frame;
            frame.id    = (cf.can_id & CAN_EFF_FLAG) ? (cf.can_id & CAN_EFF_MASK) : (cf.can_id & CAN_SFF_MASK);
            frame.dlc   = std::min<uint8_t>(cf.len, is_fd ? 64 : 8);
            frame.is_fd = is_fd;
            std::memcpy(frame.data.data(), cf.data, frame.dlc);

            diag_.rx_count++;
            if (rx_callback_) {
                try {
                    rx_callback_(frame);
                } catch (const std::exception& e) {
                    diag_.rx_errors++;
                    PUTM_LOG_ERROR("%s: exception in RX callback (id 0x%X): %s", config_.interface.c_str(), frame.id, e.what());
                } catch (...) {
                    diag_.rx_errors++;
                    PUTM_LOG_ERROR("%s: unknown exception in RX callback (id 0x%X)", config_.interface.c_str(), frame.id);
                }
            }
        }
    }
}

PUTM_CAN::BusDiagnostics SocketCanHal::get_diagnostics() const {
    PUTM_CAN::BusDiagnostics d;
    d.rx_count        = diag_.rx_count.load();
    d.tx_count        = diag_.tx_count.load();
    d.tx_errors       = diag_.tx_errors.load();
    d.rx_errors       = diag_.rx_errors.load();
    d.last_error_code = diag_.last_error_code.load();
    d.rx_dropped      = diag_.rx_dropped.load();
    d.tx_buffer_full  = diag_.tx_buffer_full.load();
    d.bus_off_count   = diag_.bus_off_count.load();
    return d;
}

} // namespace putm_ev_can
