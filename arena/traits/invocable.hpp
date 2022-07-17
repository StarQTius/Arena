#pragma once

#include <functional>

#include <forward.hpp>

// For IWYU
#define CallableWithSignature CallableWithSignature
#define MemberFunction MemberFunction
#define WithSignature WithSignature

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
  using free_signature_t = R (*)(C &, Args...);

  template <std::size_t I> using free_arg_t = std::tuple_element_t<I, free_args_t>;

  constexpr static auto free_arity = sizeof...(Args) + 1;
};

template <typename F> struct signature_info<std::function<F>> : signature_info<F *> {};

template <typename R, typename C, typename... Args>
struct signature_info<R (C::*)(Args...) const> : signature_info<R (*)(Args...)> {
  using signature_t = R (C::*)(Args...) const;
  using class_t = const C;

  using free_args_t = std::tuple<const C &, Args...>;
  using free_signature_t = R (*)(const C &, Args...);

  template <std::size_t I> using free_arg_t = std::tuple_element_t<I, free_args_t>;

  constexpr static auto free_arity = sizeof...(Args) + 1;
};

template <typename F> using signature_t = typename signature_info<F>::signature_t;
template <typename F> using args_t = typename signature_info<F>::args_t;
template <typename F> using return_t = typename signature_info<F>::return_t;
template <typename F> using class_t = typename signature_info<F>::class_t;

template <typename F> using free_signature_t = typename signature_info<F>::free_signature_t;
template <typename F> using free_args_t = typename signature_info<F>::free_args_t;

template <std::size_t I, typename F> using arg_t = typename signature_info<F>::template arg_t<I>;
template <std::size_t I, typename F> using free_arg_t = typename signature_info<F>::template free_arg_t<I>;

template <typename F> constexpr auto arity_v = signature_info<F>::arity;
template <typename F> constexpr auto free_arity_v = signature_info<F>::free_arity;

template <typename...> struct prepend_args;
template <typename R, typename... Args, typename T> struct prepend_args<R (*)(Args...), T> {
  using type = R (*)(T, Args...);
};
template <typename R, typename C, typename... Args, typename T> struct prepend_args<R (C::*)(Args...), T> {
  using type = R (C::*)(T, Args...);
};

template <typename F, typename... Ts> using prepend_args_t = typename prepend_args<F, Ts...>::type;

template <typename F>
concept CallableWithSignature = requires(F f) {
  std::function{FWD(f)};
};

template <typename Mf>
concept MemberFunction = std::is_member_function_pointer_v<std::decay_t<Mf>>;

template <typename F>
concept WithSignature = CallableWithSignature<F> || MemberFunction<F>;

template <typename F, std::size_t I, typename T>
concept Accepting = WithSignature<F> && I < arity_v<F> && std::convertible_to<T, arg_t<I, std::decay_t<F>>>;

template <WithSignature F> auto *signature_ptr(F &&) { return (signature_t<std::decay_t<F>>)nullptr; }

template <WithSignature F> auto *free_signature_ptr(F &&) { return (free_signature_t<std::decay_t<F>>)nullptr; }

template <WithSignature F> auto *args_ptr(F &&) { return (args_t<std::decay_t<F>> *)nullptr; }

template <WithSignature F> auto *free_args_ptr(F &&) { return (free_args_t<std::decay_t<F>> *)nullptr; }

template <typename Mf>
requires std::is_member_function_pointer_v<Mf>
auto make_free_function(Mf mf) {
  auto impl = [&]<typename... Args>(std::tuple<Args...> *) {
    return [mf](Args... args) { return std::invoke(mf, FWD(args)...); };
  };

  return impl(free_args_ptr(mf));
}
