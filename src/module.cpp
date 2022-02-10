#include <box2d/b2_circle_shape.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <units/isq/si/length.h>

#include <arena/2021/cup.hpp>
#include <arena/binding/fetcher.hpp>
#include <arena/component/body.hpp>
#include <arena/component/host.hpp>
#include <arena/entity/bot.hpp>
#include <arena/entity/field.hpp>
#include <arena/environment.hpp>

#ifdef ARENA_EMBED
#define ARENA_MODULE(NAME, M) PYBIND11_EMBEDDED_MODULE(NAME, M)
#else // ARENA_EMBED
#define ARENA_MODULE(NAME, M) PYBIND11_MODULE(NAME, M)
#endif // ARENA_EMBED

namespace py = pybind11;

using namespace arena;
using namespace py::literals;
using namespace units::isq::si::literals;
using namespace units::isq::si::length_references;
using namespace units::isq::si::mass_references;
using namespace units::isq::si::time_references;

namespace {

const auto bot_shape = component::make_circle_shape(10_q_cm);

FetcherMap fetchers{{"Body", get_component<component::BodyPtr>}};

void upkeep(entt::registry &registry) {
  for (auto &&[self, py_host] : registry.view<component::PyHost>().each())
    py_host.invoke(registry, self, fetchers);
}

} // namespace

ARENA_MODULE(arena, module) {
  py::class_<Environment, std::shared_ptr<Environment>>(module, "Environment")
      .def(py::init([]() {
        auto environment_ptr = std::make_shared<Environment>(upkeep);
        environment_ptr->create(entity::Field{.width = 3_q_m, .height = 2_q_m});
        return environment_ptr;
      }))
      .def("create", [](Environment &self, const entity::Bot &bot) { self.create(bot, bot_shape); })
      .def("create", [](Environment &self, const entity::Cup &cup) { self.create(cup); })
      .def("step", [](Environment &self, precision_t dt) { self.step(dt * s); })
      .def_property_readonly(
          "renderer", [](const Environment &self) -> auto & { return self.renderer; });

  py::class_<sf::RenderWindow>(module, "Renderer")
      .def("__enter__",
           [](sf::RenderWindow &self) {
             self.create(sf::VideoMode{1000, 800}, "Arena");
             self.setView(sf::View{{0, 0}, {1000, 800}});
           })
      .def("__exit__", [](sf::RenderWindow &self, py::object, py::object, py::object) { self.close(); });

  //
  // Components
  //

  py::class_<b2Body, ObserverPtr<b2Body>>(module, "Body")
      .def_property_readonly("position",
                             [](const b2Body &self) {
                               auto &velocity = self.GetPosition();
                               return py::make_tuple(velocity.x, velocity.y);
                             })
      .def_property(
          "velocity",
          [](const b2Body &self) {
            auto &velocity = self.GetLinearVelocity();
            return py::make_tuple(velocity.x, velocity.y);
          },
          [](b2Body &self, py::tuple value) {
            self.SetLinearVelocity({py::cast<float>(value[0]), py::cast<float>(value[1])});
          });

  //
  // Entities
  //

  py::class_<entity::Bot>(module, "Bot")
      .def(py::init([](precision_t x, precision_t y, precision_t mass, pybind11::function logic) {
             return entity::Bot(x * m, y * m, mass * kg, logic);
           }),
           py::arg("x"), py::arg("y"), py::arg("mass"), py::arg("logic"));

  py::class_<entity::Cup>(module, "Cup")
      .def(py::init([](precision_t x, precision_t y, component::Color color) {
             return entity::Cup{x * m, y * m, color};
           }),
           "x"_a, "y"_a, "color"_a);

  //
  // Miscellaneous
  //

  py::enum_<component::Color>(module, "Color")
      .value("RED", component::Color::RED)
      .value("GREEN", component::Color::GREEN);
}
