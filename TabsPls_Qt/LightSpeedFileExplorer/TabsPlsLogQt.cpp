#include "TabsPlsLogQt.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <cstdarg>
#include <cstdio>

#include <QDebug>
#include <QMessageLogger>

constexpr const char* logFile = "log.txt";
static bool logFileWritable = true;

namespace TabsPlsLog {

void WriteLogStatementToStream(std::ostream& out, const char* logLevelName, const char* formattedMessage,
                               const char* file, int lineno, const char* function) {
    out << logLevelName << ": " << formattedMessage << " (" << file << ":" << lineno << ", " << function << ")"
        << std::endl;
}

std::vector<char> WriteFormattedMessageToBuffer(const char* fmt, va_list args) {
    int requiredBufferSize = std::vsnprintf(nullptr, 0, fmt, args);

    std::vector<char> buffer(requiredBufferSize + 1, 0);
    (void)std::vsnprintf(buffer.data(), buffer.size(), fmt, args);

    return buffer;
}

void WriteLogStatement(const char* logLevelName, const char* file, int lineno, const char* function, const char* fmt,
                       va_list args) {
    const auto formattedMessage = WriteFormattedMessageToBuffer(fmt, args);

    if (logFileWritable) {
        std::ofstream logFileOstream(logFile, std::ios::out | std::ios::app);
        if (logFileOstream.is_open())
            WriteLogStatementToStream(logFileOstream, logLevelName, formattedMessage.data(), file, lineno, function);
    }
    WriteLogStatementToStream(std::cerr, logLevelName, formattedMessage.data(), file, lineno, function);
}

//! \brief A very simple log writing function that appends to the log file and to stderr,
//! if more advanced functionality is needed an external logging library should be used
void WriteLogStatement(const char* logLevelName, const char* file, int lineno, const char* function, const char* fmt,
                       ...) {
    va_list args{};
    va_start(args, fmt);
    WriteLogStatement(logLevelName, file, lineno, function, fmt, args);
    va_end(args);
}

void QtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        WriteLogStatement("Debug", file, context.line, function, localMsg.constData());
        break;
    case QtInfoMsg:
        WriteLogStatement("Info", file, context.line, function, localMsg.constData());
        break;
    case QtWarningMsg:
        WriteLogStatement("Warning", file, context.line, function, localMsg.constData());
        break;
    case QtCriticalMsg:
        WriteLogStatement("Critical", file, context.line, function, localMsg.constData());
        break;
    case QtFatalMsg:
        WriteLogStatement("Fatal", file, context.line, function, localMsg.constData());
        abort();
        break;
    }
}

void Init() {
    // Shouldn't use TabsPlsCore::FileSystem to check for existance, as that component might use this logging component
    if (!std::filesystem::exists(logFile)) {
        std::ofstream logFileOutStream(logFile);
        logFileWritable = logFileOutStream.is_open();
    }

    (void)qInstallMessageHandler(&QtMessageHandler);
}

void Debug(const char* fileName, int lineno, const char* function, const char* category, const char* fmt, ...) {
    va_list args{};
    va_start(args, fmt);
    WriteLogStatement("Debug", fileName, lineno, function, fmt, args);
    va_end(args);
}

void Warning(const char* fileName, int lineno, const char* function, const char* category, const char* fmt, ...) {
    va_list args{};
    va_start(args, fmt);
    WriteLogStatement("Warning", fileName, lineno, function, fmt, args);
    va_end(args);
}

void Error(const char* fileName, int lineno, const char* function, const char* category, const char* fmt, ...) {
    va_list args{};
    va_start(args, fmt);
    WriteLogStatement("Error", fileName, lineno, function, fmt, args);
    va_end(args);
}
} // namespace TabsPlsLog