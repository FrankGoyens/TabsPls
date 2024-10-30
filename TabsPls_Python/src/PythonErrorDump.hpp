#pragma once

#include <string>

#include "PythonRAII.hpp"

namespace PythonErrorDump {
std::string FormatPythonException(PyObject* exceptionType, PyObject* exceptionValue);
}