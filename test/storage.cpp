#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>
#include <utility>

#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>
#include <ltl/Range/Filter.h>
#include <ltl/Range/Map.h>
#include <ltl/Range/Taker.h>
#include <ltl/Range/seq.h>
#include <ltl/algos.h>
#include <ltl/functional.h>
#include <ltl/operator.h>
#include <units/isq/si/length.h>

#include <arena/component/body.hpp>
#include <arena/component/storage.hpp>
#include <arena/environment.hpp>

using namespace arena;

namespace {

struct storable_t {
  size_t storing_count, removal_count;
};

} // namespace

template <> struct arena_component_info<storable_t> {
  static void on_storing(storable_t &storable, Environment &) { storable.storing_count++; }

  static void on_removal(storable_t &storable, Environment &) { storable.removal_count++; }
};

TEST_CASE("Storage component", "[component]") {
  using namespace arena::component;

  Environment environment{[](auto &&...) {}};
  auto &storage = environment.attach(environment.create(), Storage<storable_t>{3});

  auto create_entity = [&]() { return environment.create(); };

  SECTION("Storage components can store several entity references up to its maximum capacity") {
    using namespace ltl;

    for (auto entity : seq(create_entity) | take_n(4))
      environment.attach(entity, storable_t{});
    for (auto entity : storage.view(environment)) {
      REQUIRE(bool(storage.store(environment, entity)) ^ (storage.count() == storage.capacity()));
    }

    REQUIRE(count_if(storage.view(environment).each(), unzip(_((, storable), storable.storing_count == 1))) ==
            (int)storage.capacity());
  }

  SECTION("Storage components owned entity references can be iterated through") {
    using namespace ltl;

    for (auto entity : seq(create_entity) | take_n(3)) {
      environment.attach(entity, storable_t{});
      storage.store(environment, entity);
    }

    for (auto entity : storage.owned(environment)) {
      auto view = storage.view(environment);
      REQUIRE(find_ptr(view, entity));
    }
  }

  SECTION("Entity references can be removed from storage components") {
    using namespace ltl;

    for (auto entity : seq(create_entity) | take_n(3)) {
      environment.attach(entity, storable_t{});
      storage.store(environment, entity);
    }

    for (auto entity : storage.owned(environment))
      REQUIRE(storage.remove(environment, entity));

    REQUIRE(storage.count() == 0);
    REQUIRE(all_of(storage.view(environment).each(),
                   unzip(_((, storable), storable.storing_count == 1 && storable.removal_count == 1))));
  }

  SECTION("Removing an unowned entity from storage component yield an error") {
    using namespace ltl;

    for (auto entity : seq(create_entity) | take_n(2)) {
      environment.attach(entity, storable_t{});
      storage.store(environment, entity);
    }

    auto owned_entity = *storage.owned(environment).begin();
    auto unowned_entity = environment.create();
    environment.attach(unowned_entity, storable_t{});

    REQUIRE(storage.remove(environment, owned_entity));
    REQUIRE(!storage.remove(environment, owned_entity));
    REQUIRE(!storage.remove(environment, unowned_entity));
  }
}
