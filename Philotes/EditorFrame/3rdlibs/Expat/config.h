/*================================================================

** Copyright 2000, Clark Cooper

** All rights reserved.

**

** This is free software. You are permitted to copy, distribute, or modify

** it under the terms of the MIT/X license (contained in the COPYING file

** with this distribution.)

**

**

*/



#ifndef WINCONFIG_H

#define WINCONFIG_H


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif


#if !defined(PS3)
	#include <memory.h>
#endif
#include <string.h>


#define XMLPARSEAPI(type) type
#define VERSION "1.95.2"

#define XML_NS 1

//#define XML_DTD 1

#define XML_BYTE_ORDER 12

#define XML_CONTEXT_BYTES 1024



#endif /* ndef WINCONFIG_H */