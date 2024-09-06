













#ifndef CEREAL_RAPIDJSON_CEREAL_RAPIDJSON_H_
#define CEREAL_RAPIDJSON_CEREAL_RAPIDJSON_H_





#include <cstdlib>  
#include <cstring>  









#define CEREAL_RAPIDJSON_STRINGIFY(x) CEREAL_RAPIDJSON_DO_STRINGIFY(x)
#define CEREAL_RAPIDJSON_DO_STRINGIFY(x) #x


#define CEREAL_RAPIDJSON_JOIN(X, Y) CEREAL_RAPIDJSON_DO_JOIN(X, Y)
#define CEREAL_RAPIDJSON_DO_JOIN(X, Y) CEREAL_RAPIDJSON_DO_JOIN2(X, Y)
#define CEREAL_RAPIDJSON_DO_JOIN2(X, Y) X##Y






#define CEREAL_RAPIDJSON_MAJOR_VERSION 1
#define CEREAL_RAPIDJSON_MINOR_VERSION 1
#define CEREAL_RAPIDJSON_PATCH_VERSION 0
#define CEREAL_RAPIDJSON_VERSION_STRING \
    CEREAL_RAPIDJSON_STRINGIFY(CEREAL_RAPIDJSON_MAJOR_VERSION.CEREAL_RAPIDJSON_MINOR_VERSION.CEREAL_RAPIDJSON_PATCH_VERSION)






#ifndef CEREAL_RAPIDJSON_NAMESPACE
#define CEREAL_RAPIDJSON_NAMESPACE rapidjson
#endif
#ifndef CEREAL_RAPIDJSON_NAMESPACE_BEGIN
#define CEREAL_RAPIDJSON_NAMESPACE_BEGIN namespace CEREAL_RAPIDJSON_NAMESPACE {
#endif
#ifndef CEREAL_RAPIDJSON_NAMESPACE_END
#define CEREAL_RAPIDJSON_NAMESPACE_END }
#endif




#ifndef CEREAL_RAPIDJSON_HAS_STDSTRING
#ifdef CEREAL_RAPIDJSON_DOXYGEN_RUNNING
#define CEREAL_RAPIDJSON_HAS_STDSTRING 1 
#else
#define CEREAL_RAPIDJSON_HAS_STDSTRING 0 
#endif

#endif 

#if CEREAL_RAPIDJSON_HAS_STDSTRING
#include <string>
#endif 





#ifndef CEREAL_RAPIDJSON_NO_INT64DEFINE

#if defined(_MSC_VER) && (_MSC_VER < 1800)	
#include "msinttypes/stdint.h"
#include "msinttypes/inttypes.h"
#else

#include <stdint.h>
#include <inttypes.h>
#endif

#ifdef CEREAL_RAPIDJSON_DOXYGEN_RUNNING
#define CEREAL_RAPIDJSON_NO_INT64DEFINE
#endif
#endif 




#ifndef CEREAL_RAPIDJSON_FORCEINLINE

#if defined(_MSC_VER) && defined(NDEBUG)
#define CEREAL_RAPIDJSON_FORCEINLINE __forceinline
#elif defined(__GNUC__) && __GNUC__ >= 4 && defined(NDEBUG)
#define CEREAL_RAPIDJSON_FORCEINLINE __attribute__((always_inline))
#else
#define CEREAL_RAPIDJSON_FORCEINLINE
#endif

#endif 



#define CEREAL_RAPIDJSON_LITTLEENDIAN  0   
#define CEREAL_RAPIDJSON_BIGENDIAN     1   



#ifndef CEREAL_RAPIDJSON_ENDIAN

#  ifdef __BYTE_ORDER__
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#      define CEREAL_RAPIDJSON_ENDIAN CEREAL_RAPIDJSON_LITTLEENDIAN
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#      define CEREAL_RAPIDJSON_ENDIAN CEREAL_RAPIDJSON_BIGENDIAN
#    else
#      error Unknown machine endianness detected. User needs to define CEREAL_RAPIDJSON_ENDIAN.
#    endif 

#  elif defined(__GLIBC__)
#    include <endian.h>
#    if (__BYTE_ORDER == __LITTLE_ENDIAN)
#      define CEREAL_RAPIDJSON_ENDIAN CEREAL_RAPIDJSON_LITTLEENDIAN
#    elif (__BYTE_ORDER == __BIG_ENDIAN)
#      define CEREAL_RAPIDJSON_ENDIAN CEREAL_RAPIDJSON_BIGENDIAN
#    else
#      error Unknown machine endianness detected. User needs to define CEREAL_RAPIDJSON_ENDIAN.
#   endif 

