#pragma once

#define WIN32_LEAN_AND_MEAN
// ReSharper disable once CppUnusedIncludeDirective
#include <Windows.h>

#include <memory>
#include <vector>

#include <Vigem/Client.h>

namespace remote_hid
{

class gamepad
{
    struct vigem_deleter
    {
        void operator()(PVIGEM_CLIENT client) const;

        void operator()(PVIGEM_TARGET target) const;
    };

    using vigem_client = std::unique_ptr<std::remove_pointer_t<PVIGEM_CLIENT>, vigem_deleter>;
    using vigem_target = std::unique_ptr<std::remove_pointer_t<PVIGEM_TARGET>, vigem_deleter>;
    using vigem_target_vector = std::vector<vigem_target>;

public:
    gamepad();

    ~gamepad();

    gamepad(gamepad &&other) noexcept;

    gamepad &operator=(gamepad &&other) noexcept;

    gamepad(gamepad const &) = delete;

    void operator=(gamepad const &) = delete;

    int add_controller();

    void update_controller(int id, XUSB_REPORT const &report) const;

private:
    vigem_client        client_{};   ///< ViGEm client
    vigem_target_vector targets_{};  ///< ViGEm targets
};

inline void gamepad::vigem_deleter::operator()(PVIGEM_CLIENT client) const { vigem_free(client); }

inline void gamepad::vigem_deleter::operator()(PVIGEM_TARGET target) const
{
    vigem_target_free(target);
}

inline gamepad::gamepad(gamepad &&other) noexcept
    : client_(std::move(other.client_)), targets_(std::move(other.targets_))
{
}

inline gamepad &gamepad::operator=(gamepad &&other) noexcept
{
    if (this != &other)
    {
        client_  = std::move(other.client_);
        targets_ = std::move(other.targets_);
    }

    return *this;
}

inline void gamepad::update_controller(int id, XUSB_REPORT const &report) const
{
    auto const target = targets_.at(id).get();
    vigem_target_x360_update(client_.get(), target, report);
}

}  // namespace remote_hid
