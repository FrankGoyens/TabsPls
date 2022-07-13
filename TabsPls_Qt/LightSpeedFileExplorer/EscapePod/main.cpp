#include <filesystem>
#include <iostream>
#include <string>

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " EXECUTABLE WORKING_DIR" << std::endl;
        return -1;
    }

    QApplication app(argc, argv);

    const std::string executable(argv[1]);
    const std::string workingDir(argv[2]);

    if (!std::filesystem::is_directory(workingDir)) {
        std::cerr << "Given working directory '" << workingDir << "' is not a directory" << std::endl;
        return -1;
    }

    try {
        std::filesystem::current_path(workingDir.c_str());
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(executable)));
    } catch (std::filesystem::filesystem_error& e) {
        std::cerr << "Error setting current path: " << e.what() << std::endl;
    }

    return 0;
}
