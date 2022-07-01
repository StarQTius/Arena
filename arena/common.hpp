#pragma once

#include <string>

#include <entt/entity/entity.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <arena/binding/fetcher.hpp>
#include <arena/environment.hpp>

#define ARENA_MODULE_NAME "arena"

void register_fetcher(const std::string &, const arena::Fetcher &);
const arena::FetcherMap &get_fetchers();
pybind11::object fetch_component(arena::Environment &, entt::entity, pybind11::object);

void initialize_base(pybind11::module_ &);
void initialize_c21(pybind11::module_ &);
