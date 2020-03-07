#pragma once

#include <stdbool.h>
#include <sys/types.h>

// Fixed-size integral types
typedef __uint8_t Char;
typedef __int32_t Int;
typedef __uint32_t UInt;
typedef __int64_t Long;
typedef __uint64_t ULong;

// Platform-dependent integral types
typedef int ErrorCode;
typedef size_t Size;
typedef ssize_t SSize;

// Floating-point types
typedef double Double;

// Unix types (I'm pretty sure these are all going to be ints)
typedef pid_t Pid;
typedef int FileDescriptor;

// Misc. types
typedef bool Bool;
typedef void Void;