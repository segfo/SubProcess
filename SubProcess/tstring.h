#pragma once
#include<iostream>
#include<tchar.h>

#ifdef __linux__
typedef TCHAR char;
#endif
typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > tstring;