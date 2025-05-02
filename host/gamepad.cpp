#include "gamepad.h"

#include <stdexcept>

namespace remote_hid
{

gamepad::gamepad() : client_(vigem_alloc())
{
    // Allocate a ViGEm client
    if (!client_)
    {
        throw std::runtime_error("Failed to allocate ViGEm client");
    }

    // Connect to the ViGEm bus
    // This is an "expensive" operation, so it should be done once per application
    auto const err = vigem_connect(client_.get());
    if (!VIGEM_SUCCESS(err))
    {
        throw std::runtime_error("Failed to connect to ViGEm bus");
    }
}

gamepad::~gamepad()
{
    // Remove all targets
    for (auto const &target : targets_)
    {
        vigem_target_remove(client_.get(), target.get());
    }

    // Disconnect from the ViGEm bus
    vigem_disconnect(client_.get());
}

int gamepad::add_controller()
{
    // Allocate aa x360 target
    vigem_target target(vigem_target_x360_alloc());
    if (!target)
    {
        return -1;
    }

    // Register the target with the ViGEm client
    auto const error = vigem_target_add(client_.get(), target.get());
    if (!VIGEM_SUCCESS(error))
    {
        return -1;
    }

    // Store the target
    targets_.push_back(std::move(target));

    // return the index of the target
    return static_cast<int>(targets_.size() - 1);
}

}  // namespace remote_hid
