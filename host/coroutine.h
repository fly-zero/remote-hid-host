#pragma once

#include <boost/coroutine2/coroutine.hpp>

namespace remote_hid
{

class coroutine_base
{
    using co_push = boost::coroutines2::coroutine<void>::pull_type;
    using co_pull = boost::coroutines2::coroutine<void>::push_type;

public:
    coroutine_base()          = default;
    virtual ~coroutine_base() = default;

    coroutine_base(const coroutine_base &) = delete;
    void operator=(const coroutine_base &) = delete;

    coroutine_base(coroutine_base &&other) noexcept;

    coroutine_base &operator=(coroutine_base &&other) noexcept;

private:
    void run(co_push &source);

protected:
    virtual void run() = 0;

    void yield() const;

    void resume();

private:
    co_pull  sink_{[this](co_push &source) { run(source); }};  ///< coroutine sink
    co_push *source_{nullptr};                                 ///< coroutine source
};

inline coroutine_base::coroutine_base(coroutine_base &&other) noexcept
    : sink_{std::move(other.sink_)}, source_{std::exchange(other.source_, nullptr)}
{
}

inline coroutine_base &coroutine_base::operator=(coroutine_base &&other) noexcept
{
    if (this != &other)
    {
        sink_   = std::move(other.sink_);
        source_ = std::exchange(other.source_, nullptr);
    }

    return *this;
}

inline void coroutine_base::run(co_push &source)
{
    source_ = &source;
    run();
}

inline void coroutine_base::yield() const { (*source_)(); }

inline void coroutine_base::resume() { sink_(); }

}  // namespace remote_hid
