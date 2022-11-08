#include <TabsPlsCore/EmbeddedPython.hpp>

#include <sstream>

#include "PythonRAII.hpp"

namespace TabsPlsPython::EmbeddedPython {
bool ComponentIsAvailable() { return true; }

struct InitializePyInstanceWithEmbeddedPath {
    InitializePyInstanceWithEmbeddedPath(const char* programName) {
        InitializePyInstance(programName);

        std::ostringstream oss;
        oss << "import sys, os " << std::endl
            << R"(sys.path.append(os.path.join(os.path.dirname(r")" << programName
            << R"("), "tabspls_embedded_python")))" << std::endl;

        PyRun_SimpleString(oss.str().c_str());
    }
};

void Init(const char* programName) { static InitializePyInstanceWithEmbeddedPath _(programName); }
} // namespace TabsPlsPython::EmbeddedPython