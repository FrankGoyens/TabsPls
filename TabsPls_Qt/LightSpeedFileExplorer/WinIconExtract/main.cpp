#include <fstream>
#include <iostream>

#include <WinIconExtract.hpp>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <exe or dll path>" << std::endl;
        return 0;
    }

    WinIconExtract::Init();

    struct FileDumper : WinIconExtract::IconDumper {
        void Dump(const std::vector<unsigned char>& data, int, int) const override {
            std::ofstream out(std::string("icon_") + std::to_string(++index) + std::string(".bmp"),
                              std::ios::out | std::ios::binary);
            out.write((char*)data.data(), data.size());
        }

        mutable int index = 0;
    };

    WinIconExtract::DumpIconInfo(argv[1], FileDumper{});

    WinIconExtract::DeInit();
}