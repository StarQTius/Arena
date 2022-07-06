#include <array>
#include <cmath>
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
#include "utility.hpp"

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

PyGameDrawer &enter_drawer(PyGameDrawer &self) {
  auto pygame_pymodule = py::module::import("pygame");
  auto init_pyfunction = pygame_pymodule.attr("init");

  auto display_pymodule = pygame_pymodule.attr("display");
  auto set_mode_pyfunction = display_pymodule.attr("set_mode");

  init_pyfunction();
  self.bind_screen(set_mode_pyfunction(py::make_tuple(800, 500)));
  return self;
}

void exit_drawer(PyGameDrawer &self, py::object, py::object, py::object) {
  auto pygame_pymodule = py::module::import("pygame");
  auto quit_pyfunction = pygame_pymodule.attr("quit");

  self.unbind_screen();
  quit_pyfunction();
}

void set_forward_velocity(b2Body &self, speed_t speed) {
  auto angle = self.GetAngle();
  self.SetLinearVelocity({std::cos(angle) * speed.number(), std::sin(angle) * speed.number()});
}

void set_position(b2Body &self, length_vec position) { self.SetTransform(to_box2d(position), self.GetAngle()); }

void set_angle(b2Body &self, angle_t angle) { self.SetTransform(self.GetPosition(), to_box2d(angle)); }

} // namespace

void initialize_base(py::module_ &pymodule) {
  using rvp = pybind11::return_value_policy;

  register_fetcher("Environment", [](Environment &retval, entt::entity) { return py::cast(retval); });
  register_fetcher("Entity", [](Environment &, entt::entity retval) { return py::cast(retval); });
  register_fetcher("Body", get_component<component::BodyPtr>);

  pymodule                                                                                   //
      | def("create", DISAMBIGUATE_MEMBER(create, Environment &, entity::Bot))               //
      | def("create", DISAMBIGUATE_MEMBER(create, Environment &, const entity::c21::Cup &)); //

  py::class_<Environment>(pymodule, "Environment") | R"(
      Contains a simulated state)"                               //
      | ctor(&create_environment, "width"_a = 3, "height"_a = 2) //
      | def("create", create_from_pyargs)                        //
      | def("step", &Environment::step)                          //
      | def("get", fetch_component, rvp::reference_internal)     //
      | property("renderer", &Environment::renderer);            //

  py::enum_<entt::entity>(pymodule, "Entity");

  py::class_<PyGameDrawer>(pymodule, "Renderer") | R"(
      Manage a rendering context
      It is implemented as a context manager. When entering the context, a window will appear that renders the
      evolution of the simulated state the manager is associated with.)" //
      | def("__enter__", enter_drawer)                                   //
      | def("__exit__", exit_drawer);                                    //

  py::class_<b2Body, ObserverPtr<b2Body>>(pymodule, "Body") | R"(
      Represents a physical body in a simulated state)"                                                               //
      | box2d_property<length_vec>("position", &b2Body::GetPosition, rvp::automatic)                                  //
      | box2d_property<angle_t>("angle", &b2Body::GetAngle, rvp::automatic)                                           //
      | box2d_property<speed_vec>("velocity", &b2Body::GetLinearVelocity, &b2Body::SetLinearVelocity, rvp::automatic) //
      | box2d_property<angular_speed_t>("angular_velocity", &b2Body::GetAngularVelocity, &b2Body::SetAngularVelocity,
                                        rvp::automatic) //
      | property(
            "forward_velocity", []() {}, set_forward_velocity, rvp::automatic) //
      | def("set_position", set_position)                                      //
      | def("set_angle", set_angle);                                           //

  py::class_<entity::Bot>(pymodule, "Bot") | R"(
      Contains data for creating a bot entity.)"                                                                     //
      | ctor<length_t, length_t, mass_t, py::function, size_t>("x"_a, "y"_a, "mass"_a, "logic"_a, "cup_capacity"_a); //
}

PYBIND11_MODULE(_details, pymodule) {
  initialize_base(pymodule);
  initialize_c21(pymodule);
}
