


































































#if !defined(spp_utils_h_guard_)
#define spp_utils_h_guard_

#if defined(_MSC_VER)
    #if (_MSC_VER >= 1600 )                      
        #include <functional>
        #define SPP_HASH_CLASS std::hash
    #else
        #include  <hash_map>
        #define SPP_HASH_CLASS stdext::hash_compare
    #endif
    #if (_MSC_FULL_VER < 190021730)
        #define SPP_NO_CXX11_NOEXCEPT
    #endif
#elif defined __clang__
    #if __has_feature(cxx_noexcept)  
       #include <functional>
       #define SPP_HASH_CLASS  std::hash
    #else
       #include <tr1/unordered_map>
       #define SPP_HASH_CLASS std::tr1::hash
    #endif

    #if !__has_feature(cxx_noexcept)
        #define SPP_NO_CXX11_NOEXCEPT
    #endif
#elif defined(__GNUC__)
    #if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
        #include <functional>
        #define SPP_HASH_CLASS std::hash

        #if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) < 40600
            #define SPP_NO_CXX11_NOEXCEPT
        #endif
    #else
        #include <tr1/unordered_map>
        #define SPP_HASH_CLASS std::tr1::hash
        #define SPP_NO_CXX11_NOEXCEPT
    #endif
#else
    #include <functional>
    #define SPP_HASH_CLASS  std::hash
#endif

#ifdef SPP_NO_CXX11_NOEXCEPT
    #define SPP_NOEXCEPT
#else
    #define SPP_NOEXCEPT noexcept
#endif

#ifdef SPP_NO_CXX11_CONSTEXPR
    #define SPP_CONSTEXPR
#else
    #define SPP_CONSTEXPR constexpr
#endif

#ifdef SPP_NO_CXX14_CONSTEXPR
    #define SPP_CXX14_CONSTEXPR
#else
    #define SPP_CXX14_CONSTEXPR constexpr
#endif

#define SPP_INLINE

#ifndef spp_
    #define spp_ spp
#endif

namespace spp_
{

template <class T>  T spp_min(T a, T b) { return a < b  ? a : b; }
template <class T>  T spp_max(T a, T b) { return a >= b ? a : b; }

template <class T>
struct spp_hash
{
    SPP_INLINE size_t operator()(const T &__v) const SPP_NOEXCEPT
    {
        SPP_HASH_CLASS<T> hasher;
        return hasher(__v);
    }
};

template <class T>
struct spp_hash<T *>
{
    static size_t spp_log2 (size_t val) SPP_NOEXCEPT
    {
        size_t res = 0;
        while (val > 1)
        {
            val >>= 1;
            res++;
        }
        return res;
    }

