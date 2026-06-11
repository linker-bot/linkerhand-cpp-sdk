#ifndef LINKER_HAND_EXPORT_H
#define LINKER_HAND_EXPORT_H

#if defined(_WIN32) || defined(_WIN64)
    #if defined(LINKERHAND_EXPORTS)
        #define LINKERHAND_API __declspec(dllexport)
    #else
        #define LINKERHAND_API __declspec(dllimport)
    #endif
#else
    #define LINKERHAND_API
#endif

#endif // LINKER_HAND_EXPORT_H
