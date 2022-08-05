#pragma once

#include <pybind11/pybind11.h>

#include <arena/traits/invocable.hpp>
#include <arena/traits/template.hpp>

#if defined(ARENA_IWYU)
#define Getter Getter
#define Setter Setter
#define Binding Binding
#endif

template <typename Mf>
concept Getter = arena::MemberFunction<Mf> && !std::is_void_v<arena::return_t<Mf>>;

template <typename Mf>
concept Setter = Getter<Mf> && std::assignable_from<arena::return_t<Mf>, arena::return_t<Mf>>;

template <typename T>
concept Binding = arena::InstanceOf<std::remove_cvref_t<T>, pybind11::class_> ||
    std::same_as<std::remove_cvref_t<T>, pybind11::module_> ||
    arena::InstanceOf<std::remove_cvref_t<T>, pybind11::enum_>;
