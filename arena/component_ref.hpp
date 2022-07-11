#pragma once

#include <memory>
#include <variant>

#include <entt/entity/entity.hpp>

#include <arena/environment.hpp>

#include "traits/crtp.hpp" // IWYU pragma: keep
#include "traits/invocable.hpp"
#include "traits/template.hpp"
#include <forward.hpp>

template <typename D> class ComponentRef_base {
  D &derived() { return static_cast<D &>(*this); }
  const D &derived() const { return static_cast<const D &>(*this); }

public:
  using raw_component_t = template_parameter_t<0, D>;
  using component_t = std::remove_pointer_t<raw_component_t>;

  auto &operator*() {
    auto &environment = derived().environment();
    auto entity = derived().entity();

    if constexpr (std::is_pointer_v<raw_component_t>) {
      return **environment.registry.template try_get<raw_component_t>(entity);
    } else {
      return *environment.registry.template try_get<raw_component_t>(entity);
    }
  }

  const auto &operator*() const {
    auto &environment = derived().environment();
    auto entity = derived().entity();

    if constexpr (std::is_pointer_v<raw_component_t>) {
      return **environment.registry.template try_get<raw_component_t>(entity);
    } else {
      return *environment.registry.template try_get<raw_component_t>(entity);
    }
  }

  auto *operator->() { return &operator*(); }

  const auto *operator->() const { return &operator*(); }

  template <Accepting<0, component_t &> F> static auto make_wrapper(F &&f) {
    auto impl = [&]<typename R, typename T, typename... Args>(R(*)(T, Args...)) {
      return [f = std::move(f)](D &self, Args... args) { return std::invoke(f, *self, FWD(args)...); };
    };

    return impl(signature_ptr(f));
  }

  template <typename F>
  requires(Accepting<F, 0, component_t &> &&Accepting<F, 1, arena::Environment &>) static auto make_wrapper(F &&f) {
    auto impl = [&]<typename R, typename T1, typename T2, typename... Args>(R(*)(T1, T2, Args...)) {
      return
          [f = std::move(f)](D &self, Args... args) { return std::invoke(f, *self, self.environment(), FWD(args)...); };
    };

    return impl(signature_ptr(f));
  }

  template <WithSignature Mf>
  requires std::is_member_function_pointer_v<Mf>
  static auto make_wrapper(Mf mf) { return make_wrapper(make_free_function(mf)); }
};

template <typename T> class InternalComponentRef : public ComponentRef_base<InternalComponentRef<T>> {
public:
  explicit InternalComponentRef(arena::Environment &environment, entt::entity entity)
      : m_environment{environment}, m_entity{entity} {}

  arena::Environment &environment() { return m_environment; }

  entt::entity entity() const { return m_entity; }

private:
  arena::Environment &m_environment;
  entt::entity m_entity;
};

template <typename T> class ComponentRef : public ComponentRef_base<ComponentRef<T>> {
public:
  explicit ComponentRef(std::convertible_to<T> auto &&component)
      : m_state{std::in_place_index<0>, new T{FWD(component)}} {}
  explicit ComponentRef(auto &&...args) requires std::constructible_from<T, decltype(args)...>
      : m_state{std::in_place_index<0>, new T{FWD(args)...}} {}
  explicit ComponentRef(arena::Environment &environment, entt::entity entity)
      : m_state{std::in_place_index<1>, environment, entity} {}

  void attach(arena::Environment &environment, entt::entity entity) {
    emplace(environment, entity, std::move(*std::get<0>(m_state)));
    m_state.template emplace<1>(environment, entity);
  }

  arena::Environment &environment() { return std::get<1>(m_state).environment(); }

  entt::entity entity() const { return std::get<1>(m_state).entity(); }

private:
  static void emplace(arena::Environment &environment, entt::entity entity, std::convertible_to<T> auto &&component) {
    environment.registry.emplace_or_replace<T>(entity, FWD(component));
  }

  std::variant<std::unique_ptr<T>, InternalComponentRef<T>> m_state;
};

template <typename T> ComponentRef(T &&) -> ComponentRef<T>;

template <CuriouslyRecurring<ComponentRef_base> ComponentRef_T>
using component_t = typename ComponentRef_T::component_t;
