#include <cmath>
#include <stdexcept>
#include <string>

#include <box2d/b2_circle_shape.h>
#include <ltl/Range/Map.h>
#include <ltl/tuple_algos.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <units/isq/si/length.h>

#include <arena/2021/cup.hpp>
#include <arena/binding/fetcher.hpp>
#include <arena/component/body.hpp>
#include <arena/component/host.hpp>
#include <arena/entity/bot.hpp>
#include <arena/entity/field.hpp>
#include <arena/environment.hpp>

#ifdef ARENA_EMBED
#include <pybind11/embed.h>
#define ARENA_MODULE(NAME, M) PYBIND11_EMBEDDED_MODULE(NAME, M)
#else // ARENA_EMBED
#define ARENA_MODULE(NAME, M) PYBIND11_MODULE(NAME, M)
#endif // ARENA_EMBED

namespace py = pybind11;

using namespace arena;
using namespace py::literals;
using namespace std::literals::string_literals;
using namespace units::isq::si::literals;
using namespace units::isq::si::length_references;
using namespace units::isq::si::mass_references;
using namespace units::isq::si::time_references;

namespace {

const auto bot_shape = component::make_circle_shape(10_q_cm);

FetcherMap fetchers{{"Environment", [](Environment &retval, entt::entity) { return py::cast(retval); }},
                    {"Entity", [](Environment &, entt::entity retval) { return py::cast(retval); }},
                    {"Body", get_component<component::BodyPtr>},
                    {"CupGrabber", get_component_with_environment<component::CupGrabber>}};

void upkeep(Environment &environment) {
  for (auto &&[self, py_host] : environment.registry.view<component::PyHost>().each())
    py_host.invoke(environment, self, fetchers);

  /*if (environment.renderer) {
    environment.registry.view<component::BodyPtr, component::CupColor>().each(
        [&](const component::BodyPtr &body_ptr, component::CupColor cup_color) {
          environment.renderer.draw(cup_sprite(body_ptr, cup_color));
        });
  }*/
}

} // namespace

ARENA_MODULE(arena, module) {
  py::class_<Environment, std::shared_ptr<Environment>>(module, "Environment")
      .def(py::init([](precision_t width, precision_t height) {
             auto environment_ptr = std::make_shared<Environment>(upkeep);
             environment_ptr->create(entity::Field{.width = width * m, .height = height * m});
             return environment_ptr;
           }),
           "width"_a = 3, "height"_a = 2)
      .def("create", [](Environment &self, const entity::Bot &bot) { self.create(bot, bot_shape); })
      .def("create", [](Environment &self, const entity::Cup &cup) { self.create(cup); })
      .def("step", [](Environment &self, precision_t dt) { self.step(dt * s); })
      .def_property_readonly(
          "renderer", [](const Environment &self) -> auto & { return self.renderer; })
      .def_property_readonly(
          "cups",
          [](Environment &self) {
            auto component_view = self.registry.view<component::BodyPtr, component::CupColor>().each() |
                                  ltl::map(dereference_tuple_elements_if_needed);
            return py::make_iterator(component_view.begin(), component_view.end());
          },
          py::return_value_policy::copy, py::keep_alive<0, 1>{});

  py::class_<entt::entity>(module, "Entity");

  py::class_<PyGameDrawer>(module, "Renderer")
      .def("__enter__",
           [](PyGameDrawer &self) {
             auto pygame_pymodule = py::module::import("pygame");
             auto init_pyfunction = pygame_pymodule.attr("init");

             auto display_pymodule = pygame_pymodule.attr("display");
             auto set_mode_pyfunction = display_pymodule.attr("set_mode");

             init_pyfunction();
             self.bind_screen(set_mode_pyfunction(py::make_tuple(800, 500)));
             return self;
           })
      .def("__exit__", [](PyGameDrawer &self, py::object, py::object, py::object) {
        auto pygame_pymodule = py::module::import("pygame");
        auto quit_pyfunction = pygame_pymodule.attr("quit");

        self.unbind_screen();
        quit_pyfunction();
      });

  //
  // Components
  //

  py::class_<b2Body, ObserverPtr<b2Body>>(module, "Body")
      .def_property_readonly("position",
                             [](const b2Body &self) {
                               auto &velocity = self.GetPosition();
                               return py::make_tuple(velocity.x, velocity.y);
                             })
      .def_property_readonly("angle", [](const b2Body &self) { return self.GetAngle(); })
      .def_property(
          "velocity",
          [](const b2Body &self) {
            auto &velocity = self.GetLinearVelocity();
            return py::make_tuple(velocity.x, velocity.y);
          },
          [](b2Body &self, py::tuple value) {
            self.SetLinearVelocity({py::cast<float>(value[0]), py::cast<float>(value[1])});
          })
      .def_property(
          "angular_velocity",
          [](const b2Body &self) {
            auto velocity = self.GetAngularVelocity();
            return velocity;
          },
          [](b2Body &self, float value) { self.SetAngularVelocity(value); })
      .def_property(
          "forward_velocity", [](const b2Body &) { return py::none{}; },
          [](b2Body &self, float value) {
            auto angle = self.GetAngle();
            self.SetLinearVelocity({std::cos(angle) * value, std::sin(angle) * value});
          })
      .def("set_position",
           [](b2Body &self, py::tuple position) {
             self.SetTransform({position[0].cast<precision_t>(), position[1].cast<precision_t>()}, self.GetAngle());
           })
      .def("set_angle", [](b2Body &self, precision_t angle) { self.SetTransform(self.GetPosition(), angle); });

  py::class_<WithEnvironment<component::CupGrabber>>(module, "CupGrabber")
      .def("grab", [](WithEnvironment<component::CupGrabber> &self,
                      entt::entity target) { return self.value.get().grab(self.environment.get(), target); })
      .def("drop", [](WithEnvironment<component::CupGrabber> &self,
                      const entity::Cup &cup) { return self.value.get().drop(self.environment.get(), cup); })
      .def_property_readonly(
          "storage", [](const WithEnvironment<component::CupGrabber> &self) { return self.value.get().storage; });

  //
  // Entities
  //

  py::class_<entity::Bot>(module, "Bot")
      .def(py::init([](precision_t x, precision_t y, precision_t mass, pybind11::function logic, size_t cup_capacity) {
             return entity::Bot{x * m, y * m, mass * kg, logic, cup_capacity};
           }),
           "x"_a, "y"_a, "mass"_a, "logic"_a, "cup_capacity"_a);

  py::class_<entity::Cup>(module, "Cup")
      .def(py::init([](precision_t x, precision_t y, component::CupColor color) {
             return entity::Cup{x * m, y * m, color};
           }),
           "x"_a, "y"_a, "color"_a);

  //
  // Miscellaneous
  //

  py::enum_<component::CupColor>(module, "CupColor")
      .value("RED", component::CupColor::RED)
      .value("GREEN", component::CupColor::GREEN);
}