#  elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#    define CEREAL_RAPIDJSON_ENDIAN CEREAL_RAPIDJSON_LITTLEENDIAN
#  elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#    define CEREAL_RAPIDJSON_ENDIAN CEREAL_RAPIDJSON_BIGENDIAN

#  elif defined(__sparc) || defined(__sparc__) || defined(_POWER) || defined(__powerpc__) || defined(__ppc__) || defined(__hpux) || defined(__hppa) || defined(_MIPSEB) || defined(_POWER) || defined(__s390__)
#    define CEREAL_RAPIDJSON_ENDIAN CEREAL_RAPIDJSON_BIGENDIAN
#  elif defined(__i386__) || defined(__alpha__) || defined(__ia64) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__bfin__)
#    define CEREAL_RAPIDJSON_ENDIAN CEREAL_RAPIDJSON_LITTLEENDIAN
#  elif defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
#    define CEREAL_RAPIDJSON_ENDIAN CEREAL_RAPIDJSON_LITTLEENDIAN
#  elif defined(CEREAL_RAPIDJSON_DOXYGEN_RUNNING)
#    define CEREAL_RAPIDJSON_ENDIAN
#  else
#    error Unknown machine endianness detected. User needs to define CEREAL_RAPIDJSON_ENDIAN.   
#  endif
#endif 





#ifndef CEREAL_RAPIDJSON_64BIT
#if defined(__LP64__) || (defined(__x86_64__) && defined(__ILP32__)) || defined(_WIN64) || defined(__EMSCRIPTEN__)
#define CEREAL_RAPIDJSON_64BIT 1
#else
#define CEREAL_RAPIDJSON_64BIT 0
#endif
#endif 






#ifndef CEREAL_RAPIDJSON_ALIGN
#define CEREAL_RAPIDJSON_ALIGN(x) (((x) + static_cast<size_t>(7u)) & ~static_cast<size_t>(7u))
#endif






#ifndef CEREAL_RAPIDJSON_UINT64_C2
#define CEREAL_RAPIDJSON_UINT64_C2(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))
#endif






#ifndef CEREAL_RAPIDJSON_48BITPOINTER_OPTIMIZATION
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define CEREAL_RAPIDJSON_48BITPOINTER_OPTIMIZATION 1
#else
#define CEREAL_RAPIDJSON_48BITPOINTER_OPTIMIZATION 0
#endif
#endif 

#if CEREAL_RAPIDJSON_48BITPOINTER_OPTIMIZATION == 1
#if CEREAL_RAPIDJSON_64BIT != 1
#error CEREAL_RAPIDJSON_48BITPOINTER_OPTIMIZATION can only be set to 1 when CEREAL_RAPIDJSON_64BIT=1
#endif
#define CEREAL_RAPIDJSON_SETPOINTER(type, p, x) (p = reinterpret_cast<type *>((reinterpret_cast<uintptr_t>(p) & static_cast<uintptr_t>(CEREAL_RAPIDJSON_UINT64_C2(0xFFFF0000, 0x00000000))) | reinterpret_cast<uintptr_t>(reinterpret_cast<const void*>(x))))
#define CEREAL_RAPIDJSON_GETPOINTER(type, p) (reinterpret_cast<type *>(reinterpret_cast<uintptr_t>(p) & static_cast<uintptr_t>(CEREAL_RAPIDJSON_UINT64_C2(0x0000FFFF, 0xFFFFFFFF))))
#else
#define CEREAL_RAPIDJSON_SETPOINTER(type, p, x) (p = (x))
#define CEREAL_RAPIDJSON_GETPOINTER(type, p) (p)
#endif





#if defined(CEREAL_RAPIDJSON_SSE2) || defined(CEREAL_RAPIDJSON_SSE42) \
    || defined(CEREAL_RAPIDJSON_NEON) || defined(CEREAL_RAPIDJSON_DOXYGEN_RUNNING)
#define CEREAL_RAPIDJSON_SIMD
#endif




#ifndef CEREAL_RAPIDJSON_NO_SIZETYPEDEFINE

#ifdef CEREAL_RAPIDJSON_DOXYGEN_RUNNING
#define CEREAL_RAPIDJSON_NO_SIZETYPEDEFINE
#endif
CEREAL_RAPIDJSON_NAMESPACE_BEGIN


typedef unsigned SizeType;
CEREAL_RAPIDJSON_NAMESPACE_END
#endif


CEREAL_RAPIDJSON_NAMESPACE_BEGIN
using std::size_t;
CEREAL_RAPIDJSON_NAMESPACE_END






#ifndef CEREAL_RAPIDJSON_ASSERT
#include <cassert>
#define CEREAL_RAPIDJSON_ASSERT(x) assert(x)
#endif 





