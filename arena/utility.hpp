#pragma once

#define DISAMBIGUATE(F, ...)                                                                                           \
  []<typename... Args>(std::tuple<Args...> *) {                                                                        \
    return [](Args... args) -> decltype(auto) { return F(FWD(args)...); };                                             \
  }                                                                                                                    \
  ((std::tuple<__VA_ARGS__> *)nullptr)

#define DISAMBIGUATE_MEMBER(F, ...)                                                                                    \
  []<typename T, typename... Args>(std::tuple<T, Args...> *) {                                                         \
    return [](T x, Args... args) -> decltype(auto) { return FWD(x).F(FWD(args)...); };                                 \
  }                                                                                                                    \
  ((std::tuple<__VA_ARGS__> *)nullptr)
