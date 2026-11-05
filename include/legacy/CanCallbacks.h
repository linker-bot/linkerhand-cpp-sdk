#pragma once

#include <linkerbot/can/dispatcher.hpp>
#include <linkerbot/can/frame.hpp>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <string>

namespace linkerhand::legacy {
// A single-frame callback bridge. The caller owns the physical dispatcher and
// must retain this adapter until LinkerHandApi has stopped invoking callbacks.
// It does not infer transaction boundaries, device identity or reply IDs.
class CanCallbacks {
    struct Inbox {
        std::mutex mutex;
        std::condition_variable ready;
        std::deque<linkerbot::can::Message> frames;
        bool closed = false;
        size_t overflow = 0;
        std::string error;
    };
public:
    CanCallbacks(std::shared_ptr<linkerbot::can::CANDispatcher> dispatcher,
                 bool fd, bool extended, size_t capacity = 512)
        : dispatcher_(std::move(dispatcher)), inbox_(std::make_shared<Inbox>()),
          fd_(fd), extended_(extended) {
        if (!dispatcher_ || capacity == 0) throw std::invalid_argument("dispatcher and capacity are required");
        auto inbox = inbox_;
        subscription_ = dispatcher_->subscribe([inbox, capacity, fd, extended](const auto& frame) {
            // Preserve legacy protocol filtering in the prebuilt SDK. Frame type
            // is filtered here; payload-specific correlation remains above us.
            if (frame.is_fd != fd || frame.is_extended != extended) return;
            linkerbot::can::validate_arbitration_id(frame.id, frame.is_extended);
            linkerbot::can::validate_frame_flags(frame.is_fd, frame.is_brs);
            linkerbot::can::validate_payload_length(frame.dlc,
                fd ? linkerbot::can::LinkKind::kCanFd : linkerbot::can::LinkKind::kClassic);
            std::lock_guard<std::mutex> lock(inbox->mutex);
            if (inbox->closed) return;
            if (inbox->frames.size() == capacity) {
                ++inbox->overflow;
                inbox->error = "legacy receive queue overflow";
                return;
            }
            inbox->frames.push_back(frame);
            inbox->ready.notify_one();
        });
    }
    ~CanCallbacks() { close(); }
    CanCallbacks(const CanCallbacks&) = delete;
    CanCallbacks& operator=(const CanCallbacks&) = delete;

    // Return 0 only after synchronous backend acceptance, not a device ACK.
    int32_t transmit(uint32_t id, const uint8_t* data, uintptr_t size) noexcept {
        try {
            if (size > (fd_ ? 64u : 8u) || (size && !data))
                throw std::invalid_argument("invalid legacy transmit payload");
            {
                std::lock_guard<std::mutex> lock(inbox_->mutex);
                if (inbox_->closed) return -1;
            }
            linkerbot::can::Message frame;
            linkerbot::can::validate_arbitration_id(id, extended_);
            frame.id = id;
            frame.dlc = static_cast<uint8_t>(size);
            frame.is_fd = fd_;
            frame.is_extended = extended_;
            // BRS is opt-in; no legacy callback field requests it.
            if (size) std::memcpy(frame.data.data(), data, size);
            dispatcher_->send(frame);
            return 0;
        } catch (const std::exception& error) {
            record_error(error.what());
        } catch (...) {
            record_error("unknown legacy transmit error");
        }
        return -1;
    }

    // RX has no capacity argument in the old ABI. The SDK must supply 8 bytes
    // for Classic or 64 bytes for FD. Length is a byte count, never a wire DLC.
    int32_t receive(uint32_t* id, uint8_t* data, uint8_t* size,
                    std::chrono::milliseconds timeout = std::chrono::milliseconds{10}) noexcept {
        try {
            if (!id || !data || !size || timeout.count() < 0) return -1;
            std::unique_lock<std::mutex> lock(inbox_->mutex);
            if (!inbox_->ready.wait_for(lock, timeout, [&] { return inbox_->closed || !inbox_->frames.empty(); }) || inbox_->closed)
                return -1;
            const auto frame = inbox_->frames.front();
            inbox_->frames.pop_front();
            *id = frame.id;
            *size = frame.dlc;
            std::memcpy(data, frame.data.data(), frame.dlc);
            return 0;
        } catch (const std::exception& error) {
            record_error(error.what());
        } catch (...) {
            record_error("unknown legacy receive error");
        }
        return -1;
    }
    void close() noexcept {
        {
            std::lock_guard<std::mutex> lock(inbox_->mutex);
            inbox_->closed = true;
            inbox_->frames.clear();
            inbox_->ready.notify_all();
        }
        // Removing one adapter does not stop a dispatcher shared by others.
        try { (void)dispatcher_->unsubscribe(subscription_); } catch (...) {}
    }
    size_t overflows() const {
        std::lock_guard<std::mutex> lock(inbox_->mutex);
        return inbox_->overflow;
    }
    std::string last_error() const {
        std::lock_guard<std::mutex> lock(inbox_->mutex);
        return inbox_->error;
    }
private:
    void record_error(const char* error) noexcept {
        try {
            std::lock_guard<std::mutex> lock(inbox_->mutex);
            inbox_->error = error;
        } catch (...) {}
    }
    std::shared_ptr<linkerbot::can::CANDispatcher> dispatcher_;
    std::shared_ptr<Inbox> inbox_;
    linkerbot::can::SubscriptionId subscription_{};
    bool fd_;
    bool extended_;
};
} // namespace linkerhand::legacy
