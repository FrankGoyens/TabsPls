#include "WindowsNativeContextMenu.hpp"

#include <algorithm>
#include <memory>

#include <ShlObj_core.h>
#include <shtypes.h>
#include <windows.h>

namespace WindowsNativeContextMenu {

static void ReleaseIdList(ITEMIDLIST* idList) {

    if (idList)
        CoTaskMemFree(idList);
}

static void ReleaseComInterface(IUnknown* i) {
    if (i)
        i->Release();
}

using ItemIDListUniquePtr = std::unique_ptr<ITEMIDLIST, decltype(&ReleaseIdList)>;
using IShellFolderUniquePtr = std::unique_ptr<IShellFolder, decltype(&ReleaseComInterface)>;

struct ScopedItemIDList {
    ItemIDListUniquePtr itemIDList{nullptr, &ReleaseIdList};
    LPCITEMIDLIST childIDItemList{};
    IShellFolderUniquePtr ifolder{nullptr, &ReleaseComInterface};
};

static ScopedItemIDList ToShellFolderWithItemIDLists(const std::wstring& path) {
    ITEMIDLIST* id = 0;
    std::wstring windowsPath = path;
    std::replace(windowsPath.begin(), windowsPath.end(), '/', '\\');
    HRESULT result = SHParseDisplayName(windowsPath.c_str(), 0, &id, 0, 0);
    if (!SUCCEEDED(result) || !id)
        return ScopedItemIDList{};
    std::unique_ptr<ITEMIDLIST, decltype(&ReleaseIdList)> scopedID(id, &ReleaseIdList);

    IShellFolder* ifolder = 0;

    LPCITEMIDLIST idChild = 0;
    result = SHBindToParent(id, IID_IShellFolder, (void**)&ifolder, &idChild);
    if (!SUCCEEDED(result) || !ifolder)
        return ScopedItemIDList{};

    IShellFolderUniquePtr scopedFolder(ifolder, &ReleaseComInterface);
    return {std::move(scopedID), idChild, std::move(scopedFolder)};
}

bool ShowContextMenuForItem(const std::wstring& path, int xPos, int yPos, void* parentWindow) {
    if (!parentWindow)
        return false;

    const auto [itemIDList, childIDItemList, ifolder] = ToShellFolderWithItemIDLists(path);

    IContextMenu* imenu = 0;
    HRESULT result = result = ifolder->GetUIObjectOf((HWND)parentWindow, 1, (const ITEMIDLIST**)&childIDItemList,
                                                     IID_IContextMenu, 0, (void**)&imenu);
    if (!SUCCEEDED(result) || !imenu)
        return false;
    std::unique_ptr<IContextMenu, decltype(&ReleaseComInterface)> scopedMenu(imenu, &ReleaseComInterface);

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu)
        return false;
    if (SUCCEEDED(imenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_NORMAL))) {
        int iCmd = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, xPos, yPos, (HWND)parentWindow, NULL);
        if (iCmd > 0) {
            CMINVOKECOMMANDINFOEX info = {0};
            info.cbSize = sizeof(info);
            info.fMask = CMIC_MASK_UNICODE;
            info.hwnd = (HWND)parentWindow;
            info.lpVerb = MAKEINTRESOURCEA(iCmd - 1);
            info.lpVerbW = MAKEINTRESOURCEW(iCmd - 1);
            info.nShow = SW_SHOWNORMAL;
            imenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
        }
    }
    DestroyMenu(hMenu);

    return true;
}
} // namespace WindowsNativeContextMenu