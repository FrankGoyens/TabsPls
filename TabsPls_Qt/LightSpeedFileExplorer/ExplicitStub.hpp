#pragma once

#include <stdexcept>

// \brief This is thrown when a stubbed component is called. Components could be stubbed due to build options.
struct ExplicitStubException : std::exception {};