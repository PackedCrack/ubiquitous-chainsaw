#pragma once
// std
#include <variant>

namespace ble
{
class CWhoAmI;
using Profile = std::variant<CWhoAmI>;
}	// namespace ble
// profile headers must be included last
#include "CWhoAmI.hpp"