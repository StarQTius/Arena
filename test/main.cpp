#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>

#include <pybind11/embed.h>

namespace py = pybind11;

int main(int argc, char **argv) {
  py::scoped_interpreter guard;
  return Catch::Session().run(argc, argv);
}
