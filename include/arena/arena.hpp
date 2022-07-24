#pragma once

#include <ltl/functional.h>
#include <ltl/operator.h>
#include <ltl/optional.h>
#include <tl/expected.hpp>
#include <entt/entity/entity.hpp>

#include "../../arena/traits/template.hpp"
#include "ltl/Tuple.h"

#define ARENA_FWD(X) static_cast<decltype(X) &&>(X)

#define ARENA_ASSERT(CONDITION, ERROR)                                                                                 \
  if (!(CONDITION))                                                                                                    \
  return unexpected(ERROR)

#define ARENA_IF_LEGAL(...) \
  if constexpr (requires { __VA_ARGS__ }) { \
    __VA_ARGS__ \
  } \

namespace arena {

template <typename T>
concept Optional =
    InstanceOf<std::remove_cvref_t<T>, tl::expected> || InstanceOf<std::remove_cvref_t<T>, ltl::optional>;

enum class Error { NOT_ATTACHED, ALREADY_ATTACHED, NOT_IN_STORAGE, STORAGE_FULL, NOT_OWNED, ALREADY_OWNED, DANGLING_COMPONENT, UNKNOWN = 0 };

template <typename T = void> using Expected = tl::expected<T, Error>;

constexpr auto unexpected(Error error) { return tl::unexpected<Error>{error}; }

template <typename T> constexpr auto expected(T &&x) { return Expected<std::remove_cvref_t<T>>{ARENA_FWD(x)}; }

constexpr auto expected(Optional auto &&opt, Error error) {
  return opt ? Expected<std::remove_cvref_t<decltype(*opt)>>(ARENA_FWD(*opt)) : unexpected(error);
}

template <typename T> constexpr auto expected(T *p, Error error) {
  return p ? Expected<std::reference_wrapper<T>>(std::ref(*p)) : unexpected(error);
}

constexpr auto expected(entt::entity entity, Error error) {
  return entity != entt::null ? Expected<entt::entity>{entity} : unexpected(error);
}

template <typename T> constexpr auto expected(std::reference_wrapper<T> ref, Error error) {
  return expected(static_cast<T &>(ref), error);
}

template<typename... Ts>
constexpr auto expected(const Expected<Ts> &... xps) {
  using namespace ltl;

  bool error_found = false;
  Error error;
  for_each(tuple_t{std::ref(xps)...}, [&](auto &xp) {
    if (!xp && !error_found) {
      error_found = true;
      error = xp.error();
    }
  });

  return !error_found ? expected(tuple_t{xps.value()...}) : unexpected(error);
}

constexpr auto expected() { return Expected<>{}; }

constexpr inline auto if_invalid = ltl::curry(_((error, p), expected(p, error)));

} // namespace arena
