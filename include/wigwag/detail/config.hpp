#ifndef WIGWAG_DETAIL_CONFIG_HPP
#define WIGWAG_DETAIL_CONFIG_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <exception>

#include <stdio.h>


#if defined(__GNUC__) || defined(__clang)
#	if !defined(__EXCEPTIONS) && !defined(WIGWAG_NOEXCEPTIONS)
#		define WIGWAG_NOEXCEPTIONS 1
#	endif

#	if !defined(WIGWAG_PLATFORM_POSIX)
#		define WIGWAG_PLATFORM_POSIX 1
#	endif
#endif

#if defined(__GNUC__) || defined(__clang)
#	define WIGWAG_EXPECT(A_, B_) __builtin_expect((long)(A_), (long)(B_))
#endif

#if defined(_MSC_VER)
#	if !defined(_CPPUNWIND) && !defined(WIGWAG_NOEXCEPTIONS)
#		define WIGWAG_NOEXCEPTIONS 1
#	endif

#	if !defined(WIGWAG_PLATFORM_WINDOWS)
#		define WIGWAG_PLATFORM_WINDOWS 1
#	endif
#endif


#if !defined(WIGWAG_DEBUG)
#	if defined(NDEBUG)
#		define WIGWAG_DEBUG 0
#	else
#		define WIGWAG_DEBUG 1
#	endif
#endif


#if !defined(WIGWAG_NOEXCEPTIONS)
#	define WIGWAG_NOEXCEPTIONS 0
#endif

#if !defined(WIGWAG_PLATFORM_POSIX)
#	define WIGWAG_PLATFORM_POSIX 0
#endif

#if !defined(WIGWAG_PLATFORM_WINDOWS)
#	define WIGWAG_PLATFORM_WINDOWS 0
#endif


#if !defined(WIGWAG_EXPECT)
#	define WIGWAG_EXPECT(A_, B_) (A_)
#endif


#if !defined(WIGWAG_THROW)
#	if WIGWAG_NOEXCEPTIONS
#		define WIGWAG_THROW(Msg_) do { fprintf(stderr, "WIGWAG_THROW: %s\nFile: %s:%d\nFunction: %s\n", Msg_, __FILE__, __LINE__, __func__); std::terminate(); } while (0)
#	else
#		define WIGWAG_THROW(Msg_) throw std::runtime_error(Msg_)
#	endif
#endif

#if !defined(WIGWAG_ASSERT)
#	if WIGWAG_DEBUG
#		define WIGWAG_ASSERT(Expr_, Msg_) do { if (!(Expr_)) { fprintf(stderr, "WIGWAG_ASSERT: %s\nFile: %s:%d\nFunction: %s\n", Msg_, __FILE__, __LINE__, __func__); std::terminate(); } } while (0)
#	else
#		define WIGWAG_ASSERT(...) do { } while (0)
#	endif
#endif

#if defined(_MSC_VER) && (WIGWAG_NOEXCEPTIONS || _MSC_VER < 1900)
#	define WIGWAG_NOEXCEPT
#else
#	define WIGWAG_NOEXCEPT noexcept
#endif

#if defined(_MSC_VER)
#	define WIGWAG_HAS_UNRESTRICTED_UNIONS (_MSC_VER >= 1900)
#else
#	define WIGWAG_HAS_UNRESTRICTED_UNIONS 1
#endif


#endif
