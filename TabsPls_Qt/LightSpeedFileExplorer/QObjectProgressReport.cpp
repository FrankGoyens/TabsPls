#include "QObjectProgressReport.hpp"

void QObjectProgressReport::UpdateValue(int value) { emit Updated(value); }

void QObjectProgressReport::SetMinimum(int minimum) { emit MinimumSet(minimum); }

void QObjectProgressReport::SetMaximum(int maximum) { emit MaximumSet(maximum); }
