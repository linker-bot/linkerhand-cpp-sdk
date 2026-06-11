// 共享头：Windows 上把控制台输入/输出码页切到 UTF-8。
// 源码里的中文字符串是 UTF-8（mingw/clang 默认；MSVC 由 /utf-8 保证），
// 控制台默认 GBK 会把 cout/cerr 的中文输出按错误码页解读 → 乱码，连带
// 让 catch 块里的中文报错肉眼无法识别。
// 通过匿名 namespace 静态对象在 main() 进入前完成初始化，example 只需 include 即可。
#pragma once
#ifdef _WIN32
#include <windows.h>
namespace {
struct _LhConsoleUtf8Init {
    _LhConsoleUtf8Init() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
} _lh_console_utf8_init;
}
#endif
