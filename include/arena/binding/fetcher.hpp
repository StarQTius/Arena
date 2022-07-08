#pragma once

#include <memory>
#include <variant>

#include <entt/entity/entity.hpp>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

#include <arena/environment.hpp>

namespace arena {

// Observer pointer wrapper for Python (because it seems pybind11 is handling raw pointers in a specific way)
template <typename T> using ObserverPtr = std::unique_ptr<T, decltype([](T *) {})>;

template <typename T> class ComponentRef {
public:
  explicit ComponentRef(std::convertible_to<T> auto &&component) : m_state{new T{FWD(component)}} {}
  explicit ComponentRef(Environment &environment, entt::entity entity)
      : m_state{std::in_place_index<1>, environment, entity} {}

  void attach(Environment &environment, entt::entity entity) {
    emplace(environment, entity, std::move(*std::get<0>(m_state)));
    m_state.emplace<1>(environment, entity);
  }

  Environment &environment() { return std::get<1>(m_state).first; }

  entt::entity entity() const { return std::get<1>(m_state).second; }

  T *operator->() {
    auto &[environment, entity] = std::get<1>(m_state);
    return environment.registry.template try_get<T>(entity);
  }

  const T *operator->() const {
    auto &[environment, entity] = std::get<1>(m_state);
    return environment.registry.template try_get<T>(entity);
  }

private:
  static void emplace(Environment &environment, entt::entity entity, std::convertible_to<T> auto &&component) {
    environment.registry.emplace_or_replace(entity, FWD(component));
  }

  std::variant<std::unique_ptr<T>, std::pair<Environment &, entt::entity>> m_state;
};

inline pybind11::object get_pycomponent(Environment &environment, entt::entity entity, pybind11::object pytype) {
  return pytype.attr("__get")(environment, entity);
}

inline pybind11::object create_pyentity(Environment &environment, pybind11::object pyentity_data) {
  return pyentity_data.attr("__create")(environment);
}

} // namespace arena
