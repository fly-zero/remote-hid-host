#pragma once

#include <sal.h>

#include <stdexcept>

namespace remote_hid
{
namespace exception
{

std::runtime_error runtime_error(_In_z_ _Printf_format_string_ const char *format, ...);

std::invalid_argument invalid_argument(_In_z_ _Printf_format_string_ const char *format, ...);

}  // namespace exception
}  // namespace remote_hid