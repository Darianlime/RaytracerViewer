#include <pybind11/pybind11.h>

namespace py = pybind11;

// example function
int add(int a, int b) {
    return a + b;
}

PYBIND11_MODULE(raytracer_bindings, m) {
	m.doc() = "pybind11 example plugin"; // optional module docstring
    m.def("add", &add);
}