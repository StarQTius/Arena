#pragma once

#include <pybind11/pybind11.h>

#define ARENA_MODULE_NAME "arena"

void initialize_base(pybind11::module_ &);
void initialize_sail_the_world(pybind11::module_ &);
