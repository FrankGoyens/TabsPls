#pragma once

#include <QtDebug>

// Alas, these are macros, but we need to also log the callsite's file and lineno

// Log an error that is supposed to be helpful for developers to fix the problem
#define TabsPlsLog_Debug(message) (qDebug() << message)