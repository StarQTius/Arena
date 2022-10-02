#pragma once

#include <pybind11/pybind11.h>

void initialize_base(pybind11::module_ &);
void initialize_sail_the_world(pybind11::module_ &);
void initialize_the_cherry_on_the_cake(pybind11::module_ &&);
