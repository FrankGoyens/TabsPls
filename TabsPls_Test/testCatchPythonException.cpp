#include <gtest/gtest.h>

#include "../TabsPls_Python/src/PythonErrorDump.hpp"

namespace ToolbarInternal {
std::vector<std::string> FormatPythonExceptionWithBacktrace();
} // namespace ToolbarInternal

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
              PythonErrorDump::FormatPythonException(type, value));

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);

    PyErr_Clear();
}

TEST(CatchPythonExceptionTest, ToolbarInternal_FormatPythonExceptionWithBacktrace) {
    RaisePythonException();

    auto lines = ToolbarInternal::FormatPythonExceptionWithBacktrace();

    ASSERT_EQ(3u, lines.size());

    EXPECT_EQ("Traceback (most recent call last):\n", lines[0]);
    EXPECT_EQ("  File \"<string>\", line 1, in <module>\n", lines[1]);
    EXPECT_EQ("Exception: something went wrong\n", lines[2]);

    PyErr_Clear();
}

TEST(CatchPythonExceptionTest, ToolbarInternal_FormatPythonExceptionWithBacktrace_NoError) {
    auto lines = ToolbarInternal::FormatPythonExceptionWithBacktrace();

    EXPECT_TRUE(lines.empty());

    PyErr_Clear();
}

TEST(CatchPythonExceptionTest, ToolbarInternal_FormatPythonExceptionWithBacktrace_ErrorPersists) {
    RaisePythonException();

    auto lines = ToolbarInternal::FormatPythonExceptionWithBacktrace();

    ASSERT_EQ(3u, lines.size());

    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    ASSERT_TRUE(type && value && traceback);

    PyErr_Clear();
}
} // namespace CatchPythonExceptionTests