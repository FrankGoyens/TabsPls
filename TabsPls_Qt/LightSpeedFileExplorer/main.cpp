#include <QApplication>

#include <TabsPlsCore/EmbeddedPython.hpp>
#include <TabsPlsCore/FileSystem.hpp>
#include <TabsPlsCore/Send2Trash.hpp>

#include "FileSystemDefsConversion.hpp"
#include "TabsPlsMainWindow.hpp"

int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    if (TabsPlsPython::EmbeddedPython::ComponentIsAvailable())
        TabsPlsPython::EmbeddedPython::Init(argv[0]);

    if (TabsPlsPython::Send2Trash::ComponentIsAvailable())
        TabsPlsPython::Send2Trash::Init(argv[0]);

    TabsPlsMainWindow mainWindow(FileSystem::StringConversion::FromRawPath(FileSystem::GetWorkingDirectory()));
    mainWindow.setMinimumWidth(600);
    mainWindow.setMinimumHeight(480);

    mainWindow.show();

    return app.exec();
}
