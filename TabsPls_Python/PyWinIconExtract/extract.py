import win32gui, win32api, win32con
import multiprocessing


# ctypes configuring. pywin32 has no a lot of required functions
import ctypes
import ctypes.util

libc = ctypes.cdll.msvcrt
libc.memcpy.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t]
libc.memcpy.restype = ctypes.c_char_p

def DumpIconInfo(path):
    data = []
    try:
        hlib = win32api.LoadLibrary(path)
        icon_names = win32api.EnumResourceNames(hlib, win32con.RT_ICON)

        for icon_name in icon_names:
            hlib_c = ctypes.cast(hlib, ctypes.wintypes.HMODULE)
            hResInfo = ctypes.windll.kernel32.FindResourceW(hlib_c, icon_name, win32con.RT_ICON)
            size = ctypes.windll.kernel32.SizeofResource(hlib_c, hResInfo)

            rec = win32api.LoadResource(hlib, win32con.RT_ICON, icon_name)
            hicon = win32gui.CreateIconFromResource(rec, True)
            info = win32gui.GetIconInfo(hicon)
            bminfo = win32gui.GetObject(info[3])
            data.append("%2d: 0x%08X -> %d %d " % (icon_name, hicon, bminfo.bmWidth, bminfo.bmHeight))
            bitmap_pointer = ctypes.windll.kernel32.LockResource(rec)

            py_binary_data = (ctypes.c_ubyte * size)()
            libc.memcpy(py_binary_data, bitmap_pointer, size)
            win32gui.DestroyIcon(hicon)

        win32api.FreeLibrary(hlib)
    except win32api.error:
        data.append("Problem retrieving icons: {}".format(path))
    return data

if __name__ == "__main__":
    paths = ["C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe",
        "C:\\Program Files (x86)\\Notepad++\\notepad++.exe",
        "C:\\Program Files (x86)\\ownCloud\\owncloud.exe",
        "C:\\Program Files\\7-Zip\\7zFM.exe"]

    data = DumpIconInfo(paths[2])

    # pool = multiprocessing.Pool()
    # data = pool.map(DumpIconInfo, paths)

    print(data)