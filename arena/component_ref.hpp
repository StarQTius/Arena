#pragma once

#include <memory>
#include <variant>

#include <entt/entity/entity.hpp>
#include <ltl/functional.h>

#include <arena/component/common.hpp>
#include <arena/environment.hpp>
#include <arena/traits/crtp.hpp>
#include <arena/traits/template.hpp>
#include <arena/traits/type.hpp>

#include "utility.hpp"
#include <forward.hpp>

template <typename D> class ComponentRef_base {
  D &derived() { return static_cast<D &>(*this); }
  const D &derived() const { return static_cast<const D &>(*this); }

public:
  using raw_component_t = arena::template_parameter_t<0, D>;
  using component_t = std::remove_pointer_t<raw_component_t>;

  component_t &operator*() {
    using namespace arena;
    using enum arena::Error;

    Environment &environment = derived().environment();
    auto entity = derived().entity();

    if constexpr (std::is_pointer_v<raw_component_t>) {
      return environment.try_get<raw_component_t>(entity).and_then(if_invalid(UNKNOWN)).or_else(pyraise).value();
    } else {
      return environment.try_get<raw_component_t>(entity).or_else(pyraise).value();
    }
  }

  const component_t &operator*() const {
    using namespace arena;
    using enum arena::Error;

    Environment &environment = derived().environment();
    auto entity = derived().entity();

    if constexpr (std::is_pointer_v<raw_component_t>) {
      return environment.try_get<raw_component_t>(entity).and_then(if_invalid(UNKNOWN)).or_else(pyraise).value();
    } else {
      return environment.try_get<raw_component_t>(entity).or_else(pyraise).value();
    }
  }

  auto *operator->() { return &operator*(); }

  const auto *operator->() const { return &operator*(); }
};

template <arena::Component Component_T>
class InternalComponentRef : public ComponentRef_base<InternalComponentRef<Component_T>> {
public:
  explicit InternalComponentRef(arena::Environment &environment, entt::entity entity)
      : m_environment{environment}, m_entity{entity} {}

  arena::Environment &environment() { return m_environment; }

  entt::entity entity() const { return m_entity; }

private:
  arena::Environment &m_environment;
  entt::entity m_entity;
};

template <arena::Component Component_T> class ComponentRef : public ComponentRef_base<ComponentRef<Component_T>> {
public:
  explicit ComponentRef(std::convertible_to<Component_T> auto &&component)
      : m_state{std::in_place_index<0>, new Component_T{FWD(component)}} {}
  explicit ComponentRef(auto &&...args) requires arena::ListInitializableFrom<Component_T, decltype(args)...>
      : m_state{std::in_place_index<0>, new Component_T{FWD(args)...}} {}
  explicit ComponentRef(arena::Environment &environment, entt::entity entity)
      : m_state{std::in_place_index<1>, environment, entity} {}

  void attach(arena::Environment &environment, entt::entity entity) {
    emplace(environment, entity, std::move(*std::get<0>(m_state)));
    m_state.template emplace<1>(environment, entity);
  }

  arena::Environment &environment() { return std::get<1>(m_state).environment(); }

  entt::entity entity() const { return std::get<1>(m_state).entity(); }

private:
  static void emplace(arena::Environment &environment, entt::entity entity,
                      std::convertible_to<Component_T> auto &&component) {
    using enum arena::Error;

    if (environment.all_of<Component_T>(entity))
      pyraise(ALREADY_ATTACHED);
    environment.attach(entity, FWD(component));
  }

  std::variant<std::unique_ptr<Component_T>, InternalComponentRef<Component_T>> m_state;
};

template <typename Component_T> ComponentRef(Component_T &&) -> ComponentRef<Component_T>;

template <arena::CuriouslyRecurring<ComponentRef_base> ComponentRef_Component_T>
using component_t = typename ComponentRef_Component_T::component_t;
