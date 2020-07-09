#pragma once

#include <FileSystemDefs.hpp>

namespace FileSystem
{
	namespace Algorithm
	{
		RawPath StripTrailingPathSeparators(RawPath);
		RawPath StripLeadingPathSeparators(RawPath);
	}
}