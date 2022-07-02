#pragma once

#include <forward.hpp>

//! \brief Choose an overload for a non-member function, given the types of its input parameters
#define DISAMBIGUATE(F, ...)                                                                                           \
  []<typename... Args>(std::tuple<Args...> *) {                                                                        \
    return [](Args... args) -> decltype(auto) { return F(FWD(args)...); };                                             \
  }                                                                                                                    \
  ((std::tuple<__VA_ARGS__> *)nullptr)

//! \brief Choose an overload for a member function, given the types of its input parameters
#define DISAMBIGUATE_MEMBER(F, ...)                                                                                    \
  []<typename T, typename... Args>(std::tuple<T, Args...> *) {                                                         \
    return [](T x, Args... args) -> decltype(auto) { return FWD(x).F(FWD(args)...); };                                 \
  }                                                                                                                    \
  ((std::tuple<__VA_ARGS__> *)nullptr)

//! \brief Do nothing
//! \brief It is useful for defining write-only Python properties.
template <typename... Args> void noop(const Args &...){};
