#pragma once

#include <memory>
#include <variant>

#include <entt/entity/entity.hpp>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

#include <arena/environment.hpp>

#define ARENA_FWD(X) static_cast<decltype(X) &&>(X)

//! \brief Satisfied when `T` can be called
template <typename T>
concept Invocable = requires(T x) {
  std::function{ARENA_FWD(x)};
}
|| std::is_member_function_pointer_v<T>;

template <typename> struct get_free_signature;

template <typename R, typename C, typename... Args> struct get_free_signature<R (C::*)(Args...)> {
  using type = R(C &, Args...);
};
template <typename R, typename C, typename... Args> struct get_free_signature<R (C::*)(Args...) const> {
  using type = R(const C &, Args...);
};

template <typename Mf>
requires std::is_member_function_pointer_v<Mf>
using get_free_signature_t = typename get_free_signature<Mf>::type;

template <typename Mf>
requires std::is_member_function_pointer_v<Mf>
auto make_free_function(Mf mf) {
  auto impl = [&]<typename C, typename... Args>(auto (*)(C, Args...)) {
    return [mf](C self, Args... args) { return std::invoke(mf, FWD(self), FWD(args)...); };
  };

  return impl((get_free_signature_t<Mf> *)nullptr);
}

template <typename F> struct signature_info : signature_info<decltype(std::function{std::declval<F>()})> {};

template <typename R, typename... Args> struct signature_info<R (*)(Args...)> {
  using signature_t = R (*)(Args...);
  using return_t = R;
  using args_t = std::tuple<Args...>;

  constexpr static auto arity = sizeof...(Args);

  using free_args_t = args_t;
  using free_signature_t = signature_t;

  template <std::size_t I> using arg_t = std::tuple_element_t<I, args_t>;
  template <std::size_t I> using free_arg_t = std::tuple_element_t<I, free_args_t>;

  constexpr static auto free_arity = sizeof...(Args);
};

template <typename R, typename C, typename... Args>
struct signature_info<R (C::*)(Args...)> : signature_info<R (*)(Args...)> {
  using signature_t = R (C::*)(Args...);
  using class_t = C;

  using free_args_t = std::tuple<C &, Args...>;
  using free_signature_t = R(C &, Args...);

  template <std::size_t I> using free_arg_t = std::tuple_element_t<I, free_args_t>;

  constexpr static auto free_arity = sizeof...(Args) + 1;
};

template <typename F> struct signature_info<std::function<F>> : signature_info<F *> {};

template <typename R, typename C, typename... Args>
struct signature_info<R (C::*)(Args...) const> : signature_info<R (*)(Args...)> {
  using signature_t = R (C::*)(Args...) const;
  using class_t = C;

  using free_args_t = std::tuple<const C &, Args...>;
  using free_signature_t = R(const C &, Args...);

  template <std::size_t I> using free_arg_t = std::tuple_element_t<I, free_args_t>;

  constexpr static auto free_arity = sizeof...(Args) + 1;
};

template <std::size_t I, typename F> using arg_t = typename signature_info<F>::template arg_t<I>;

template <typename F> constexpr auto arity_v = signature_info<F>::arity;

