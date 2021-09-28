#include <WinIconExtract.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <Shlwapi.h>
#include <Windows.h>
#include <gdiplus.h>

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
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

static void DumpBitmap(HICON hIcon, const WinIconExtract::IconDumper& dumper) {
    auto* image = new Gdiplus::Bitmap(hIcon);

    CLSID myClsId;
    int retVal = GetEncoderClsid(L"image/bmp", &myClsId);

    IStream* stream = SHCreateMemStream(NULL, 0);
    image->Save(stream, &myClsId, NULL);

    stream->Seek(LARGE_INTEGER{0}, STREAM_SEEK_SET, NULL);
    std::vector<unsigned char> data;

    constexpr ULONG bytes_to_read = 512;
    ULONG bytesread = 0;
    do {
        unsigned char buffer[bytes_to_read];
        stream->Read((void*)buffer, bytes_to_read, &bytesread);
        data.insert(data.end(), std::begin(buffer), std::begin(buffer) + bytesread);
    } while (bytesread == bytes_to_read);

    dumper.Dump(data, image->GetWidth(), image->GetHeight());

    delete image;
}

static void DumpPNGIcon(HGLOBAL res, LPSTR icon_name, DWORD size) {
    auto bitmap_pointer = LockResource(res);
    std::ofstream outputFile("icon_" + std::to_string((int)icon_name) + std::string(".png"),
                             std::ios::out | std::ios::binary);
    outputFile.write((char*)bitmap_pointer, size);
}

static BOOL resourceCallback(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam) {
    auto& icon_names = *reinterpret_cast<std::vector<std::pair<LPCWSTR, LPWSTR>>*>(lParam);
    icon_names.emplace_back(lpType, lpName);
    return TRUE;
}

namespace WinIconExtract {
static ULONG_PTR gdiplusToken;
void Init() {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}
void DeInit() { Gdiplus::GdiplusShutdown(gdiplusToken); }

void DumpAssociatedIconInfo(const std::wstring& path, const IconDumper& dumper) {
    WORD index = 0;
    auto hIcon = ExtractAssociatedIconW(0, (LPWSTR)path.c_str(), &index);
    if (hIcon != 0) {
        DumpBitmap(hIcon, dumper);
        DestroyIcon(hIcon);
    }
}

void DumpIconInfo(const std::wstring& path, const IconDumper& dumper) {
    auto hlib = LoadLibraryW(static_cast<LPCWSTR>(path.c_str()));
    std::vector<std::pair<LPCWSTR, LPWSTR>> icon_names;
    BOOL success = EnumResourceNamesW(hlib, (LPWSTR)RT_ICON, resourceCallback, reinterpret_cast<LONG_PTR>(&icon_names));
    if (!success)
        std::cout << "Enumerating names went wrong" << std::endl;

    std::unique_ptr<unsigned char> bitmap_data;

    for (auto& icon : icon_names) {
        auto resInfo = FindResourceW(hlib, icon.second, (LPWSTR)RT_ICON);
        if (resInfo != 0) {
            auto size = SizeofResource(hlib, resInfo);
            auto res = LoadResource(hlib, resInfo);
            if (res != 0) {
                auto hIcon = CreateIconFromResource((PBYTE)res, size, TRUE, 0x00030000);
                if (hIcon != 0) {
                    DumpBitmap(hIcon, dumper);
                    DestroyIcon(hIcon);
                }
            }
        }
    }
}
} // namespace WinIconExtract
