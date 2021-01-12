#include <QApplication>

#include <TabsPlsCore/FileSystem.hpp>

#include "FileSystemDefsConversion.hpp"
#include "TabsPlsMainWindow.hpp"

int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    TabsPlsMainWindow mainWindow(FileSystem::StringConversion::FromRawPath(FileSystem::GetWorkingDirectory()));
    mainWindow.setMinimumWidth(600);
    mainWindow.setMinimumHeight(480);

    mainWindow.show();

    return app.exec();
}
