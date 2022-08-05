#pragma once

#include <tuple>

#include "common.hpp"

#if defined(ARENA_IWYU)
#define TemplateInstance TemplateInstance
#define InstanceOf InstanceOf
#endif // defined(ARENA_IWYU)

namespace arena {
namespace detail {

template <typename> struct is_template_instance : std::false_type {};
template <template <typename...> typename TT, typename... Ts>
struct is_template_instance<TT<Ts...>> : std::true_type {};

template <typename, template <typename...> typename> struct is_instance_of : std::false_type {};
template <template <typename...> typename TT, typename... Ts> struct is_instance_of<TT<Ts...>, TT> : std::true_type {};

} // namespace detail

template <typename> struct template_parameters;
template <typename... Ts, template <typename...> typename TT> struct template_parameters<TT<Ts...>> {
  using type = std::tuple<Ts...>;
};

template <typename T> using template_parameters_t = typename template_parameters<T>::type;

template <std::size_t I, typename T> using template_parameter_t = std::tuple_element_t<I, template_parameters_t<T>>;

template <typename> struct template_arity;
template <template <typename...> typename TT, typename... Ts>
struct template_arity<TT<Ts...>> : integral_constant<sizeof...(Ts)> {};

template <typename T> constexpr auto template_arity_v = template_arity<T>::value;

template <typename T>
concept TemplateInstance = detail::is_template_instance<T>::value;

template <typename T, template <typename...> typename TT>
concept InstanceOf = TemplateInstance<T> && detail::is_instance_of<T, TT>::value;

} // namespace arena
