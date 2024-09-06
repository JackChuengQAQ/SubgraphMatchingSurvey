#if !defined(spp_config_h_guard)
#define spp_config_h_guard





#ifndef SPP_NAMESPACE
     #define SPP_NAMESPACE spp
#endif

#ifndef spp_
    #define spp_ SPP_NAMESPACE
#endif

#ifndef SPP_DEFAULT_ALLOCATOR
    #if (defined(SPP_USE_SPP_ALLOC) && SPP_USE_SPP_ALLOC) && defined(_MSC_VER)
        
        
        
        
        
        
        
        #define SPP_DEFAULT_ALLOCATOR spp_::spp_allocator
        #define SPP_INCLUDE_SPP_ALLOC
    #else
        #define SPP_DEFAULT_ALLOCATOR spp_::libc_allocator
    #endif
#endif

#ifndef SPP_GROUP_SIZE
    
    #define SPP_GROUP_SIZE 32
#endif

#ifndef SPP_ALLOC_SZ
    
    #define SPP_ALLOC_SZ 0
#endif

#ifndef SPP_STORE_NUM_ITEMS
    
    #define SPP_STORE_NUM_ITEMS 1 
#endif

































#if defined __clang__

    #if defined(i386)
        #include <cpuid.h>
        inline void spp_cpuid(int info[4], int InfoType) {
            __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
        }
    #endif

    #define SPP_POPCNT   __builtin_popcount
    #define SPP_POPCNT64 __builtin_popcountll

    #define SPP_HAS_CSTDINT

    #ifndef __has_extension
        #define __has_extension __has_feature
    #endif

    #if !__has_feature(cxx_exceptions) && !defined(SPP_NO_EXCEPTIONS)
        #define SPP_NO_EXCEPTIONS
    #endif

    #if !__has_feature(cxx_rtti) && !defined(SPP_NO_RTTI)
      #define SPP_NO_RTTI
    #endif

    #if !__has_feature(cxx_rtti) && !defined(SPP_NO_TYPEID)
        #define SPP_NO_TYPEID
    #endif

    #if defined(__int64) && !defined(__GNUC__)
        #define SPP_HAS_MS_INT64
    #endif

    #define SPP_HAS_NRVO

    
    #if defined(__has_builtin)
        #if __has_builtin(__builtin_expect)
             #define SPP_LIKELY(x) __builtin_expect(x, 1)
             #define SPP_UNLIKELY(x) __builtin_expect(x, 0)
        #endif
    #endif

    
    #define SPP_HAS_LONG_LONG

    #if !__has_feature(cxx_constexpr)
        #define SPP_NO_CXX11_CONSTEXPR
    #endif

    #if !__has_feature(cxx_decltype)
        #define SPP_NO_CXX11_DECLTYPE
    #endif

    #if !__has_feature(cxx_decltype_incomplete_return_types)
        #define SPP_NO_CXX11_DECLTYPE_N3276
    #endif

    #if !__has_feature(cxx_defaulted_functions)
        #define SPP_NO_CXX11_DEFAULTED_FUNCTIONS
    #endif

    #if !__has_feature(cxx_deleted_functions)
        #define SPP_NO_CXX11_DELETED_FUNCTIONS
    #endif

    #if !__has_feature(cxx_explicit_conversions)
        #define SPP_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
    #endif

    #if !__has_feature(cxx_default_function_template_args)
        #define SPP_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    #endif

    #if !__has_feature(cxx_generalized_initializers)
        #define SPP_NO_CXX11_HDR_INITIALIZER_LIST
    #endif

    #if !__has_feature(cxx_lambdas)
        #define SPP_NO_CXX11_LAMBDAS
    #endif

    #if !__has_feature(cxx_local_type_template_args)
        #define SPP_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
    #endif

    #if !__has_feature(cxx_raw_string_literals)
        #define SPP_NO_CXX11_RAW_LITERALS
    #endif

    #if !__has_feature(cxx_reference_qualified_functions)
        #define SPP_NO_CXX11_REF_QUALIFIERS
    #endif

    #if !__has_feature(cxx_generalized_initializers)
        #define SPP_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
    #endif

    #if !__has_feature(cxx_rvalue_references)
        #define SPP_NO_CXX11_RVALUE_REFERENCES
    #endif

    #if !__has_feature(cxx_static_assert)
        #define SPP_NO_CXX11_STATIC_ASSERT
    #endif

    #if !__has_feature(cxx_alias_templates)
        #define SPP_NO_CXX11_TEMPLATE_ALIASES
    #endif

    #if !__has_feature(cxx_variadic_templates)
        #define SPP_NO_CXX11_VARIADIC_TEMPLATES
    #endif

    #if !__has_feature(cxx_user_literals)
        #define SPP_NO_CXX11_USER_DEFINED_LITERALS
    #endif

    #if !__has_feature(cxx_alignas)
        #define SPP_NO_CXX11_ALIGNAS
    #endif

    #if !__has_feature(cxx_trailing_return)
        #define SPP_NO_CXX11_TRAILING_RESULT_TYPES
    #endif

    #if !__has_feature(cxx_inline_namespaces)
        #define SPP_NO_CXX11_INLINE_NAMESPACES
    #endif

    #if !__has_feature(cxx_override_control)
        #define SPP_NO_CXX11_FINAL
    #endif

    #if !(__has_feature(__cxx_binary_literals__) || __has_extension(__cxx_binary_literals__))
        #define SPP_NO_CXX14_BINARY_LITERALS
    #endif

    #if !__has_feature(__cxx_decltype_auto__)
        #define SPP_NO_CXX14_DECLTYPE_AUTO
    #endif

    #if !__has_feature(__cxx_init_captures__)
        #define SPP_NO_CXX14_INITIALIZED_LAMBDA_CAPTURES
    #endif

    #if !__has_feature(__cxx_generic_lambdas__)
        #define SPP_NO_CXX14_GENERIC_LAMBDAS
    #endif


    #if !__has_feature(__cxx_generic_lambdas__) || !__has_feature(__cxx_relaxed_constexpr__)
        #define SPP_NO_CXX14_CONSTEXPR
    #endif

    #if !__has_feature(__cxx_return_type_deduction__)
        #define SPP_NO_CXX14_RETURN_TYPE_DEDUCTION
    #endif

    #if !__has_feature(__cxx_variable_templates__)
        #define SPP_NO_CXX14_VARIABLE_TEMPLATES
    #endif

    #if __cplusplus < 201400
        #define SPP_NO_CXX14_DIGIT_SEPARATORS
    #endif

    #if defined(__has_builtin) && __has_builtin(__builtin_unreachable)
      #define SPP_UNREACHABLE_RETURN(x) __builtin_unreachable();
    #endif

    #define SPP_ATTRIBUTE_UNUSED __attribute__((__unused__))

    #ifndef SPP_COMPILER
        #define SPP_COMPILER "Clang version " __clang_version__
    #endif

    #define SPP_CLANG 1


