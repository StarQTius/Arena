#pragma once

#include <memory>

#include <box2d/b2_body.h>
#include <box2d/b2_world.h>

namespace component {
namespace detail {

// Deleters for Box2D objects
// WARNING : be careful to delete any 'b2World' instance AFTER deleting the Box2D objects managed by that instance
inline auto delete_body = [](b2Body *body_ptr) { body_ptr->GetWorld()->DestroyBody(body_ptr); };

} // namespace detail

// Thin wrapper around b2Body which allows automatic destruction
using BodyPtr = std::unique_ptr<b2Body, decltype(detail::delete_body)>;

} // namespace component
