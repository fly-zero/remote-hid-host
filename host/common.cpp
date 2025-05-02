#include "common.h"

#include <cassert>
#include <cstdarg>

namespace remote_hid
{
namespace exception
{

inline const char *construct_error_message(const char *format, va_list args)
{
    static char buffer[1024];
    auto const  ret = vsnprintf(
        buffer, sizeof(buffer), format, args);  // NOLINT(clang-diagnostic-format-nonliteral)
    assert(0 <= ret && static_cast<uint64_t>(ret) < sizeof buffer);
    return buffer;
}

std::runtime_error runtime_error(_In_z_ _Printf_format_string_ const char *format, ...)
{
    va_list args;
    va_start(args, format);
    auto const msg = construct_error_message(format, args);
    va_end(args);
    return std::runtime_error(msg);
}

std::invalid_argument invalid_argument(_In_z_ _Printf_format_string_ const char *format, ...)
{
    va_list args;
    va_start(args, format);
    auto const msg = construct_error_message(format, args);
    va_end(args);
    return std::invalid_argument(msg);
}

}  // namespace exception
}  // namespace remote_hid