#ifndef CEREAL_RAPIDJSON_STATIC_ASSERT
#if __cplusplus >= 201103L || ( defined(_MSC_VER) && _MSC_VER >= 1800 )
#define CEREAL_RAPIDJSON_STATIC_ASSERT(x) \
   static_assert(x, CEREAL_RAPIDJSON_STRINGIFY(x))
#endif 
#endif 


#ifndef CEREAL_RAPIDJSON_STATIC_ASSERT
#ifndef __clang__

#endif
CEREAL_RAPIDJSON_NAMESPACE_BEGIN
template <bool x> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
template <size_t x> struct StaticAssertTest {};
CEREAL_RAPIDJSON_NAMESPACE_END

#if defined(__GNUC__) || defined(__clang__)
#define CEREAL_RAPIDJSON_STATIC_ASSERT_UNUSED_ATTRIBUTE __attribute__((unused))
#else
#define CEREAL_RAPIDJSON_STATIC_ASSERT_UNUSED_ATTRIBUTE 
#endif
#ifndef __clang__

#endif


#define CEREAL_RAPIDJSON_STATIC_ASSERT(x) \
    typedef ::CEREAL_RAPIDJSON_NAMESPACE::StaticAssertTest< \
      sizeof(::CEREAL_RAPIDJSON_NAMESPACE::STATIC_ASSERTION_FAILURE<bool(x) >)> \
    CEREAL_RAPIDJSON_JOIN(StaticAssertTypedef, __LINE__) CEREAL_RAPIDJSON_STATIC_ASSERT_UNUSED_ATTRIBUTE
#endif 






#ifndef CEREAL_RAPIDJSON_LIKELY
#if defined(__GNUC__) || defined(__clang__)
#define CEREAL_RAPIDJSON_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define CEREAL_RAPIDJSON_LIKELY(x) (x)
#endif
#endif



#ifndef CEREAL_RAPIDJSON_UNLIKELY
#if defined(__GNUC__) || defined(__clang__)
#define CEREAL_RAPIDJSON_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CEREAL_RAPIDJSON_UNLIKELY(x) (x)
#endif
#endif






#define CEREAL_RAPIDJSON_MULTILINEMACRO_BEGIN do {  
#define CEREAL_RAPIDJSON_MULTILINEMACRO_END \
} while((void)0, 0)


#define CEREAL_RAPIDJSON_VERSION_CODE(x,y,z) \
  (((x)*100000) + ((y)*100) + (z))




#if defined(__GNUC__)
#define CEREAL_RAPIDJSON_GNUC \
    CEREAL_RAPIDJSON_VERSION_CODE(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__)
#endif

#if defined(__clang__) || (defined(CEREAL_RAPIDJSON_GNUC) && CEREAL_RAPIDJSON_GNUC >= CEREAL_RAPIDJSON_VERSION_CODE(4,2,0))

#define CEREAL_RAPIDJSON_PRAGMA(x) _Pragma(CEREAL_RAPIDJSON_STRINGIFY(x))
#define CEREAL_RAPIDJSON_DIAG_PRAGMA(x) CEREAL_RAPIDJSON_PRAGMA(GCC diagnostic x)
#define CEREAL_RAPIDJSON_DIAG_OFF(x) \
    CEREAL_RAPIDJSON_DIAG_PRAGMA(ignored CEREAL_RAPIDJSON_STRINGIFY(CEREAL_RAPIDJSON_JOIN(-W,x)))


#if defined(__clang__) || (defined(CEREAL_RAPIDJSON_GNUC) && CEREAL_RAPIDJSON_GNUC >= CEREAL_RAPIDJSON_VERSION_CODE(4,6,0))
#define CEREAL_RAPIDJSON_DIAG_PUSH CEREAL_RAPIDJSON_DIAG_PRAGMA(push)
#define CEREAL_RAPIDJSON_DIAG_POP  CEREAL_RAPIDJSON_DIAG_PRAGMA(pop)
#else 
#define CEREAL_RAPIDJSON_DIAG_PUSH 
#define CEREAL_RAPIDJSON_DIAG_POP 
#endif

#elif defined(_MSC_VER)


#define CEREAL_RAPIDJSON_PRAGMA(x) __pragma(x)
#define CEREAL_RAPIDJSON_DIAG_PRAGMA(x) CEREAL_RAPIDJSON_PRAGMA(warning(x))

#define CEREAL_RAPIDJSON_DIAG_OFF(x) CEREAL_RAPIDJSON_DIAG_PRAGMA(disable: x)
#define CEREAL_RAPIDJSON_DIAG_PUSH CEREAL_RAPIDJSON_DIAG_PRAGMA(push)
#define CEREAL_RAPIDJSON_DIAG_POP  CEREAL_RAPIDJSON_DIAG_PRAGMA(pop)

