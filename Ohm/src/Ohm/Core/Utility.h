#pragma once

#include "Ohm/Core/Log.h"

#define OHM_BIND_FN(fn) [this](auto&& ... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
