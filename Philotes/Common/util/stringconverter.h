#pragma once
//------------------------------------------------------------------------------
/**
    @class Win32::Win32StringConverter
    
    Convert between UTF-8 and 16-bit wide strings.
    
    (C) 2009 Radon Labs GmbH
*/
#include "util/string.h"

//------------------------------------------------------------------------------
namespace Philo
{
class StringConverter
{
public:
    /// convert from UTF-8 encoded string object to wide string, return number of used bytes
    static SizeT UTF8ToWide(const String& src, ushort* dst, SizeT dstMaxBytes);
    /// convert from UTF-8 raw string to wide string, return number of used bytes
    static SizeT UTF8ToWide(const char* src, ushort* dst, SizeT dstMaxBytes);
    /// convert from wide string to UTF-8 string
    static String WideToUTF8(ushort* src);
};

} // namespace Win32
//------------------------------------------------------------------------------
    