#elif defined __GNUC__

    #define SPP_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

    
    
    
    
    

    #if defined(i386)
        #include <cpuid.h>
        inline void spp_cpuid(int info[4], int InfoType) {
            __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
        }
    #endif

    
    
    #ifdef __POPCNT__
        
        #define SPP_POPCNT   __builtin_popcount
        #define SPP_POPCNT64 __builtin_popcountll
    #endif

    #if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
        #define SPP_GCC_CXX11
    #endif

    #if __GNUC__ == 3
        #if defined (__PATHSCALE__)
             #define SPP_NO_TWO_PHASE_NAME_LOOKUP
             #define SPP_NO_IS_ABSTRACT
        #endif

        #if __GNUC_MINOR__ < 4
             #define SPP_NO_IS_ABSTRACT
        #endif

        #define SPP_NO_CXX11_EXTERN_TEMPLATE
    #endif

    #if __GNUC__ < 4
    
    
    
    #define SPP_NO_TWO_PHASE_NAME_LOOKUP
        #ifdef __OPEN64__
            #define SPP_NO_IS_ABSTRACT
        #endif
    #endif

    
    #if SPP_GCC_VERSION >= 30400
        #define SPP_HAS_PRAGMA_ONCE
    #endif

    #if SPP_GCC_VERSION < 40400
        
        
        
        
        
        
        
        
        #define SPP_NO_COMPLETE_VALUE_INITIALIZATION
    #endif

    #if !defined(__EXCEPTIONS) && !defined(SPP_NO_EXCEPTIONS)
        #define SPP_NO_EXCEPTIONS
    #endif

    
    
    
    
    
    #if !defined(__MINGW32__) && !defined(linux) && !defined(__linux) && !defined(__linux__)
        #define SPP_HAS_THREADS
    #endif

    
    
    
    
    
    #if !defined(__DARWIN_NO_LONG_LONG)
        #define SPP_HAS_LONG_LONG
    #endif

    
    
    
    #define SPP_HAS_NRVO

    
    #define SPP_LIKELY(x) __builtin_expect(x, 1)
    #define SPP_UNLIKELY(x) __builtin_expect(x, 0)

    
    
    
    #if __GNUC__ >= 4
       #if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
            
            
            
            #define SPP_HAS_DECLSPEC
            #define SPP_SYMBOL_EXPORT __attribute__((__dllexport__))
            #define SPP_SYMBOL_IMPORT __attribute__((__dllimport__))
       #else
            #define SPP_SYMBOL_EXPORT __attribute__((__visibility__("default")))
            #define SPP_SYMBOL_IMPORT
       #endif

       #define SPP_SYMBOL_VISIBLE __attribute__((__visibility__("default")))
    #else
       
       #define SPP_SYMBOL_EXPORT
    #endif

    
    
    
    #if SPP_GCC_VERSION > 40300
        #ifndef __GXX_RTTI
            #ifndef SPP_NO_TYPEID
                #define SPP_NO_TYPEID
            #endif
            #ifndef SPP_NO_RTTI
                #define SPP_NO_RTTI
            #endif
        #endif
    #endif

    
    
    
    
    
    
    
    
    
    
    
    #if defined(__CUDACC__)
        #if defined(SPP_GCC_CXX11)
            #define SPP_NVCC_CXX11
        #else
            #define SPP_NVCC_CXX03
        #endif
    #endif

    #if defined(__SIZEOF_INT128__) && !defined(SPP_NVCC_CXX03)
        #define SPP_HAS_INT128
    #endif
    
    
    
    
    
    
    
    
    
    #ifdef __cplusplus
        #include <cstddef>
    #else
        #include <stddef.h>
    #endif

    #if defined(_GLIBCXX_USE_FLOAT128) && !defined(__STRICT_ANSI__) && !defined(SPP_NVCC_CXX03)
         #define SPP_HAS_FLOAT128
    #endif

    
    
    #if (SPP_GCC_VERSION >= 40300) && defined(SPP_GCC_CXX11)
       
       
       
       #define SPP_HAS_DECLTYPE
       #define SPP_HAS_RVALUE_REFS
       #define SPP_HAS_STATIC_ASSERT
       #define SPP_HAS_VARIADIC_TMPL
       #define SPP_HAS_CSTDINT
    #else
       #define SPP_NO_CXX11_DECLTYPE
       #define SPP_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
       #define SPP_NO_CXX11_RVALUE_REFERENCES
       #define SPP_NO_CXX11_STATIC_ASSERT
    #endif

    
    
    #if (SPP_GCC_VERSION < 40400) || !defined(SPP_GCC_CXX11)
       #define SPP_NO_CXX11_AUTO_DECLARATIONS
       #define SPP_NO_CXX11_AUTO_MULTIDECLARATIONS
       #define SPP_NO_CXX11_CHAR16_T
       #define SPP_NO_CXX11_CHAR32_T
       #define SPP_NO_CXX11_HDR_INITIALIZER_LIST
       #define SPP_NO_CXX11_DEFAULTED_FUNCTIONS
       #define SPP_NO_CXX11_DELETED_FUNCTIONS
       #define SPP_NO_CXX11_TRAILING_RESULT_TYPES
       #define SPP_NO_CXX11_INLINE_NAMESPACES
       #define SPP_NO_CXX11_VARIADIC_TEMPLATES
    #endif

    #if SPP_GCC_VERSION < 40500
       #define SPP_NO_SFINAE_EXPR
    #endif

    
    #if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ == 5) || !defined(SPP_GCC_CXX11)
       #define SPP_NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS
    #endif

    
    
    #if (SPP_GCC_VERSION < 40500) || !defined(SPP_GCC_CXX11)
       #define SPP_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
       #define SPP_NO_CXX11_LAMBDAS
       #define SPP_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
       #define SPP_NO_CXX11_RAW_LITERALS
    #endif

    
    
    #if (SPP_GCC_VERSION < 40600) || !defined(SPP_GCC_CXX11)
        #define SPP_NO_CXX11_CONSTEXPR
        #define SPP_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
    #endif

    
    
    #if (SPP_GCC_VERSION < 40700) || !defined(SPP_GCC_CXX11)
        #define SPP_NO_CXX11_FINAL
        #define SPP_NO_CXX11_TEMPLATE_ALIASES
        #define SPP_NO_CXX11_USER_DEFINED_LITERALS
        #define SPP_NO_CXX11_FIXED_LENGTH_VARIADIC_TEMPLATE_EXPANSION_PACKS
    #endif

    
    
    #if (SPP_GCC_VERSION < 40800) || !defined(SPP_GCC_CXX11)
        #define SPP_NO_CXX11_ALIGNAS
    #endif

    
    
    #if (SPP_GCC_VERSION < 40801) || !defined(SPP_GCC_CXX11)
        #define SPP_NO_CXX11_DECLTYPE_N3276
        #define SPP_NO_CXX11_REF_QUALIFIERS
        #define SPP_NO_CXX14_BINARY_LITERALS
    #endif

    
    
    #if (SPP_GCC_VERSION < 40900) || (__cplusplus < 201300)
        #define SPP_NO_CXX14_RETURN_TYPE_DEDUCTION
        #define SPP_NO_CXX14_GENERIC_LAMBDAS
        #define SPP_NO_CXX14_DIGIT_SEPARATORS
        #define SPP_NO_CXX14_DECLTYPE_AUTO
        #if !((SPP_GCC_VERSION >= 40801) && (SPP_GCC_VERSION < 40900) && defined(SPP_GCC_CXX11))
            #define SPP_NO_CXX14_INITIALIZED_LAMBDA_CAPTURES
        #endif
    #endif


    
    #if !defined(__cpp_constexpr) || (__cpp_constexpr < 201304)
        #define SPP_NO_CXX14_CONSTEXPR
    #endif
    #if !defined(__cpp_variable_templates) || (__cpp_variable_templates < 201304)
        #define SPP_NO_CXX14_VARIABLE_TEMPLATES
    #endif

    
    
    #if __GNUC__ >= 4
        #define SPP_ATTRIBUTE_UNUSED __attribute__((__unused__))
    #endif
    
    
    #if SPP_GCC_VERSION >= 40800
        #define SPP_UNREACHABLE_RETURN(x) __builtin_unreachable();
    #endif

    #ifndef SPP_COMPILER
        #define SPP_COMPILER "GNU C++ version " __VERSION__
    #endif

    
    
    #ifdef __GXX_CONCEPTS__
        #define SPP_HAS_CONCEPTS
        #define SPP_COMPILER "ConceptGCC version " __VERSION__
    #endif

