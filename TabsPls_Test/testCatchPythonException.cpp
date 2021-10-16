#include <gtest/gtest.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

namespace TabsPlsPython::Send2Trash {
std::string FormatPythonException(PyObject* exceptionType, PyObject* exceptionValue);
} // namespace TabsPlsPython::Send2Trash
namespace CatchPythonExceptionTests {
struct PythonInstance {
    PythonInstance() { Py_Initialize(); }
    ~PythonInstance() { Py_Finalize(); }
};

static PythonInstance& GetPythonInstance() {
    static PythonInstance instance;
    return instance;
}

static void RaisePythonException() {
    GetPythonInstance();
    auto* pGlobal = PyDict_New();

    auto* pModule = PyModule_New("mymod");
    PyModule_AddStringConstant(pModule, "__file__", "");

    auto* pLocal = PyModule_GetDict(pModule);

    PyRun_String("raise Exception(\"something went wrong\")", Py_file_input, pGlobal, pLocal);

    ASSERT_TRUE(PyErr_Occurred());
}

TEST(CatchPythonExceptionTest, Catch) {
    RaisePythonException();

    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);

    PyErr_NormalizeException(&type, &value, &traceback);

    auto* typeString = PyObject_Str(type);
    EXPECT_EQ(std::string("<class 'Exception'>"), std::string(PyUnicode_AsUTF8(typeString)));
    Py_XDECREF(typeString);

    auto* valueString = PyObject_Str(value);
    EXPECT_EQ(std::string("something went wrong"), std::string(PyUnicode_AsUTF8(valueString)));
    Py_XDECREF(valueString);

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);

    PyErr_Clear();
}

TEST(CatchPythonExceptionTest, Send2Trash_FormatPythonException) {
    RaisePythonException();

    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    PyErr_NormalizeException(&type, &value, &traceback);

    EXPECT_EQ(std::string("<class 'Exception'>: something went wrong"),
              TabsPlsPython::Send2Trash::FormatPythonException(type, value));

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);

    PyErr_Clear();
}
} // namespace CatchPythonExceptionTests