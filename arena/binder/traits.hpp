#pragma once

#include <pybind11/pybind11.h>

#include "../traits/invocable.hpp"
#include "../traits/template.hpp"

// For IWYU
#define Getter Getter
#define Setter Setter
#define Binding Binding

template <typename Mf>
concept Getter = MemberFunction<Mf> && !std::is_void_v<return_t<Mf>>;

template <typename Mf>
concept Setter = Getter<Mf> && std::assignable_from<return_t<Mf>, return_t<Mf>>;

template <typename T>
concept Binding = InstanceOf<std::remove_cvref_t<T>, pybind11::class_> ||
    std::same_as<std::remove_cvref_t<T>, pybind11::module_> || InstanceOf<std::remove_cvref_t<T>, pybind11::enum_>;
