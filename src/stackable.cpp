#include <arena/component/stackable.hpp>
#include <ltl/functional.h>

arena::Expected<arena::component::Stackable> arena::component::make_stackable(Environment &environment,
                                                                              entt::entity top) {
  using ltl::unzip;
  using enum Error;

  if (top == entt::null)
    return expected(Stackable{.next = entt::null});

  return environment.try_get<Stackable, b2Body *>(top).and_then(
      unzip([&](auto &&, auto *top_body_p) -> Expected<Stackable> {
        ARENA_ASSERT(top_body_p->IsEnabled(), DISABLED_BODY);

        top_body_p->SetEnabled(false);
        return expected(Stackable{.next = top});
      }));
}
