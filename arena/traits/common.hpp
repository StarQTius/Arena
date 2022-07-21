#pragma once

#include <type_traits>

template <auto X> using integral_constant = std::integral_constant<decltype(X), X>;
