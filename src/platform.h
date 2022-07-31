#ifndef PLATFORM_H__HEADER_GUARD__
#define PLATFORM_H__HEADER_GUARD__

/* Windows */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#	define PLATFORM_WINDOWS

/* Apple */
#elif defined(__APPLE__)
#	define PLATFORM_APPLE

/* Linux */
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define PLATFORM_LINUX

/* Unix */
#elif defined(__unix__) || defined(unix)
#	define PLATFORM_UNIX

/* Unknown */
#else
#	define PLATFORM_UNKNOWN
#endif

#ifdef PLATFORM_UNKNOWN
#	error "Unknown platform"
#elif defined(PLATFORM_WINDOWS)
#	error "Windows is not supported for now"
#elif defined(PLATFORM_APPLE)
#	define PLATFORM_LINUX
#elif defined(PLATFORM_UNIX)
#	define PLATFORM_LINUX
#endif

#endif
