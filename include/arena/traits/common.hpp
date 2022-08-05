#pragma once

#include <type_traits>

namespace arena {

template <auto X> using integral_constant = std::integral_constant<decltype(X), X>;

} // namespace arena
