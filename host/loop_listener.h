#pragma once

#include <boost/intrusive/list_hook.hpp>

namespace remote_hid
{

class loop_listener
{
    friend class event_dispatch;

    using list_hook_t = boost::intrusive::list_member_hook<>;

public:
    loop_listener() = default;
    virtual ~loop_listener() = default;

    loop_listener(const loop_listener&) = delete;
    void operator=(const loop_listener&) = delete;
    loop_listener(loop_listener&&) = delete;
    void operator=(loop_listener&&) = delete;

protected:
    virtual void on_loop() = 0;

private:
    list_hook_t list_hook_{};
};

}