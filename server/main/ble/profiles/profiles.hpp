#pragma once
// std
#include <variant>
namespace ble
{
class CWhoAmI;
class CWhereAmI;

using Profile = std::variant<CWhoAmI, CWhereAmI>;

}    // namespace ble
// profile headers must be included last

#include "CWhoAmI.hpp"
#include "CWhereAmI.hpp"
