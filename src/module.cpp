#include <stdexcept>
#include <string>

#include <SFML/Graphics.hpp>
#include <box2d/b2_circle_shape.h>
#include <ltl/Range/Map.h>
#include <ltl/tuple_algos.h>
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
using namespace std::literals::string_literals;
using namespace units::isq::si::literals;
using namespace units::isq::si::length_references;
using namespace units::isq::si::mass_references;
using namespace units::isq::si::time_references;

namespace {

const auto bot_shape = component::make_circle_shape(10_q_cm);

sf::Font font;

auto sfml_color(component::CupColor &cup_color) {
  switch (cup_color) {
  case component::CupColor::RED:
    return sf::Color::Red;
  case component::CupColor::GREEN:
    return sf::Color::Green;
  default:
    return sf::Color::Transparent;
  }
}

auto cup_sprite(const component::BodyPtr &body_ptr, component::CupColor cup_color) {
  constexpr float radius = 10;

  sf::CircleShape sprite{radius};
  sprite.setOutlineColor(sfml_color(cup_color));
  sprite.setFillColor(sf::Color::Transparent);
  sprite.setOutlineThickness(2);
  sprite.setOrigin(sf::Vector2f{radius, radius});
  sprite.setPosition(sfdd::SCALE * sf::Vector2f{body_ptr->GetWorldCenter().x, body_ptr->GetWorldCenter().y});

  return sprite;
}

auto bot_sprite(const component::BodyPtr &body_ptr, component::CupGrabber &cup_grabber) {
  sf::Text text{"RED: "s + std::to_string(cup_grabber.storage[component::CupColor::RED]) + "\nGREEN: "s +
                    std::to_string(cup_grabber.storage[component::CupColor::GREEN]),
                font, 15};
  text.setPosition(sfdd::SCALE * sf::Vector2f{body_ptr->GetWorldCenter().x, body_ptr->GetWorldCenter().y} +
                   sf::Vector2f{-30, -70});

  return text;
}

FetcherMap fetchers{{"Environment", [](Environment &retval, entt::entity) { return py::cast(retval); }},
                    {"Entity", [](Environment &, entt::entity retval) { return py::cast(retval); }},
                    {"Body", get_component<component::BodyPtr>},
                    {"CupGrabber", get_component_with_environment<component::CupGrabber>}};

void upkeep(Environment &environment) {
  for (auto &&[self, py_host] : environment.registry.view<component::PyHost>().each())
    py_host.invoke(environment, self, fetchers);

  if (environment.renderer.isOpen()) {
    environment.registry.view<component::BodyPtr, component::CupColor>().each(
        [&](const component::BodyPtr &body_ptr, component::CupColor cup_color) {
          environment.renderer.draw(cup_sprite(body_ptr, cup_color));
        });
    environment.registry.view<component::BodyPtr, component::CupGrabber>().each(
        [&](const component::BodyPtr &body_ptr, component::CupGrabber cup_grabber) {
          environment.renderer.draw(bot_sprite(body_ptr, cup_grabber));
        });
  }
}

} // namespace

ARENA_MODULE(arena, module) {
  if (!font.loadFromFile(ARENA_ASSET_PATH "/F25_Bank_Printer.ttf"))
    throw std::runtime_error{"Font not found"};

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

  py::class_<WithEnvironment<component::CupGrabber>>(module, "CupGrabber")
      .def("grab", [](WithEnvironment<component::CupGrabber> &self,
                      entt::entity target) { return self.value.get().grab(self.environment.get(), target); })
      .def("drop", [](WithEnvironment<component::CupGrabber> &self, const entity::Cup &cup) {
        return self.value.get().drop(self.environment.get(), cup);
      });

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
