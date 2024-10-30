#include "PythonErrorDump.hpp"

#include <sstream>

namespace PythonErrorDump {
std::string FormatPythonException(PyObject* exceptionType, PyObject* exceptionValue) {
    if (exceptionType && exceptionValue) {
        AcquiredPyObject typeString, valueString;
        typeString.obj = PyObject_Str(exceptionType);
        valueString.obj = PyObject_Str(exceptionValue);
        std::ostringstream message;
        message << PyUnicode_AsUTF8(typeString.obj) << ": " << PyUnicode_AsUTF8(valueString.obj);
        return message.str();
    }
    return "";
}
} // namespace PythonErrorDump