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

template <typename> struct get_class;

template <typename F, typename C> struct get_class<F C::*> { using type = C; };

template <typename Mf>
requires std::is_member_function_pointer_v<Mf>
using get_class_t = typename get_class<Mf>::type;

template <typename T> struct remove_rvalue_reference { using type = T; };
template <typename T> struct remove_rvalue_reference<T &&> { using type = T; };

template <typename T> using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;

template <typename T> T copy_rvalue(T &&x) { return FWD(x); }
