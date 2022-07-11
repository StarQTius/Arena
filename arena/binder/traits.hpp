#pragma once

#include <pybind11/pybind11.h>

#include <arena/binding/fetcher.hpp> // IWYU pragma: keep

#include "../utility.hpp"
#include <forward.hpp>

template <typename Mf>
concept Getter = !std::is_void_v<std::invoke_result<Mf, get_class_t<Mf> &>> && Invocable<Mf>;

template <typename Mf>
concept Setter = Getter<Mf> && std::assignable_from < std::invoke_result < Mf,
        get_class_t<Mf>
& >, std::invoke_result<Mf, get_class_t<Mf> &> > ;

template <typename T>
concept PybindClass = requires(T x) {
  {
    pybind11::class_ { FWD(x) }
    } -> std::convertible_to<std::decay_t<T>>;
};

template <typename T>
concept BindableTo = requires(T x) {
  {
    x.def("", [](T &) {})
    } -> std::convertible_to<T>;
};
