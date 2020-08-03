#pragma once

#include <QString>

#include "FileSystemDefs.hpp"

namespace FileSystem
{
    //Convert QStrings from and to internal string types
    //Internal string types are those defines in FileSystemDefs.hpp
    namespace StringConversionImpl
    {
        QString FromName(const Name&);
        QString FromRawPath(const RawPath&);

        Name ToName(const QString&);
        RawPath ToRawPath(const QString&);
    }
}