#else

#define CEREAL_RAPIDJSON_DIAG_OFF(x) 
#define CEREAL_RAPIDJSON_DIAG_PUSH   
#define CEREAL_RAPIDJSON_DIAG_POP    

#endif 




#ifndef CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
#if defined(__clang__)
#if __has_feature(cxx_rvalue_references) && \
    (defined(_MSC_VER) || defined(_LIBCPP_VERSION) || defined(__GLIBCXX__) && __GLIBCXX__ >= 20080306)
#define CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS 1
#else
#define CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS 0
#endif
#elif (defined(CEREAL_RAPIDJSON_GNUC) && (CEREAL_RAPIDJSON_GNUC >= CEREAL_RAPIDJSON_VERSION_CODE(4,3,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
      (defined(_MSC_VER) && _MSC_VER >= 1600) || \
      (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5140 && defined(__GXX_EXPERIMENTAL_CXX0X__))

#define CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS 1
#else
#define CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS 0
#endif
#endif 

#ifndef CEREAL_RAPIDJSON_HAS_CXX11_NOEXCEPT
#if defined(__clang__)
#define CEREAL_RAPIDJSON_HAS_CXX11_NOEXCEPT __has_feature(cxx_noexcept)
#elif (defined(CEREAL_RAPIDJSON_GNUC) && (CEREAL_RAPIDJSON_GNUC >= CEREAL_RAPIDJSON_VERSION_CODE(4,6,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
    (defined(_MSC_VER) && _MSC_VER >= 1900) || \
    (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5140 && defined(__GXX_EXPERIMENTAL_CXX0X__))
#define CEREAL_RAPIDJSON_HAS_CXX11_NOEXCEPT 1
#else
#define CEREAL_RAPIDJSON_HAS_CXX11_NOEXCEPT 0
#endif
#endif
#if CEREAL_RAPIDJSON_HAS_CXX11_NOEXCEPT
#define CEREAL_RAPIDJSON_NOEXCEPT noexcept
#else
#define CEREAL_RAPIDJSON_NOEXCEPT 
#endif 


#ifndef CEREAL_RAPIDJSON_HAS_CXX11_TYPETRAITS
#if (defined(_MSC_VER) && _MSC_VER >= 1700)
#define CEREAL_RAPIDJSON_HAS_CXX11_TYPETRAITS 1
#else
#define CEREAL_RAPIDJSON_HAS_CXX11_TYPETRAITS 0
#endif
#endif

#ifndef CEREAL_RAPIDJSON_HAS_CXX11_RANGE_FOR
#if defined(__clang__)
#define CEREAL_RAPIDJSON_HAS_CXX11_RANGE_FOR __has_feature(cxx_range_for)
#elif (defined(CEREAL_RAPIDJSON_GNUC) && (CEREAL_RAPIDJSON_GNUC >= CEREAL_RAPIDJSON_VERSION_CODE(4,6,0)) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
      (defined(_MSC_VER) && _MSC_VER >= 1700) || \
      (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5140 && defined(__GXX_EXPERIMENTAL_CXX0X__))
#define CEREAL_RAPIDJSON_HAS_CXX11_RANGE_FOR 1
#else
#define CEREAL_RAPIDJSON_HAS_CXX11_RANGE_FOR 0
#endif
#endif 




 




#ifndef CEREAL_RAPIDJSON_NOEXCEPT_ASSERT
#ifdef CEREAL_RAPIDJSON_ASSERT_THROWS
#if CEREAL_RAPIDJSON_HAS_CXX11_NOEXCEPT
#define CEREAL_RAPIDJSON_NOEXCEPT_ASSERT(x)
#else
#define CEREAL_RAPIDJSON_NOEXCEPT_ASSERT(x) CEREAL_RAPIDJSON_ASSERT(x)
#endif 
#else
#define CEREAL_RAPIDJSON_NOEXCEPT_ASSERT(x) CEREAL_RAPIDJSON_ASSERT(x)
#endif 
#endif 




#ifndef CEREAL_RAPIDJSON_NEW

#define CEREAL_RAPIDJSON_NEW(TypeName) new TypeName
#endif
#ifndef CEREAL_RAPIDJSON_DELETE

#define CEREAL_RAPIDJSON_DELETE(x) delete x
#endif





CEREAL_RAPIDJSON_NAMESPACE_BEGIN


enum Type {
    kNullType = 0,      
    kFalseType = 1,     
    kTrueType = 2,      
    kObjectType = 3,    
    kArrayType = 4,     
    kStringType = 5,    
    kNumberType = 6     
};

CEREAL_RAPIDJSON_NAMESPACE_END

#endif 