namespace arena {

// Observer pointer wrapper for Python (because it seems pybind11 is handling raw pointers in a specific way)
template <typename T> using ObserverPtr = std::unique_ptr<T, std::identity>;

template <typename> struct get_template_parameters;

template <template <typename...> typename TT, typename... Ts> struct get_template_parameters<TT<Ts...>> {
  using type = std::tuple<Ts...>;
};

template <typename T> using get_template_parameters_t = typename get_template_parameters<T>::type;

template <std::size_t I, typename T>
using get_template_parameter_t = std::tuple_element_t<I, get_template_parameters_t<T>>;

auto *signature_ptr(auto &&f) {
  using function_t = decltype(std::function{ARENA_FWD(f)});
  return (function_t *)nullptr;
}

template <std::size_t I, typename F, typename T>
concept Accepting = I < arity_v<F> && std::constructible_from<arg_t<I, std::decay_t<F>>, T>;

template <typename D> class ComponentRef_base {
  D &derived() { return static_cast<D &>(*this); }
  const D &derived() const { return static_cast<const D &>(*this); }

public:
  using component_t = get_template_parameter_t<0, D>;

  auto &operator*() {
    auto &environment = derived().environment();
    auto entity = derived().entity();

    if constexpr (std::is_pointer_v<component_t>) {
      return **environment.registry.template try_get<component_t>(entity);
    } else {
      return *environment.registry.template try_get<component_t>(entity);
    }
  }

  const auto &operator*() const {
    auto &environment = derived().environment();
    auto entity = derived().entity();

    if constexpr (std::is_pointer_v<component_t>) {
      return **environment.registry.template try_get<component_t>(entity);
    } else {
      return *environment.registry.template try_get<component_t>(entity);
    }
  }

  auto *operator->() { return &operator*(); }

  const auto *operator->() const { return &operator*(); }

  template <typename F>
  requires Accepting < 0, F, std::remove_pointer_t<component_t>
  & > static auto make_wrapper(F &&f) {
    auto impl = [&]<typename R, typename T, typename... Args>(std::function<R(T, Args...)> *) {
      return [f = std::move(f)](D &self, Args... args) { return std::invoke(f, *self, ARENA_FWD(args)...); };
    };

    return impl(signature_ptr(f));
  }

  template <typename F>
  requires(Accepting<0, F, std::remove_pointer_t<component_t> &>
               &&Accepting<1, F, Environment &>) static auto make_wrapper(F &&f) {
    auto impl = [&]<typename R, typename T1, typename T2, typename... Args>(std::function<R(T1, T2, Args...)> *) {
      return [f = std::move(f)](D &self, Args... args) {
        return std::invoke(f, *self, self.environment(), ARENA_FWD(args)...);
      };
    };

    return impl(signature_ptr(f));
  }

  template <typename Mf>
  requires std::is_member_function_pointer_v<Mf>
  static auto make_wrapper(Mf mf) { return make_wrapper(make_free_function(mf)); }
};

template <typename T> class InternalComponentRef : public ComponentRef_base<InternalComponentRef<T>> {
public:
  explicit InternalComponentRef(Environment &environment, entt::entity entity)
      : m_environment{environment}, m_entity{entity} {}

  Environment &environment() { return m_environment; }

  entt::entity entity() const { return m_entity; }

private:
  Environment &m_environment;
  entt::entity m_entity;
};

template <typename T> class ComponentRef : public ComponentRef_base<ComponentRef<T>> {
public:
  explicit ComponentRef(std::convertible_to<T> auto &&component)
      : m_state{std::in_place_index<0>, new T{FWD(component)}} {}
  explicit ComponentRef(auto &&...args) requires std::constructible_from<T, decltype(args)...>
      : m_state{std::in_place_index<0>, new T{FWD(args)...}} {}
  explicit ComponentRef(Environment &environment, entt::entity entity)
      : m_state{std::in_place_index<1>, environment, entity} {}

  void attach(Environment &environment, entt::entity entity) {
    emplace(environment, entity, std::move(*std::get<0>(m_state)));
    m_state.template emplace<1>(environment, entity);
  }

  Environment &environment() { return std::get<1>(m_state).environment(); }

  entt::entity entity() const { return std::get<1>(m_state).entity(); }

private:
  static void emplace(Environment &environment, entt::entity entity, std::convertible_to<T> auto &&component) {
    environment.registry.emplace_or_replace<T>(entity, FWD(component));
  }

  std::variant<std::unique_ptr<T>, InternalComponentRef<T>> m_state;
};

template <typename T> ComponentRef(T &&) -> ComponentRef<T>;

inline pybind11::object get_pycomponent(Environment &environment, entt::entity entity, pybind11::object pytype) {
  return pytype.attr("__get")(environment, entity);
}

inline pybind11::object create_pyentity(Environment &environment, pybind11::object pyentity_data) {
  return pyentity_data.attr("__create")(environment);
}

inline pybind11::object attach_pycomponent(Environment &environment, entt::entity entity,
                                           pybind11::object pycomponent) {
  return pycomponent.attr("__attach")(environment, entity);
}

} // namespace arena
