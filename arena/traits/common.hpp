#pragma once

template <auto X> using integral_constant = std::integral_constant<decltype(X), X>;