#elif defined _MSC_VER

    #include <intrin.h>                     

    #define SPP_POPCNT_CHECK  
    #define spp_cpuid(info, x)    __cpuid(info, x)

    #define SPP_POPCNT __popcnt
    #if (SPP_GROUP_SIZE == 64 && INTPTR_MAX == INT64_MAX)
        #define SPP_POPCNT64 __popcnt64
    #endif

    
    #pragma warning( disable : 4503 ) 

    #define SPP_HAS_PRAGMA_ONCE
    #define SPP_HAS_CSTDINT

   
    
    
    #if _MSC_VER < 1310
        #error "Antique compiler not supported"
    #endif

    #if _MSC_FULL_VER < 180020827
        #define SPP_NO_FENV_H
    #endif

    #if _MSC_VER < 1400
        
        
        #define SPP_NO_SWPRINTF

        
        #define SPP_NO_CXX11_EXTERN_TEMPLATE

        
        #define SPP_NO_CXX11_VARIADIC_MACROS
    #endif

    #if _MSC_VER < 1500  
        #undef SPP_HAS_CSTDINT
        #define SPP_NO_MEMBER_TEMPLATE_FRIENDS
    #endif

    #if _MSC_VER < 1600  
        
        #define SPP_NO_ADL_BARRIER
    #endif


    
    
    
    
    
    
    
    
    
    
    
    
    
    #define SPP_NO_COMPLETE_VALUE_INITIALIZATION

    #ifndef _NATIVE_WCHAR_T_DEFINED
        #define SPP_NO_INTRINSIC_WCHAR_T
    #endif

    
    
    #if !defined(_CPPUNWIND) && !defined(SPP_NO_EXCEPTIONS)
        #define SPP_NO_EXCEPTIONS
    #endif

    
    
    
    #define SPP_HAS_MS_INT64
    #if defined(_MSC_EXTENSIONS) || (_MSC_VER >= 1400)
        #define SPP_HAS_LONG_LONG
    #else
        #define SPP_NO_LONG_LONG
    #endif

    #if (_MSC_VER >= 1400) && !defined(_DEBUG)
        #define SPP_HAS_NRVO
    #endif

    #if _MSC_VER >= 1500  
        #define SPP_HAS_PRAGMA_DETECT_MISMATCH
    #endif

    
    
    
    
    #if !defined(_MSC_EXTENSIONS) && !defined(SPP_DISABLE_WIN32)
        #define SPP_DISABLE_WIN32
    #endif

    #if !defined(_CPPRTTI) && !defined(SPP_NO_RTTI)
        #define SPP_NO_RTTI
    #endif

    
    
    
    #if _MSC_VER >= 1700
        
        
        #define SPP_HAS_TR1_UNORDERED_MAP
        #define SPP_HAS_TR1_UNORDERED_SET
    #endif

    
    
    
    

    
    
    #if _MSC_VER < 1600
        #define SPP_NO_CXX11_AUTO_DECLARATIONS
        #define SPP_NO_CXX11_AUTO_MULTIDECLARATIONS
        #define SPP_NO_CXX11_LAMBDAS
        #define SPP_NO_CXX11_RVALUE_REFERENCES
        #define SPP_NO_CXX11_STATIC_ASSERT
        #define SPP_NO_CXX11_DECLTYPE
    #endif 

    #if _MSC_VER >= 1600
        #define SPP_HAS_STDINT_H
    #endif

    
    
    #if _MSC_VER < 1700
        #define SPP_NO_CXX11_FINAL
    #endif 

    
    
    #if _MSC_FULL_VER < 180020827
        #define SPP_NO_CXX11_DEFAULTED_FUNCTIONS
        #define SPP_NO_CXX11_DELETED_FUNCTIONS
        #define SPP_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
        #define SPP_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
        #define SPP_NO_CXX11_RAW_LITERALS
        #define SPP_NO_CXX11_TEMPLATE_ALIASES
        #define SPP_NO_CXX11_TRAILING_RESULT_TYPES
        #define SPP_NO_CXX11_VARIADIC_TEMPLATES
        #define SPP_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
        #define SPP_NO_CXX11_DECLTYPE_N3276
    #endif

    
    #if (_MSC_FULL_VER < 190021730)
        #define SPP_NO_CXX11_REF_QUALIFIERS
        #define SPP_NO_CXX11_USER_DEFINED_LITERALS
        #define SPP_NO_CXX11_ALIGNAS
        #define SPP_NO_CXX11_INLINE_NAMESPACES
        #define SPP_NO_CXX14_DECLTYPE_AUTO
        #define SPP_NO_CXX14_INITIALIZED_LAMBDA_CAPTURES
        #define SPP_NO_CXX14_RETURN_TYPE_DEDUCTION
        #define SPP_NO_CXX11_HDR_INITIALIZER_LIST
    #endif

    
    #define SPP_NO_CXX11_CHAR16_T
    #define SPP_NO_CXX11_CHAR32_T
    #define SPP_NO_CXX11_CONSTEXPR
    #define SPP_NO_SFINAE_EXPR
    #define SPP_NO_TWO_PHASE_NAME_LOOKUP

    
    #if !defined(__cpp_binary_literals) || (__cpp_binary_literals < 201304)
        #define SPP_NO_CXX14_BINARY_LITERALS
    #endif

    #if !defined(__cpp_constexpr) || (__cpp_constexpr < 201304)
        #define SPP_NO_CXX14_CONSTEXPR
    #endif

    #if (__cplusplus < 201304) 
        #define SPP_NO_CXX14_DIGIT_SEPARATORS
    #endif

    #if !defined(__cpp_generic_lambdas) || (__cpp_generic_lambdas < 201304)
        #define SPP_NO_CXX14_GENERIC_LAMBDAS
    #endif

    #if !defined(__cpp_variable_templates) || (__cpp_variable_templates < 201304)
         #define SPP_NO_CXX14_VARIABLE_TEMPLATES
    #endif

#endif



#ifndef SPP_ATTRIBUTE_UNUSED
    #define SPP_ATTRIBUTE_UNUSED
#endif


#ifndef SPP_FORCEINLINE
    #if defined(__GNUC__)
        #define SPP_FORCEINLINE __inline __attribute__ ((always_inline))
    #elif defined(_MSC_VER)
        #define SPP_FORCEINLINE __forceinline
    #else
        #define SPP_FORCEINLINE inline
    #endif
#endif


#endif 
