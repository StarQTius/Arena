#include <arena/draw.hpp>

#include <array>
#include <utility>
#include <vector>

#include <box2d/b2_draw.h>
#include <box2d/b2_math.h>
#include <box2d/b2_types.h>
#include <ltl/Range/Map.h>
#include <ltl/operator.h>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <arena/physics.hpp>

namespace py = pybind11;

constexpr unsigned thickness = 2;

arena::PyGameDrawer::PyGameDrawer(precision_t scale) : m_scale{scale}, m_screen{py::none{}} {}

arena::PyGameDrawer::operator bool() const { return !m_screen.is(py::none{}); }

void arena::PyGameDrawer::show() const {
  auto display_pymodule = py::module::import("pygame").attr("display");
  auto flip_pyfunction = display_pymodule.attr("flip");
  auto fill_pyfunction = m_screen.attr("fill");

  flip_pyfunction();
  fill_pyfunction("black");
}

void arena::PyGameDrawer::bind_screen(pybind11::object screen) {
  m_screen = screen;
  m_offset_x = screen.attr("get_width")().cast<precision_t>() / 2;
  m_offset_y = screen.attr("get_height")().cast<precision_t>() / 2;
}

void arena::PyGameDrawer::unbind_screen() { m_screen = py::none{}; }

void arena::PyGameDrawer::DrawPoint(const b2Vec2 &, float, const b2Color &) {}

void arena::PyGameDrawer::DrawPolygon(const b2Vec2 *points, int32 nb_points, const b2Color &color) {
  auto draw_pymodule = py::module::import("pygame").attr("draw");
  auto polygon_pyfunction = draw_pymodule.attr("polygon");

  auto this_make_pygame_coordinate = [&](auto *point_ptr) { return make_pygame_coordinate(*point_ptr); };
  polygon_pyfunction(m_screen, make_pygame_color(color),
                     std::vector{points, points + nb_points} | ltl::map(this_make_pygame_coordinate) | ltl::to_vector);
}

void arena::PyGameDrawer::DrawSolidPolygon(const b2Vec2 *points, int32 nb_points, const b2Color &color) {
  auto draw_pymodule = py::module::import("pygame").attr("draw");
  auto polygon_pyfunction = draw_pymodule.attr("polygon");

  auto this_make_pygame_coordinate = [&](auto *point_ptr) { return make_pygame_coordinate(*point_ptr); };
  polygon_pyfunction(m_screen, make_pygame_color(color),
                     std::vector{points, points + nb_points} | ltl::map(this_make_pygame_coordinate) | ltl::to_vector,
                     thickness);
}

void arena::PyGameDrawer::DrawCircle(const b2Vec2 &center, float radius, const b2Color &color) {
  auto draw_pymodule = py::module::import("pygame").attr("draw");
  auto circle_pyfunction = draw_pymodule.attr("circle");

  circle_pyfunction(m_screen, make_pygame_color(color), make_pygame_coordinate(center), radius * m_scale);
}

void arena::PyGameDrawer::DrawSolidCircle(const b2Vec2 &center, float radius, const b2Vec2 &axis,
                                          const b2Color &color) {
  auto draw_pymodule = py::module::import("pygame").attr("draw");
  auto circle_pyfunction = draw_pymodule.attr("circle");
  auto line_pyfunction = draw_pymodule.attr("line");

  circle_pyfunction(m_screen, make_pygame_color(color), make_pygame_coordinate(center), radius * m_scale, thickness);
  line_pyfunction(m_screen, make_pygame_color(color), make_pygame_coordinate(center),
                  make_pygame_coordinate(center + radius * axis), thickness);
}

void arena::PyGameDrawer::DrawSegment(const b2Vec2 &p1, const b2Vec2 &p2, const b2Color &color) {
  auto draw_pymodule = py::module::import("pygame").attr("draw");
  auto line_pyfunction = draw_pymodule.attr("line");

  line_pyfunction(m_screen, make_pygame_color(color), make_pygame_coordinate(p1), make_pygame_coordinate(p2),
                  thickness);
}

void arena::PyGameDrawer::DrawTransform(const b2Transform &) {}

py::tuple arena::PyGameDrawer::make_pygame_coordinate(const b2Vec2 &vector) const {
  return py::make_tuple(vector.x * m_scale + m_offset_x, vector.y * m_scale + m_offset_y);
}

py::object arena::PyGameDrawer::make_pygame_color(const b2Color &color) {
  auto color_pyclass = py::module::import("pygame").attr("Color");

  return color_pyclass(static_cast<unsigned>(color.r * 255), static_cast<unsigned>(color.g * 255),
                       static_cast<unsigned>(color.b * 255));
}
