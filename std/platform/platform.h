#ifndef BASE_PLATFORM_H
#define BASE_PLATFORM_H

/* ===========================
   Compiler Detection
   =========================== */
#if defined(__clang__)
#        define BASE_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#        define BASE_COMPILER_GCC
#elif defined(_MSC_VER)
#        define BASE_COMPILER_MSVC
#else
#        define BASE_COMPILER_UNKNOWN
#endif

/* ===========================
   OS Detection
   =========================== */
#if defined(_WIN32) || defined(_WIN64)
#        define BASE_OS_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
#        define BASE_OS_MACOS
#        include <TargetConditionals.h>
#        if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#                define BASE_OS_IOS
#        endif
#elif defined(__linux__)
#        define BASE_OS_LINUX
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#        define BASE_OS_FREEBSD
#elif defined(__NetBSD__)
#        define BASE_OS_NETBSD
#elif defined(__OpenBSD__)
#        define BASE_OS_OPENBSD
#elif defined(__sun) && defined(__SVR4)
#        define BASE_OS_ILLUMOS /* illumos or Solaris */
#elif defined(__unix__) || defined(__unix)
#        define BASE_OS_UNIX /* Fallback generic Unix */
#else
#        define BASE_OS_UNKNOWN
#endif

/* ===========================
   Architecture Detection
   =========================== */
#if defined(_M_X64) || defined(__x86_64__)
#        define BASE_ARCH_X86_64
#elif defined(_M_IX86) || defined(__i386__)
#        define BASE_ARCH_X86_32
#elif defined(__aarch64__) || defined(_M_ARM64)
#        define BASE_ARCH_ARM64
#elif defined(__arm__) || defined(_M_ARM)
#        define BASE_ARCH_ARM32
#else
#        define BASE_ARCH_UNKNOWN
#endif

#endif /* BASE_PLATFORM_H */
