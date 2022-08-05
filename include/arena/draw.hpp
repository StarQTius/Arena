#pragma once

#include <box2d/b2_draw.h>
#include <box2d/b2_math.h>
#include <box2d/b2_types.h>
#include <pybind11/pytypes.h>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/physics.hpp>

namespace arena {

class PyGameDrawer : public b2Draw {
public:
  explicit PyGameDrawer(precision_t);

  operator bool() const;

  void show() const;
  void bind_screen(pybind11::object);
  void unbind_screen();

  // Overriden 'b2Draw' functions
  void DrawPoint(const b2Vec2 &, float, const b2Color &) final;
  void DrawPolygon(const b2Vec2 *vertices, int32 vertexCount, const b2Color &color) final;
  void DrawSolidPolygon(const b2Vec2 *vertices, int32 vertexCount, const b2Color &color) final;
  void DrawCircle(const b2Vec2 &center, float radius, const b2Color &color) final;
  void DrawSolidCircle(const b2Vec2 &center, float radius, const b2Vec2 &axis, const b2Color &color) final;
  void DrawSegment(const b2Vec2 &p1, const b2Vec2 &p2, const b2Color &color) final;
  void DrawTransform(const b2Transform &xf) final;

private:
  pybind11::tuple make_pygame_coordinate(const b2Vec2 &) const;
  pybind11::object make_pygame_color(const b2Color &);

  precision_t m_scale, m_offset_x, m_offset_y;

  // PyGame context functions
  pybind11::object m_screen;
  pybind11::function m_flip, m_draw_point, m_draw_polygon, m_draw_solid_polygon, m_draw_circle, m_draw_solid_circle,
      m_draw_segment, m_draw_transform;
};

} // namespace arena
