#include <array>
#include <cmath>
#include <functional>
#include <string>
#include <type_traits>

#include <box2d/b2_body.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/time.h>

#include <arena/2021/cup.hpp>
#include <arena/binding/fetcher.hpp>
#include <arena/component/body.hpp>
#include <arena/component/host.hpp>
#include <arena/concept.hpp>
#include <arena/draw.hpp>
#include <arena/entity/bot.hpp>
#include <arena/entity/field.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

#include "binder.hpp"
#include "box2d.hpp"
#include "common.hpp"
#include "physics.hpp"
#include "python.hpp"
#include "utility.hpp"
#include "with_units.hpp"

namespace py = pybind11;

using namespace arena;
using namespace py::literals;
using namespace std::literals::string_literals;
using namespace units::isq::si::literals;
using namespace units::isq::si::length_references;
using namespace units::isq::si::mass_references;
using namespace units::isq::si::time_references;

namespace {

void upkeep(Environment &environment) {
  for (auto &&[self, py_host] : environment.registry.view<component::PyHost>().each())
    py_host.invoke(environment, self, get_fetchers());
}

entt::entity create_from_pyargs(Environment &self, py::args args, py::kwargs kwargs) {
  auto create_pyfunction = py::module_::import(ARENA_MODULE_NAME).attr("create");
  return create_pyfunction(self, *args, **kwargs).cast<entt::entity>();
}

//! \brief Create a new environment
//! The new environment is allocated dynamically because it cannot be copied or moved from.
Environment *create_environment(length_t width, length_t height) {
  auto *retval = new Environment{upkeep};
  retval->create(entity::Field{.width = width, .height = height});
  return retval;
};

} // namespace

void initialize_base(py::module_ &pymodule) {
  register_fetcher("Environment", [](Environment &retval, entt::entity) { return py::cast(retval); });
  register_fetcher("Entity", [](Environment &, entt::entity retval) { return py::cast(retval); });
  register_fetcher("Body", get_component<component::BodyPtr>);

  pymodule.def("create", DISAMBIGUATE_MEMBER(create, Environment &, const entity::Bot &))
      .def("create", DISAMBIGUATE_MEMBER(create, Environment &, const entity::c21::Cup &));

  py::class_<Environment>(pymodule, "Environment") | R"(
      Contains a simulated state)"                               //
      | ctor(&create_environment, "width"_a = 3, "height"_a = 2) //
      | def("create", create_from_pyargs)                        //
      | def("step", &Environment::step)                          //
      | def("get", fetch_component)                              //
      | property("renderer", &Environment::renderer);            //

  py::enum_<entt::entity>(pymodule, "Entity");
  py::class_<PyGameDrawer>(pymodule, "Renderer")
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

  py::class_<b2Body, ObserverPtr<b2Body>>(pymodule, "Body")
      .def_property_readonly("position", with_box2d_units<length_vec()>(&b2Body::GetPosition))
      .def_property_readonly("angle", with_box2d_units<angle_t()>(&b2Body::GetAngle))
      .def_property("velocity", with_box2d_units<speed_vec()>(&b2Body::GetLinearVelocity),
                    with_box2d_units<void(speed_vec)>(&b2Body::SetLinearVelocity))
      .def_property("angular_velocity", with_box2d_units<angular_speed_t()>(&b2Body::GetAngularVelocity),
                    with_box2d_units<void(angular_speed_t)>(&b2Body::SetAngularVelocity))
      .def_property("forward_velocity", noop<b2Body>,
                    with_units<void(b2Body &, speed_t)>([](b2Body &self, speed_t speed) {
                      auto angle = self.GetAngle();
                      self.SetLinearVelocity({std::cos(angle) * speed.number(), std::sin(angle) * speed.number()});
                    }))
      .def("set_position", with_units<void(b2Body &, length_vec)>([](b2Body &self, length_vec position) {
             self.SetTransform(to_box2d(position), self.GetAngle());
           }))
      .def("set_angle", with_units<void(b2Body &, angle_t)>([](b2Body &self, angle_t angle) {
             self.SetTransform(self.GetPosition(), to_box2d(angle));
           }));

  py::class_<entity::Bot>(pymodule, "Bot")
      .def(py::init(ctor_with_units<entity::Bot, length_t, length_t, mass_t, pybind11::function, size_t>()), "x"_a,
           "y"_a, "mass"_a, "logic"_a, "cup_capacity"_a);
}

PYBIND11_MODULE(_details, pymodule) {
  initialize_base(pymodule);
  initialize_c21(pymodule);
}
