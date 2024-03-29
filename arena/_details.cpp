#include <array>
#include <cmath>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <entt/core/any.hpp>
#include <entt/core/fwd.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>
#include <entt/signal/delegate.hpp>
#include <ltl/Range/Map.h>
#include <ltl/operator.h>
#include <pybind11/attr.h>
#include <pybind11/cast.h>
#include <pybind11/functional.h> // IWYU pragma: keep
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h> // IWYU pragma: keep
#include <tl/expected.hpp>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/time.h>

#include <arena/component/body.hpp>
#include <arena/component/ray.hpp>
#include <arena/component/stackable.hpp>
#include <arena/draw.hpp>
#include <arena/entity/bot.hpp>
#include <arena/entity/field.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>
#include <arena/signal.hpp>

#include "binder/_entity.hpp"
#include "binder/component.hpp"
#include "binder/ctor.hpp"
#include "binder/def.hpp"
#include "binder/doc.hpp"
#include "binder/fetcher.hpp"
#include "binder/host.hpp"
#include "binder/internal_component.hpp"
#include "binder/normalize.hpp"
#include "binder/property.hpp"
#include "binder/static_def.hpp"
#include "box2d.hpp"
#include "caster.hpp" // IWYU pragma: keep
#include "common.hpp"
#include "component_ref.hpp"
#include "physics.hpp"
#include "python.hpp"
#include <Python.h>

namespace py = pybind11;

using namespace arena;
using namespace py::literals;
using namespace std::literals::string_literals;
using namespace units::isq::si::literals;
using namespace units::isq::si::length_references;
using namespace units::isq::si::mass_references;
using namespace units::isq::si::time_references;

namespace {

void upkeep(duration_t, void *environment_p, entt::any, entt::any) {
  auto &environment = *static_cast<Environment *>(environment_p);

  for (auto &&[self, py_host] : environment.view<PyHost>().each())
    py_host.invoke(environment, self);
}

//! \brief Create a new environment
//! The new environment is allocated dynamically because it cannot be copied or
//! moved from.
Environment *create_environment(length_t width, length_t height) {
  auto *retval = new Environment{entt::connect_arg<upkeep>};
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

auto call_pyobject_as_collision_pycallback(PyObject &pycallback, const CollisionBeginning &event) {
  return py::handle{&pycallback}(event.entity_a, event.entity_b);
}

void register_on_collision_pycallback(Environment &environment, py::function pycallback) {
  environment.on_collision<call_pyobject_as_collision_pycallback>(*pycallback.ptr());
}

auto call_pyobject_as_ray_fired_pycallback(PyObject &pycallback, const RayFired &event) {
  return py::handle{&pycallback}(event.entity, numpy_number(event.x), numpy_number(event.y), numpy_number(event.angle),
                                 numpy_number(event.distance));
}

void register_on_ray_fired_pycallback(Environment &environment, py::function pycallback) {
  environment.on_ray_fired<call_pyobject_as_ray_fired_pycallback>(*pycallback.ptr());
}

void draw_segment(PyGameDrawer &self, length_vec p1, length_vec p2) {
  self.DrawSegment(to_box2d(p1), to_box2d(p2), {1, 1, 1});
}

auto ray_sweep(component::Ray &self, Environment &environment, angle_t fov, std::size_t definition) {
  using ltl::map, ltl::to_vector;
  return self.sweep(environment, fov, definition).value_or(std::vector<length_t>{}) | map(numpy_number<length_t &>) |
         to_vector;
}

bool has_all_of_pycomponents(Environment &self, entt::entity entity, py::list component_pytypes) {
  for (auto &pytype : component_pytypes) {
    if (!has_pycomponent(self, entity, pytype))
      return false;
  }

  return true;
}

} // namespace

void initialize_base(py::module_ &pymodule) {
  using rvp = pybind11::return_value_policy;

  py::class_<Environment>(pymodule, "Environment") | R"(
      Contains a simulated state)"                                                    //
      | ctor(create_environment, "width"_a = 3000, "height"_a = 2000)                 //
      | def("create", create_pyentity)                                                //
      | def("step", &Environment::step)                                               //
      | def("get", get_pycomponent, rvp::reference_internal)                          //
      | def("attach", attach_pycomponent)                                             //
      | def("all_of", has_all_of_pycomponents)                                        //
      | def("on_collision", register_on_collision_pycallback, py::keep_alive<1, 2>{}) //
      | def("on_ray_fired", register_on_ray_fired_pycallback, py::keep_alive<1, 2>{}) //
      | property("renderer", &Environment::renderer)                                  //
      | static_def(
            "__get", [](Environment & environment, entt::entity) -> auto & { return environment; },
            rvp::reference); //

  py::enum_<entt::entity>(pymodule, "Entity").value("NULL", entt::null);

  py::class_<PyGameDrawer>(pymodule, "Renderer") | R"(
      Manage a rendering context
      It is implemented as a context manager. When entering the context, a window will appear that renders the
      evolution of the simulated state the manager is associated with.)" //
      | def("__enter__", enter_drawer)                                   //
      | def("__exit__", exit_drawer)                                     //
      | def("draw_segment", draw_segment);                               //

  kind::internal_component<b2Body>(pymodule, "Body") | R"(
      Represents a physical body in a simulated state)"                                                               //
      | box2d_property<length_vec>("position", &b2Body::GetPosition, rvp::automatic)                                  //
      | box2d_property<angle_t>("angle", &b2Body::GetAngle, rvp::automatic)                                           //
      | box2d_property<speed_vec>("velocity", &b2Body::GetLinearVelocity, &b2Body::SetLinearVelocity, rvp::automatic) //
      | box2d_property<angular_speed_t>("angular_velocity", &b2Body::GetAngularVelocity, &b2Body::SetAngularVelocity,
                                        rvp::automatic) //
      | property(
            "forward_velocity", [](b2Body &) {}, set_forward_velocity, rvp::automatic) //
      | def("set_position", set_position)                                              //
      | def("set_angle", set_angle)                                                    //
      | property("is_enabled", &b2Body::IsEnabled);                                    //

  kind::component<component::Stackable>(pymodule, "Stackable") | def("stack", &component::Stackable::stack) |
      def("unstack", &component::Stackable::unstack) |
      property("range",
               [](component::Stackable &stackable, Environment &environment) { return stackable.range(environment); });

  kind::component<component::Ray>(pymodule, "Ray") |
      ctor<length_t, length_t, length_t, angle_t, std::function<bool(entt::entity)>>(
          "x"_a = 0, "y"_a = 0, "range"_a, "angle"_a = 0,
          "filter"_a = std::function{[](entt::entity) { return true; }}) //
      | def("cast", &component::Ray::cast)                               //
      | def("sweep", ray_sweep, "fov"_a, "definition"_a);                //

  kind::component<PyHost>(pymodule, "Host") | ctor<py::function>();

  kind::entity<entity::Bot>(pymodule, "Bot") | R"(
      Contains data for creating a bot entity.)"                  //
      | ctor<length_t, length_t, mass_t>("x"_a, "y"_a, "mass"_a); //
}

PYBIND11_MODULE(_details, pymodule) {
  initialize_base(pymodule);
  initialize_sail_the_world(pymodule);
  initialize_the_cherry_on_the_cake(pymodule.def_submodule("the_cherry_on_the_cake"));
}

void ARENA_WEAK initialize_sail_the_world(pybind11::module_ &) {}
void ARENA_WEAK initialize_the_cherry_on_the_cake(pybind11::module_ &&) {}
