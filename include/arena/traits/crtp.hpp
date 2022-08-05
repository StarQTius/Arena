#pragma once

#if defined(ARENA_IWYU)
#define CuriouslyRecurring CuriouslyRecurring
#endif // defined(ARENA_IWYU)

namespace arena {

template <typename> struct crtp_derived_t;
template <typename D, template <typename> typename Base_TT> struct crtp_derived_t<Base_TT<D>> { using type = D; };

template <typename D, template <typename> typename Base_TT>
concept CuriouslyRecurring = std::derived_from<D, Base_TT<D>>;

} // namespace arena