    SPP_INLINE size_t operator()(const T *__v) const SPP_NOEXCEPT
    {
        static const size_t shift = 3; 
        const uintptr_t i = (const uintptr_t)__v;
        return static_cast<size_t>(i >> shift);
    }
};





inline size_t spp_mix_32(uint32_t a)
{
    a = a ^ (a >> 4);
    a = (a ^ 0xdeadbeef) + (a << 5);
    a = a ^ (a >> 11);
    return static_cast<size_t>(a);
}




inline size_t spp_mix_64(uint64_t a)
{
    a = (~a) + (a << 21); 
    a = a ^ (a >> 24);
    a = (a + (a << 3)) + (a << 8); 
    a = a ^ (a >> 14);
    a = (a + (a << 2)) + (a << 4); 
    a = a ^ (a >> 28);
    a = a + (a << 31);
    return static_cast<size_t>(a);
}

template<class ArgumentType, class ResultType>
struct spp_unary_function
{
    typedef ArgumentType argument_type;
    typedef ResultType result_type;
};

template <>
struct spp_hash<bool> : public spp_unary_function<bool, size_t>
{
    SPP_INLINE size_t operator()(bool __v) const SPP_NOEXCEPT
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<char> : public spp_unary_function<char, size_t>
{
    SPP_INLINE size_t operator()(char __v) const SPP_NOEXCEPT
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<signed char> : public spp_unary_function<signed char, size_t>
{
    SPP_INLINE size_t operator()(signed char __v) const SPP_NOEXCEPT
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<unsigned char> : public spp_unary_function<unsigned char, size_t>
{
    SPP_INLINE size_t operator()(unsigned char __v) const SPP_NOEXCEPT
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<wchar_t> : public spp_unary_function<wchar_t, size_t>
{
    SPP_INLINE size_t operator()(wchar_t __v) const SPP_NOEXCEPT
    { return static_cast<size_t>(__v); }
};

template <>
struct spp_hash<int16_t> : public spp_unary_function<int16_t, size_t>
{
    SPP_INLINE size_t operator()(int16_t __v) const SPP_NOEXCEPT
    { return spp_mix_32(static_cast<uint32_t>(__v)); }
};

template <>
struct spp_hash<uint16_t> : public spp_unary_function<uint16_t, size_t>
{
    SPP_INLINE size_t operator()(uint16_t __v) const SPP_NOEXCEPT
    { return spp_mix_32(static_cast<uint32_t>(__v)); }
};

template <>
struct spp_hash<int32_t> : public spp_unary_function<int32_t, size_t>
{
    SPP_INLINE size_t operator()(int32_t __v) const SPP_NOEXCEPT
    { return spp_mix_32(static_cast<uint32_t>(__v)); }
};

template <>
struct spp_hash<uint32_t> : public spp_unary_function<uint32_t, size_t>
{
    SPP_INLINE size_t operator()(uint32_t __v) const SPP_NOEXCEPT
    { return spp_mix_32(static_cast<uint32_t>(__v)); }
};

template <>
struct spp_hash<int64_t> : public spp_unary_function<int64_t, size_t>
{
    SPP_INLINE size_t operator()(int64_t __v) const SPP_NOEXCEPT
    { return spp_mix_64(static_cast<uint64_t>(__v)); }
};

template <>
struct spp_hash<uint64_t> : public spp_unary_function<uint64_t, size_t>
{
    SPP_INLINE size_t operator()(uint64_t __v) const SPP_NOEXCEPT
    { return spp_mix_64(static_cast<uint64_t>(__v)); }
};

template <>
struct spp_hash<float> : public spp_unary_function<float, size_t>
{
    SPP_INLINE size_t operator()(float __v) const SPP_NOEXCEPT
    {
        
        uint32_t *as_int = reinterpret_cast<uint32_t *>(&__v);
        return (__v == 0) ? static_cast<size_t>(0) : spp_mix_32(*as_int);
    }
};

template <>
struct spp_hash<double> : public spp_unary_function<double, size_t>
{
    SPP_INLINE size_t operator()(double __v) const SPP_NOEXCEPT
    {
        
        uint64_t *as_int = reinterpret_cast<uint64_t *>(&__v);
        return (__v == 0) ? static_cast<size_t>(0) : spp_mix_64(*as_int);
    }
};

template <class T, int sz> struct Combiner
{
    inline void operator()(T& seed, T value);
};

template <class T> struct Combiner<T, 4>
{
    inline void  operator()(T& seed, T value)
    {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
};

template <class T> struct Combiner<T, 8>
{
    inline void  operator()(T& seed, T value)
    {
        seed ^= value + T(0xc6a4a7935bd1e995) + (seed << 6) + (seed >> 2);
    }
};

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    spp_::spp_hash<T> hasher;
    Combiner<std::size_t, sizeof(std::size_t)> combiner;

    combiner(seed, hasher(v));
}

static inline uint32_t s_spp_popcount_default(uint32_t i) SPP_NOEXCEPT
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static inline uint32_t s_spp_popcount_default(uint64_t x) SPP_NOEXCEPT
{
    const uint64_t m1  = uint64_t(0x5555555555555555); 
    const uint64_t m2  = uint64_t(0x3333333333333333); 
    const uint64_t m4  = uint64_t(0x0f0f0f0f0f0f0f0f); 
    const uint64_t h01 = uint64_t(0x0101010101010101); 

    x -= (x >> 1) & m1;             
    x = (x & m2) + ((x >> 2) & m2); 
    x = (x + (x >> 4)) & m4;        
    return (x * h01)>>56;           
}

#ifdef __APPLE__
    static inline uint32_t count_trailing_zeroes(size_t v) SPP_NOEXCEPT
    {
        size_t x = (v & -v) - 1;
        
        return sizeof(size_t) == 8 ? s_spp_popcount_default((uint64_t)x) : s_spp_popcount_default((uint32_t)x);
    }

    static inline uint32_t s_popcount(size_t v) SPP_NOEXCEPT
    {
        
        return sizeof(size_t) == 8 ? s_spp_popcount_default((uint64_t)v) : s_spp_popcount_default((uint32_t)v);
    }
#else
    static inline uint32_t count_trailing_zeroes(size_t v) SPP_NOEXCEPT
    {
        return s_spp_popcount_default((v & -(intptr_t)v) - 1);
    }

    static inline uint32_t s_popcount(size_t v) SPP_NOEXCEPT
    {
        return s_spp_popcount_default(v);
    }
#endif



template<class T>
class libc_allocator
{
public:
    typedef T         value_type;
    typedef T*        pointer;
    typedef ptrdiff_t difference_type;
    typedef const T*  const_pointer;
    typedef size_t    size_type;

    libc_allocator() {}
    libc_allocator(const libc_allocator&) {}

    template<class U>
    libc_allocator(const libc_allocator<U> &) {}

    libc_allocator& operator=(const libc_allocator &) { return *this; }

    template<class U>
    libc_allocator& operator=(const libc_allocator<U> &) { return *this; }

#ifndef SPP_NO_CXX11_RVALUE_REFERENCES    
    libc_allocator(libc_allocator &&) {}
    libc_allocator& operator=(libc_allocator &&) { return *this; }
#endif

    pointer allocate(size_t n, const_pointer  = 0) 
    {
        return static_cast<pointer>(malloc(n * sizeof(T)));
    }

    void deallocate(pointer p, size_t ) 
    {
        free(p);
    }

    pointer reallocate(pointer p, size_t new_size) 
    {
        return static_cast<pointer>(realloc(p, new_size * sizeof(T)));
    }

    
    pointer reallocate(pointer p, size_t , size_t new_size) 
    {
        return static_cast<pointer>(realloc(p, new_size * sizeof(T)));
    }

    size_type max_size() const
    {
        return static_cast<size_type>(-1) / sizeof(value_type);
    }

    void construct(pointer p, const value_type& val)
    {
        new(p) value_type(val);
    }

    void destroy(pointer p) { p->~value_type(); }

    template<class U>
    struct rebind
    {
        typedef spp_::libc_allocator<U> other;
    };

};



template<class T>
class spp_allocator;

}

template<class T>
inline bool operator==(const spp_::libc_allocator<T> &, const spp_::libc_allocator<T> &)
{
    return true;
}

template<class T>
inline bool operator!=(const spp_::libc_allocator<T> &, const spp_::libc_allocator<T> &)
{
    return false;
}

#endif 

