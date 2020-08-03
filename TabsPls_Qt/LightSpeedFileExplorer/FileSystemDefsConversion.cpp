#include "FileSystemDefsConversion.hpp"

namespace FileSystem
{
    namespace StringConversionImpl
    {
        QString FromName(const Name& name)
        {
            return QString::fromStdWString(name);
        }

        QString FromRawPath(const RawPath& rawPath)
        {
            return FromName(rawPath);
        }

        Name ToName(const QString& qstring)
        {
            return qstring.toStdWString();
        }
        
        RawPath ToRawPath(const QString& qstring)
        {
            return qstring.toStdWString();
        }
    }
}