/********************************************************************
       Copyright (c) 2006, Lee Patterson
       http://ssobjects.sourceforge.net

       filename :  msdefs.h
       author   :  Lee Patterson (workerant@users.sourceforge.net)
*********************************************************************/

#ifndef MS_DEFS
#define MS_DEFS

// ---------------------------------------------------------
// FIX: Resolve 'byte' ambiguity for MinGW / C++17
// ---------------------------------------------------------
#ifdef WIN32
    // By defining 'byte', we prevent Windows headers from 
    // using the name 'byte' which conflicts with 'std::byte'.
    #ifndef byte
        #define byte win_byte_override
    #endif
    #include <wtypes.h>
#endif

#ifdef DEBUG
# ifndef _DEBUG
#   define _DEBUG
# endif
# define __DEBUG
#endif

#ifndef WIN32
#include <sys/types.h>
#endif

// Constants needed to be compatible with MSVC compiler
#ifndef _DWORD_DEFINED
#define _WORD_DEFINED
#define _BYTE_DEFINED
#define _DWORD_DEFINED
typedef unsigned short WORD;
typedef unsigned char  BYTE;
typedef unsigned long  DWORD;
typedef unsigned int   UINT;
typedef int            BOOL;
#endif //!_WORD_DEFINED


#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif

#ifdef WIN32
// FIX: Prevent "redefined" warnings by checking if already 
// defined by the MinGW string.h header.
# ifndef strcasecmp
#  define strcasecmp _stricmp
# endif
# ifndef strncasecmp
#  define strncasecmp _strnicmp
# endif
#else
 typedef const char* LPCSTR;
 typedef char* LPSTR;
#endif

//MAXPATHLEN = 1024 (I didn't want to have to include glib.h)
#ifndef WIN32
# ifndef _MAX_PATH
#   define _MAX_PATH 1024
# endif
#endif

#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif

#endif //MS_DEFS