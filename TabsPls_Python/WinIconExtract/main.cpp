#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <Shlwapi.h>
#include <Windows.h>
#include <gdiplus.h>

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;  // number of image encoders
    UINT size = 0; // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1; // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1; // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j; // Success
        }
    }
    free(pImageCodecInfo);
    return -1; // Failure
}

static void DumpBitmap(HBITMAP hBitmap, LPSTR icon_name) {
    auto* image = new Gdiplus::Bitmap(hBitmap, NULL);

    CLSID myClsId;
    int retVal = GetEncoderClsid(L"image/bmp", &myClsId);

    // Yes, image->Save could directly write to a file, but later on, it is required to have the data in memory
    // For now, writing to a file is handy for debugging, so that happens anyway
    IStream* stream = SHCreateMemStream(NULL, 0);
    image->Save(stream, &myClsId, NULL);
    delete image;

    stream->Seek(LARGE_INTEGER{0}, STREAM_SEEK_SET, NULL);
    std::vector<unsigned char> data;

    constexpr ULONG bytes_to_read = 512;
    ULONG bytesread = 0;
    do {
        unsigned char buffer[bytes_to_read];
        stream->Read((void*)buffer, bytes_to_read, &bytesread);
        data.insert(data.end(), std::begin(buffer), std::end(buffer));
    } while (bytesread == bytes_to_read);

    std::ofstream outputFile("icon_" + std::to_string((int)icon_name) + std::string(".bmp"),
                             std::ios::out | std::ios::binary);
    outputFile.write((char*)data.data(), data.size());
}

static void DumpPNGIcon(HGLOBAL res, LPSTR icon_name, DWORD size) {
    auto bitmap_pointer = LockResource(res);
    std::ofstream outputFile("icon_" + std::to_string((int)icon_name) + std::string(".png"),
                             std::ios::out | std::ios::binary);
    outputFile.write((char*)bitmap_pointer, size);
}

static BOOL resourceCallback(HMODULE hModule, LPCSTR lpType, LPSTR lpName, LONG_PTR lParam) {
    auto& icon_names = *reinterpret_cast<std::vector<std::pair<LPCSTR, LPSTR>>*>(lParam);
    icon_names.emplace_back(lpType, lpName);
    return TRUE;
}

static void DumpIconInfo(const std::string& path) {
    auto hlib = LoadLibrary(static_cast<LPCSTR>(path.c_str()));
    std::vector<std::pair<LPCSTR, LPSTR>> icon_names;
    BOOL success = EnumResourceNames(hlib, RT_ICON, resourceCallback, reinterpret_cast<LONG_PTR>(&icon_names));
    if (!success)
        std::cout << "Enumerating names went wrong" << std::endl;

    std::unique_ptr<unsigned char> bitmap_data;

    for (auto& icon : icon_names) {
        auto resInfo = FindResourceA(hlib, icon.second, RT_ICON);
        if (resInfo != 0) {
            auto size = SizeofResource(hlib, resInfo);
            auto res = LoadResource(hlib, resInfo);
            if (res != 0) {
                auto hIcon = CreateIconFromResource((PBYTE)res, size, TRUE, 0x00030000);
                if (hIcon != 0) {
                    ICONINFO iconinfo;
                    GetIconInfo(hIcon, &iconinfo);
                    DumpBitmap(iconinfo.hbmColor, icon.second);
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <exe or dll path>" << std::endl;
        return 0;
    }

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    DumpIconInfo(argv[1]);

    Gdiplus::GdiplusShutdown(gdiplusToken);
}