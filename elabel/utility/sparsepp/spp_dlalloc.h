#ifndef spp_dlalloc__h_
#define spp_dlalloc__h_



#include "spp_utils.h"
#include "spp_smartptr.h"


#ifndef SPP_FORCEINLINE
    #if defined(__GNUC__)
        #define SPP_FORCEINLINE __inline __attribute__ ((always_inline))
    #elif defined(_MSC_VER)
        #define SPP_FORCEINLINE __forceinline
    #else
        #define SPP_FORCEINLINE inline
    #endif
#endif


#ifndef SPP_IMPL
    #define SPP_IMPL SPP_FORCEINLINE
#endif

#ifndef SPP_API
    #define SPP_API  static
#endif


namespace spp
{
    
    typedef void* mspace;

    
    SPP_API mspace create_mspace(size_t capacity, int locked);
    SPP_API size_t destroy_mspace(mspace msp);
    SPP_API void*  mspace_malloc(mspace msp, size_t bytes);
    SPP_API void   mspace_free(mspace msp, void* mem);
    SPP_API void*  mspace_realloc(mspace msp, void* mem, size_t newsize);

#if 0
    SPP_API mspace create_mspace_with_base(void* base, size_t capacity, int locked);
    SPP_API int    mspace_track_large_chunks(mspace msp, int enable);
    SPP_API void*  mspace_calloc(mspace msp, size_t n_elements, size_t elem_size);
    SPP_API void*  mspace_memalign(mspace msp, size_t alignment, size_t bytes);
    SPP_API void** mspace_independent_calloc(mspace msp, size_t n_elements,
                                             size_t elem_size, void* chunks[]);
    SPP_API void** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                               size_t sizes[], void* chunks[]);
    SPP_API size_t mspace_footprint(mspace msp);
    SPP_API size_t mspace_max_footprint(mspace msp);
    SPP_API size_t mspace_usable_size(const void* mem);
    SPP_API int    mspace_trim(mspace msp, size_t pad);
    SPP_API int    mspace_mallopt(int, int);
#endif

    
    
    struct MSpace : public spp_rc
    {
        MSpace() :
            _sp(create_mspace(0, 0))
        {}

        ~MSpace()
        {
            destroy_mspace(_sp);
        }

        mspace _sp;
    };

    
    
    template<class T>
    class spp_allocator
    {
    public:
        typedef T         value_type;
        typedef T*        pointer;
        typedef ptrdiff_t difference_type;
        typedef const T*  const_pointer;
        typedef size_t    size_type;

        MSpace *getSpace() const { return _space.get(); }

        spp_allocator() : _space(new MSpace) {}
        
        template<class U>
        spp_allocator(const spp_allocator<U> &o) : _space(o.getSpace()) {}

        template<class U>
        spp_allocator& operator=(const spp_allocator<U> &o) 
        {
            if (&o != this)
                _space = o.getSpace();
            return *this;
        }

        void swap(spp_allocator &o)
        {
            std::swap(_space, o._space);
        }

        pointer allocate(size_t n, const_pointer   = 0)
        {
            pointer res = static_cast<pointer>(mspace_malloc(_space->_sp, n * sizeof(T)));
            if (!res)
                throw std::bad_alloc();
            return res;
        }

        void deallocate(pointer p, size_t )
        {
            mspace_free(_space->_sp, p);
        }

        pointer reallocate(pointer p, size_t new_size)
        {
            pointer res = static_cast<pointer>(mspace_realloc(_space->_sp, p, new_size * sizeof(T)));
            if (!res)
                throw std::bad_alloc();
            return res;
        }

        pointer reallocate(pointer p, size_type , size_t new_size)
        {
            return reallocate(p, new_size);
        }
        
        size_type max_size() const
        {
            return static_cast<size_type>(-1) / sizeof(value_type);
        }

        void construct(pointer p, const value_type& val)
        {
            new (p) value_type(val);
        }

        void destroy(pointer p) { p->~value_type(); }

        template<class U>
        struct rebind
        {
            
            
            
            typedef spp::spp_allocator<U> other;
        };

        mspace space() const { return _space->_sp; }

        
        
        
        bool can_clear()
        {
            assert(!_space_to_clear);
            _space_to_clear.reset();
            _space_to_clear.swap(_space);
            if (_space_to_clear->count() == 1)
                return true;
            else
                _space_to_clear.swap(_space);
            return false;
        }

        void clear()
        {
            assert(!_space && !!_space_to_clear);
            _space_to_clear.reset();
            _space = new MSpace;
        }
        
    private:
        spp_sptr<MSpace> _space;
        spp_sptr<MSpace> _space_to_clear;
    };
}



template<class T>
inline bool operator==(const spp_::spp_allocator<T> &a, const spp_::spp_allocator<T> &b)
{
    return a.space() == b.space();
}

template<class T>
inline bool operator!=(const spp_::spp_allocator<T> &a, const spp_::spp_allocator<T> &b)
{
    return !(a == b);
}

namespace std
{
    template <class T>
    inline void swap(spp_::spp_allocator<T> &a, spp_::spp_allocator<T> &b)
    {
        a.swap(b);
    }
}

#if !defined(SPP_EXCLUDE_IMPLEMENTATION)

#ifndef WIN32
    #ifdef _WIN32
        #define WIN32 1
    #endif
    #ifdef _WIN32_WCE
        #define SPP_LACKS_FCNTL_H
        #define WIN32 1
    #endif
#endif

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <tchar.h>
    #define SPP_HAVE_MMAP 1
    #define SPP_LACKS_UNISTD_H
    #define SPP_LACKS_SYS_PARAM_H
    #define SPP_LACKS_SYS_MMAN_H
    #define SPP_LACKS_STRING_H
    #define SPP_LACKS_STRINGS_H
    #define SPP_LACKS_SYS_TYPES_H
    #define SPP_LACKS_ERRNO_H
    #define SPP_LACKS_SCHED_H
    #ifndef SPP_MALLOC_FAILURE_ACTION
        #define SPP_MALLOC_FAILURE_ACTION
    #endif
    #ifndef SPP_MMAP_CLEARS
        #ifdef _WIN32_WCE 
            #define SPP_MMAP_CLEARS 0
        #else
            #define SPP_MMAP_CLEARS 1
        #endif
    #endif
#endif

#if defined(DARWIN) || defined(_DARWIN)
    #define SPP_HAVE_MMAP 1
    
    #ifndef SPP_MALLOC_ALIGNMENT
        #define SPP_MALLOC_ALIGNMENT ((size_t)16U)
    #endif
#endif

#ifndef SPP_LACKS_SYS_TYPES_H
    #include <sys/types.h>  
#endif

#ifndef SPP_MALLOC_ALIGNMENT
    #define SPP_MALLOC_ALIGNMENT ((size_t)(2 * sizeof(void *)))
#endif


static const size_t spp_max_size_t = ~(size_t)0;
static const size_t spp_size_t_bitsize = sizeof(size_t) << 3;
static const size_t spp_half_max_size_t = spp_max_size_t / 2U;
static const size_t spp_chunk_align_mask = SPP_MALLOC_ALIGNMENT - 1;

#if defined(SPP_DEBUG) || !defined(NDEBUG)
static bool spp_is_aligned(void *p) { return ((size_t)p & spp_chunk_align_mask) == 0; }
#endif


static size_t align_offset(void *p)
{
    return (((size_t)p & spp_chunk_align_mask) == 0) ? 0 :
           ((SPP_MALLOC_ALIGNMENT - ((size_t)p & spp_chunk_align_mask)) & spp_chunk_align_mask);
}


#ifndef SPP_FOOTERS
    #define SPP_FOOTERS 0
#endif

#ifndef SPP_ABORT
    #define SPP_ABORT  abort()
#endif

#ifndef SPP_ABORT_ON_ASSERT_FAILURE
    #define SPP_ABORT_ON_ASSERT_FAILURE 1
#endif

#ifndef SPP_PROCEED_ON_ERROR
    #define SPP_PROCEED_ON_ERROR 0
#endif

#ifndef SPP_INSECURE
    #define SPP_INSECURE 0
#endif

#ifndef SPP_MALLOC_INSPECT_ALL
    #define SPP_MALLOC_INSPECT_ALL 0
#endif

#ifndef SPP_HAVE_MMAP
    #define SPP_HAVE_MMAP 1
#endif

#ifndef SPP_MMAP_CLEARS
    #define SPP_MMAP_CLEARS 1
#endif

#ifndef SPP_HAVE_MREMAP
    #ifdef linux
        #define SPP_HAVE_MREMAP 1
        #ifndef _GNU_SOURCE
            #define _GNU_SOURCE 
        #endif
    #else
        #define SPP_HAVE_MREMAP 0
    #endif
#endif

#ifndef SPP_MALLOC_FAILURE_ACTION
    
    #define SPP_MALLOC_FAILURE_ACTION  errno = 12
#endif


#ifndef SPP_DEFAULT_GRANULARITY
    #if defined(WIN32)
        #define SPP_DEFAULT_GRANULARITY (0)  
    #else
        #define SPP_DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
    #endif
#endif

#ifndef SPP_DEFAULT_TRIM_THRESHOLD
    #define SPP_DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#endif

#ifndef SPP_DEFAULT_MMAP_THRESHOLD
    #if SPP_HAVE_MMAP
        #define SPP_DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)
    #else
        #define SPP_DEFAULT_MMAP_THRESHOLD spp_max_size_t
    #endif
#endif

#ifndef SPP_MAX_RELEASE_CHECK_RATE
    #if SPP_HAVE_MMAP
        #define SPP_MAX_RELEASE_CHECK_RATE 4095
    #else
        #define SPP_MAX_RELEASE_CHECK_RATE spp_max_size_t
    #endif
#endif

#ifndef SPP_USE_BUILTIN_FFS
    #define SPP_USE_BUILTIN_FFS 0
#endif

#ifndef SPP_USE_DEV_RANDOM
    #define SPP_USE_DEV_RANDOM 0
#endif

#ifndef SPP_NO_SEGMENT_TRAVERSAL
    #define SPP_NO_SEGMENT_TRAVERSAL 0
#endif





#ifdef _MSC_VER
    #pragma warning( disable : 4146 ) 
#endif
#ifndef SPP_LACKS_ERRNO_H
    #include <errno.h>       
#endif

#ifdef SPP_DEBUG
    #if SPP_ABORT_ON_ASSERT_FAILURE
        #undef assert
        #define assert(x) if(!(x)) SPP_ABORT
    #else
        #include <assert.h>
    #endif
#else
    #ifndef assert
        #define assert(x)
    #endif
    #define SPP_DEBUG 0
#endif

#if !defined(WIN32) && !defined(SPP_LACKS_TIME_H)
    #include <time.h>        
#endif

#ifndef SPP_LACKS_STDLIB_H
    #include <stdlib.h>      
#endif

#ifndef SPP_LACKS_STRING_H
    #include <string.h>      
#endif

#if SPP_USE_BUILTIN_FFS
    #ifndef SPP_LACKS_STRINGS_H
        #include <strings.h>     
    #endif
#endif

#if SPP_HAVE_MMAP
    #ifndef SPP_LACKS_SYS_MMAN_H
        
        #if (defined(linux) && !defined(__USE_GNU))
            #define __USE_GNU 1
            #include <sys/mman.h>    
            #undef __USE_GNU
        #else
            #include <sys/mman.h>    
        #endif
    #endif
    #ifndef SPP_LACKS_FCNTL_H
        #include <fcntl.h>
    #endif
#endif

#ifndef SPP_LACKS_UNISTD_H
    #include <unistd.h>     
#else
    #if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
        extern void*     sbrk(ptrdiff_t);
    #endif
#endif

#include <new>

namespace spp
{


#if defined(_MSC_VER) && _MSC_VER>=1300
    #ifndef BitScanForward 
        extern "C" {
            unsigned char _BitScanForward(unsigned long *index, unsigned long mask);
            unsigned char _BitScanReverse(unsigned long *index, unsigned long mask);
        }
        
        #define BitScanForward _BitScanForward
        #define BitScanReverse _BitScanReverse
        #pragma intrinsic(_BitScanForward)
        #pragma intrinsic(_BitScanReverse)
    #endif 
#endif 

#ifndef WIN32
    #ifndef malloc_getpagesize
        #ifdef _SC_PAGESIZE         
            #ifndef _SC_PAGE_SIZE
                #define _SC_PAGE_SIZE _SC_PAGESIZE
            #endif
        #endif
        #ifdef _SC_PAGE_SIZE
            #define malloc_getpagesize sysconf(_SC_PAGE_SIZE)
        #else
            #if defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
                extern size_t getpagesize();
                #define malloc_getpagesize getpagesize()
            #else
                #ifdef WIN32 
                    #define malloc_getpagesize getpagesize()
                #else
                    #ifndef SPP_LACKS_SYS_PARAM_H
                        #include <sys/param.h>
                    #endif
                    #ifdef EXEC_PAGESIZE
                        #define malloc_getpagesize EXEC_PAGESIZE
                    #else
                        #ifdef NBPG
                            #ifndef CLSIZE
                                #define malloc_getpagesize NBPG
                            #else
                                #define malloc_getpagesize (NBPG * CLSIZE)
                            #endif
                        #else
                            #ifdef NBPC
                                #define malloc_getpagesize NBPC
                            #else
                                #ifdef PAGESIZE
                                    #define malloc_getpagesize PAGESIZE
                                #else 
                                    #define malloc_getpagesize ((size_t)4096U)
                                #endif
                            #endif
                        #endif
                    #endif
                #endif
            #endif
        #endif
    #endif
#endif







static void *mfail  = (void*)spp_max_size_t;
static char *cmfail = (char*)mfail;

#if SPP_HAVE_MMAP

#ifndef WIN32
    #define SPP_MUNMAP_DEFAULT(a, s)  munmap((a), (s))
    #define SPP_MMAP_PROT            (PROT_READ | PROT_WRITE)
    #if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
        #define MAP_ANONYMOUS        MAP_ANON
    #endif
    
    #ifdef MAP_ANONYMOUS
        #define SPP_MMAP_FLAGS           (MAP_PRIVATE | MAP_ANONYMOUS)
        #define SPP_MMAP_DEFAULT(s)       mmap(0, (s), SPP_MMAP_PROT, SPP_MMAP_FLAGS, -1, 0)
    #else 
        
        #define SPP_MMAP_FLAGS           (MAP_PRIVATE)
        static int dev_zero_fd = -1; 
        void SPP_MMAP_DEFAULT(size_t s)
        {
            if (dev_zero_fd < 0)
                dev_zero_fd = open("/dev/zero", O_RDWR);
            mmap(0, s, SPP_MMAP_PROT, SPP_MMAP_FLAGS, dev_zero_fd, 0);
        }
    #endif 
    
    #define SPP_DIRECT_MMAP_DEFAULT(s) SPP_MMAP_DEFAULT(s)
    
#else 
    
    
    static SPP_FORCEINLINE void* win32mmap(size_t size)
    {
        void* ptr = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        return (ptr != 0) ? ptr : mfail;
    }
    
    
    static SPP_FORCEINLINE void* win32direct_mmap(size_t size)
    {
        void* ptr = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN,
                                 PAGE_READWRITE);
        return (ptr != 0) ? ptr : mfail;
    }
    
    
    static SPP_FORCEINLINE int win32munmap(void* ptr, size_t size)
    {
        MEMORY_BASIC_INFORMATION minfo;
        char* cptr = (char*)ptr;
        while (size)
        {
            if (VirtualQuery(cptr, &minfo, sizeof(minfo)) == 0)
                return -1;
            if (minfo.BaseAddress != cptr || minfo.AllocationBase != cptr ||
                    minfo.State != MEM_COMMIT || minfo.RegionSize > size)
                return -1;
            if (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
                return -1;
            cptr += minfo.RegionSize;
            size -= minfo.RegionSize;
        }
        return 0;
    }
    
    #define SPP_MMAP_DEFAULT(s)             win32mmap(s)
    #define SPP_MUNMAP_DEFAULT(a, s)        win32munmap((a), (s))
    #define SPP_DIRECT_MMAP_DEFAULT(s)      win32direct_mmap(s)
#endif 
#endif 

#if SPP_HAVE_MREMAP
    #ifndef WIN32
        #define SPP_MREMAP_DEFAULT(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
    #endif
#endif


#if SPP_HAVE_MMAP
    #define USE_MMAP_BIT                1

    #ifdef SPP_MMAP
        #define SPP_CALL_MMAP(s)        SPP_MMAP(s)
    #else
        #define SPP_CALL_MMAP(s)        SPP_MMAP_DEFAULT(s)
    #endif

    #ifdef SPP_MUNMAP
        #define SPP_CALL_MUNMAP(a, s)   SPP_MUNMAP((a), (s))
    #else
        #define SPP_CALL_MUNMAP(a, s)   SPP_MUNMAP_DEFAULT((a), (s))
    #endif

    #ifdef SPP_DIRECT_MMAP
        #define SPP_CALL_DIRECT_MMAP(s) SPP_DIRECT_MMAP(s)
    #else
        #define SPP_CALL_DIRECT_MMAP(s) SPP_DIRECT_MMAP_DEFAULT(s)
    #endif

#else  
    #define USE_MMAP_BIT            0

    #define SPP_MMAP(s)                 mfail
    #define SPP_MUNMAP(a, s)            (-1)
    #define SPP_DIRECT_MMAP(s)          mfail
    #define SPP_CALL_DIRECT_MMAP(s)     SPP_DIRECT_MMAP(s)
    #define SPP_CALL_MMAP(s)            SPP_MMAP(s)
    #define SPP_CALL_MUNMAP(a, s)       SPP_MUNMAP((a), (s))
#endif


#if SPP_HAVE_MMAP && SPP_HAVE_MREMAP
    #ifdef MREMAP
        #define SPP_CALL_MREMAP(addr, osz, nsz, mv) MREMAP((addr), (osz), (nsz), (mv))
    #else
        #define SPP_CALL_MREMAP(addr, osz, nsz, mv) SPP_MREMAP_DEFAULT((addr), (osz), (nsz), (mv))
    #endif
#else
    #define SPP_CALL_MREMAP(addr, osz, nsz, mv)     mfail
#endif


static const unsigned USE_NONCONTIGUOUS_BIT = 4U;


static const unsigned EXTERN_BIT = 8U;




static const unsigned PINUSE_BIT = 1;
static const unsigned CINUSE_BIT = 2;
static const unsigned FLAG4_BIT  = 4;
static const unsigned INUSE_BITS = (PINUSE_BIT | CINUSE_BIT);
static const unsigned FLAG_BITS  = (PINUSE_BIT | CINUSE_BIT | FLAG4_BIT);



#if SPP_FOOTERS
    static const unsigned CHUNK_OVERHEAD = 2 * sizeof(size_t);
#else
    static const unsigned CHUNK_OVERHEAD = sizeof(size_t);
#endif


static const unsigned SPP_MMAP_CHUNK_OVERHEAD = 2 * sizeof(size_t);


static const unsigned SPP_MMAP_FOOT_PAD = 4 * sizeof(size_t);


struct malloc_chunk_header
{
    void set_size_and_pinuse_of_free_chunk(size_t s)
    {
        _head = s | PINUSE_BIT;
        set_foot(s);
    }

    void set_foot(size_t s)
    {
        ((malloc_chunk_header *)((char*)this + s))->_prev_foot = s;
    }

    
    bool cinuse() const        { return !!(_head & CINUSE_BIT); }
    bool pinuse() const        { return !!(_head & PINUSE_BIT); }
    bool flag4inuse() const    { return !!(_head & FLAG4_BIT); }
    bool is_inuse() const      { return (_head & INUSE_BITS) != PINUSE_BIT; }
    bool is_mmapped() const    { return (_head & INUSE_BITS) == 0; }

    size_t chunksize() const   { return _head & ~(FLAG_BITS); }

    void clear_pinuse()        { _head &= ~PINUSE_BIT; }
    void set_flag4()           { _head |= FLAG4_BIT; }
    void clear_flag4()         { _head &= ~FLAG4_BIT; }

    
    malloc_chunk_header * chunk_plus_offset(size_t s)
    {
        return (malloc_chunk_header *)((char*)this + s);
    }
    malloc_chunk_header * chunk_minus_offset(size_t s)
    {
        return (malloc_chunk_header *)((char*)this - s);
    }

    
    malloc_chunk_header * next_chunk()
    {
        return (malloc_chunk_header *)((char*)this + (_head & ~FLAG_BITS));
    }
    malloc_chunk_header * prev_chunk()
    {
        return (malloc_chunk_header *)((char*)this - (_prev_foot));
    }

    
    size_t next_pinuse()  { return next_chunk()->_head & PINUSE_BIT; }

    size_t   _prev_foot;  
    size_t   _head;       
};


struct malloc_chunk : public malloc_chunk_header
{
    
    void set_free_with_pinuse(size_t s, malloc_chunk* n)
    {
        n->clear_pinuse();
        set_size_and_pinuse_of_free_chunk(s);
    }

    
    size_t overhead_for() { return is_mmapped() ? SPP_MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD; }

    
    bool calloc_must_clear()
    {
#if SPP_MMAP_CLEARS
        return !is_mmapped();
#else
        return true;
#endif
    }

    struct malloc_chunk* _fd;         
    struct malloc_chunk* _bk;
};

static const unsigned MCHUNK_SIZE = sizeof(malloc_chunk);


static const unsigned MIN_CHUNK_SIZE = (MCHUNK_SIZE + spp_chunk_align_mask) & ~spp_chunk_align_mask;

typedef malloc_chunk  mchunk;
typedef malloc_chunk* mchunkptr;
typedef malloc_chunk_header *hchunkptr;
typedef malloc_chunk* sbinptr;         
typedef unsigned int bindex_t;         
typedef unsigned int binmap_t;         
typedef unsigned int flag_t;           


static SPP_FORCEINLINE void *chunk2mem(const void *p)       { return (void *)((char *)p + 2 * sizeof(size_t)); }
static SPP_FORCEINLINE mchunkptr mem2chunk(const void *mem) { return (mchunkptr)((char *)mem - 2 * sizeof(size_t)); }


static SPP_FORCEINLINE mchunkptr align_as_chunk(char *A)    { return (mchunkptr)(A + align_offset(chunk2mem(A))); }


static const unsigned MAX_REQUEST = (-MIN_CHUNK_SIZE) << 2;
static const unsigned MIN_REQUEST = MIN_CHUNK_SIZE - CHUNK_OVERHEAD - 1;


static SPP_FORCEINLINE size_t pad_request(size_t req)
{
    return (req + CHUNK_OVERHEAD + spp_chunk_align_mask) & ~spp_chunk_align_mask;
}


static SPP_FORCEINLINE size_t request2size(size_t req)
{
    return req < MIN_REQUEST ? MIN_CHUNK_SIZE : pad_request(req);
}







static const unsigned FENCEPOST_HEAD = INUSE_BITS | sizeof(size_t);







struct malloc_tree_chunk : public malloc_chunk_header
{
    malloc_tree_chunk *leftmost_child()
    {
        return _child[0] ? _child[0] : _child[1];
    }


    malloc_tree_chunk* _fd;
    malloc_tree_chunk* _bk;

    malloc_tree_chunk* _child[2];
    malloc_tree_chunk* _parent;
    bindex_t           _index;
};

typedef malloc_tree_chunk  tchunk;
typedef malloc_tree_chunk* tchunkptr;
typedef malloc_tree_chunk* tbinptr; 






struct malloc_segment
{
    bool is_mmapped_segment()  { return !!(_sflags & USE_MMAP_BIT); }
    bool is_extern_segment()   { return !!(_sflags & EXTERN_BIT); }

    char*           _base;          
    size_t          _size;          
    malloc_segment* _next;          
    flag_t          _sflags;        
};

typedef malloc_segment  msegment;
typedef malloc_segment* msegmentptr;






struct malloc_params
{
    malloc_params() : _magic(0) {}

    void ensure_initialization()
    {
        if (!_magic)
            _init();
    }
    
    SPP_IMPL int change(int param_number, int value);

    size_t page_align(size_t sz)
    {
        return (sz + (_page_size - 1)) & ~(_page_size - 1);
    }

    size_t granularity_align(size_t sz)
    {
        return (sz + (_granularity - 1)) & ~(_granularity - 1);
    }

    bool is_page_aligned(char *S)
    {
        return ((size_t)S & (_page_size - 1)) == 0;
    }

    SPP_IMPL int _init();

    size_t _magic;
    size_t _page_size;
    size_t _granularity;
    size_t _mmap_threshold;
    size_t _trim_threshold;
    flag_t _default_mflags;
};

static malloc_params mparams;







class malloc_state
{
public:
    
    SPP_FORCEINLINE void* _malloc(size_t bytes);
    SPP_FORCEINLINE void  _free(mchunkptr p);


    
    void *internal_malloc(size_t b) { return mspace_malloc(this, b); }
    void internal_free(void *mem)   { mspace_free(this, mem); }

    

    SPP_IMPL void      init_top(mchunkptr p, size_t psize);
    SPP_IMPL void      init_bins();
    SPP_IMPL void      init(char* tbase, size_t tsize);

    
    SPP_IMPL void*     sys_alloc(size_t nb);
    SPP_IMPL size_t    release_unused_segments();
    SPP_IMPL int       sys_trim(size_t pad);
    SPP_IMPL void      dispose_chunk(mchunkptr p, size_t psize);

    
    SPP_IMPL mchunkptr try_realloc_chunk(mchunkptr p, size_t nb, int can_move);
    SPP_IMPL void*     internal_memalign(size_t alignment, size_t bytes);
    SPP_IMPL void**    ialloc(size_t n_elements, size_t* sizes, int opts, void* chunks[]);
    SPP_IMPL size_t    internal_bulk_free(void* array[], size_t nelem);
    SPP_IMPL void      internal_inspect_all(void(*handler)(void *start, void *end,
                                                           size_t used_bytes, void* callback_arg),
                                            void* arg);

    
    bool      use_lock() const { return false; }
    void      enable_lock()    {}
    void      set_lock(int)    {}
    void      disable_lock()   {}

    bool      use_mmap() const { return !!(_mflags & USE_MMAP_BIT); }
    void      enable_mmap()    { _mflags |=  USE_MMAP_BIT; }

#if SPP_HAVE_MMAP
    void      disable_mmap()   { _mflags &= ~USE_MMAP_BIT; }
#else
    void      disable_mmap()   {}
#endif

    

    


#if !SPP_INSECURE
    
    bool        ok_address(void *a) const { return (char *)a >= _least_addr; }

    
    static bool ok_next(void *p, void *n) { return p < n; }

    
    static bool ok_inuse(mchunkptr p)     { return p->is_inuse(); }

    
    static bool ok_pinuse(mchunkptr p)    { return p->pinuse(); }

    
    bool        ok_magic() const          { return _magic == mparams._magic; }

    
  #if defined(__GNUC__) && __GNUC__ >= 3
    static bool rtcheck(bool e)       { return __builtin_expect(e, 1); }
  #else
    static bool rtcheck(bool e)       { return e; }
  #endif
#else
    static bool ok_address(void *)       { return true; }
    static bool ok_next(void *, void *)  { return true; }
    static bool ok_inuse(mchunkptr)      { return true; }
    static bool ok_pinuse(mchunkptr)     { return true; }
    static bool ok_magic()               { return true; }
    static bool rtcheck(bool)            { return true; }
#endif

    bool is_initialized() const           { return _top != 0; }

    bool use_noncontiguous()  const       { return !!(_mflags & USE_NONCONTIGUOUS_BIT); }
    void disable_contiguous()             { _mflags |=  USE_NONCONTIGUOUS_BIT; }

    
    msegmentptr segment_holding(char* addr) const
    {
        msegmentptr sp = (msegmentptr)&_seg;
        for (;;)
        {
            if (addr >= sp->_base && addr < sp->_base + sp->_size)
                return sp;
            if ((sp = sp->_next) == 0)
                return 0;
        }
    }

    
    int has_segment_link(msegmentptr ss) const
    {
        msegmentptr sp = (msegmentptr)&_seg;
        for (;;)
        {
            if ((char*)sp >= ss->_base && (char*)sp < ss->_base + ss->_size)
                return 1;
            if ((sp = sp->_next) == 0)
                return 0;
        }
    }

    bool should_trim(size_t s) const { return s > _trim_check; }

    

#if ! SPP_DEBUG
    void check_free_chunk(mchunkptr) {}
    void check_inuse_chunk(mchunkptr) {}
    void check_malloced_chunk(void*, size_t) {}
    void check_mmapped_chunk(mchunkptr) {}
    void check_malloc_state() {}
    void check_top_chunk(mchunkptr) {}
#else 
    void check_free_chunk(mchunkptr p)       { do_check_free_chunk(p); }
    void check_inuse_chunk(mchunkptr p)      { do_check_inuse_chunk(p); }
    void check_malloced_chunk(void* p, size_t s) { do_check_malloced_chunk(p, s); }
    void check_mmapped_chunk(mchunkptr p)    { do_check_mmapped_chunk(p); }
    void check_malloc_state()                { do_check_malloc_state(); }
    void check_top_chunk(mchunkptr p)        { do_check_top_chunk(p); }

    void do_check_any_chunk(mchunkptr p) const;
    void do_check_top_chunk(mchunkptr p) const;
    void do_check_mmapped_chunk(mchunkptr p) const;
    void do_check_inuse_chunk(mchunkptr p) const;
    void do_check_free_chunk(mchunkptr p) const;
    void do_check_malloced_chunk(void* mem, size_t s) const;
    void do_check_tree(tchunkptr t);
    void do_check_treebin(bindex_t i);
    void do_check_smallbin(bindex_t i);
    void do_check_malloc_state();
    int  bin_find(mchunkptr x);
    size_t traverse_and_check();
#endif

private:

    

    static bool  is_small(size_t s)          { return (s >> SMALLBIN_SHIFT) < NSMALLBINS; }
    static bindex_t  small_index(size_t s)   { return (bindex_t)(s  >> SMALLBIN_SHIFT); }
    static size_t small_index2size(size_t i) { return i << SMALLBIN_SHIFT; }
    static bindex_t  MIN_SMALL_INDEX()       { return small_index(MIN_CHUNK_SIZE); }

    
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    SPP_FORCEINLINE static bindex_t compute_tree_index(size_t S)
    {
        unsigned int X = S >> TREEBIN_SHIFT;
        if (X == 0)
            return 0;
        else if (X > 0xFFFF)
            return NTREEBINS - 1;

        unsigned int K = (unsigned) sizeof(X) * __CHAR_BIT__ - 1 - (unsigned) __builtin_clz(X);
        return (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT - 1)) & 1)));
    }

#elif defined (__INTEL_COMPILER)
    SPP_FORCEINLINE static bindex_t compute_tree_index(size_t S)
    {
        size_t X = S >> TREEBIN_SHIFT;
        if (X == 0)
            return 0;
        else if (X > 0xFFFF)
            return NTREEBINS - 1;

        unsigned int K = _bit_scan_reverse(X);
        return (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT - 1)) & 1)));
    }

#elif defined(_MSC_VER) && _MSC_VER>=1300
    SPP_FORCEINLINE static bindex_t compute_tree_index(size_t S)
    {
        size_t X = S >> TREEBIN_SHIFT;
        if (X == 0)
            return 0;
        else if (X > 0xFFFF)
            return NTREEBINS - 1;

        unsigned int K;
        _BitScanReverse((DWORD *) &K, (DWORD) X);
        return (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT - 1)) & 1)));
    }

#else 
    SPP_FORCEINLINE static bindex_t compute_tree_index(size_t S)
    {
        size_t X = S >> TREEBIN_SHIFT;
        if (X == 0)
            return 0;
        else if (X > 0xFFFF)
            return NTREEBINS - 1;

        unsigned int Y = (unsigned int)X;
        unsigned int N = ((Y - 0x100) >> 16) & 8;
        unsigned int K = (((Y <<= N) - 0x1000) >> 16) & 4;
        N += K;
        N += K = (((Y <<= K) - 0x4000) >> 16) & 2;
        K = 14 - N + ((Y <<= K) >> 15);
        return (K << 1) + ((S >> (K + (TREEBIN_SHIFT - 1)) & 1));
    }
#endif

    
    static bindex_t leftshift_for_tree_index(bindex_t i)
    {
        return (i == NTREEBINS - 1) ? 0 :
               ((spp_size_t_bitsize - 1) - ((i >> 1) + TREEBIN_SHIFT - 2));
    }

    
    static bindex_t minsize_for_tree_index(bindex_t i)
    {
        return ((size_t)1 << ((i >> 1) + TREEBIN_SHIFT)) |
               (((size_t)(i & 1)) << ((i >> 1) + TREEBIN_SHIFT - 1));
    }


    
    static binmap_t least_bit(binmap_t x) { return x & -x; }

    
    static binmap_t left_bits(binmap_t x) { return (x << 1) | -(x << 1); }

    
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    static bindex_t compute_bit2idx(binmap_t X)
    {
        unsigned int J;
        J = __builtin_ctz(X);
        return (bindex_t)J;
    }

#elif defined (__INTEL_COMPILER)
    static bindex_t compute_bit2idx(binmap_t X)
    {
        unsigned int J;
        J = _bit_scan_forward(X);
        return (bindex_t)J;
    }

#elif defined(_MSC_VER) && _MSC_VER>=1300
    static bindex_t compute_bit2idx(binmap_t X)
    {
        unsigned int J;
        _BitScanForward((DWORD *) &J, X);
        return (bindex_t)J;
    }

#elif SPP_USE_BUILTIN_FFS
    static bindex_t compute_bit2idx(binmap_t X) { return ffs(X) - 1; }

#else
    static bindex_t compute_bit2idx(binmap_t X)
    {
        unsigned int Y = X - 1;
        unsigned int K = Y >> (16 - 4) & 16;
        unsigned int N = K;        Y >>= K;
        N += K = Y >> (8 - 3) &  8;  Y >>= K;
        N += K = Y >> (4 - 2) &  4;  Y >>= K;
        N += K = Y >> (2 - 1) &  2;  Y >>= K;
        N += K = Y >> (1 - 0) &  1;  Y >>= K;
        return (bindex_t)(N + Y);
    }
#endif

    
#if !SPP_FOOTERS
    void mark_inuse_foot(malloc_chunk_header *, size_t) {}
#else
    
    void  mark_inuse_foot(malloc_chunk_header *p, size_t s)
    {
        (((mchunkptr)((char*)p + s))->prev_foot = (size_t)this ^ mparams._magic);
    }
#endif

    void set_inuse(malloc_chunk_header *p, size_t s)
    {
        p->_head = (p->_head & PINUSE_BIT) | s | CINUSE_BIT;
        ((mchunkptr)(((char*)p) + s))->_head |= PINUSE_BIT;
        mark_inuse_foot(p, s);
    }

    void set_inuse_and_pinuse(malloc_chunk_header *p, size_t s)
    {
        p->_head = s | PINUSE_BIT | CINUSE_BIT;
        ((mchunkptr)(((char*)p) + s))->_head |= PINUSE_BIT;
        mark_inuse_foot(p, s);
    }

    void set_size_and_pinuse_of_inuse_chunk(malloc_chunk_header *p, size_t s)
    {
        p->_head = s | PINUSE_BIT | CINUSE_BIT;
        mark_inuse_foot(p, s);
    }

    
    sbinptr  smallbin_at(bindex_t i) const { return (sbinptr)((char*)&_smallbins[i << 1]); }
    tbinptr* treebin_at(bindex_t i)  { return &_treebins[i]; }

    
    static binmap_t idx2bit(bindex_t i) { return ((binmap_t)1 << i); }

    
    void     mark_smallmap(bindex_t i)      { _smallmap |=  idx2bit(i); }
    void     clear_smallmap(bindex_t i)     { _smallmap &= ~idx2bit(i); }
    binmap_t smallmap_is_marked(bindex_t i) const { return _smallmap & idx2bit(i); }

    void     mark_treemap(bindex_t i)       { _treemap  |=  idx2bit(i); }
    void     clear_treemap(bindex_t i)      { _treemap  &= ~idx2bit(i); }
    binmap_t treemap_is_marked(bindex_t i)  const { return _treemap & idx2bit(i); }

    
    SPP_FORCEINLINE void insert_small_chunk(mchunkptr P, size_t S);
    SPP_FORCEINLINE void unlink_small_chunk(mchunkptr P, size_t S);
    SPP_FORCEINLINE void unlink_first_small_chunk(mchunkptr B, mchunkptr P, bindex_t I);
    SPP_FORCEINLINE void replace_dv(mchunkptr P, size_t S);

    
    SPP_FORCEINLINE void insert_large_chunk(tchunkptr X, size_t S);
    SPP_FORCEINLINE void unlink_large_chunk(tchunkptr X);

    
    SPP_FORCEINLINE void insert_chunk(mchunkptr P, size_t S);
    SPP_FORCEINLINE void unlink_chunk(mchunkptr P, size_t S);

    
    SPP_IMPL void*       mmap_alloc(size_t nb);
    SPP_IMPL mchunkptr   mmap_resize(mchunkptr oldp, size_t nb, int flags);

    SPP_IMPL void        reset_on_error();
    SPP_IMPL void*       prepend_alloc(char* newbase, char* oldbase, size_t nb);
    SPP_IMPL void        add_segment(char* tbase, size_t tsize, flag_t mmapped);

    
    SPP_IMPL void*       tmalloc_large(size_t nb);
    SPP_IMPL void*       tmalloc_small(size_t nb);

    
    static const size_t NSMALLBINS      = 32;
    static const size_t NTREEBINS       = 32;
    static const size_t SMALLBIN_SHIFT  = 3;
    static const size_t SMALLBIN_WIDTH  = 1 << SMALLBIN_SHIFT;
    static const size_t TREEBIN_SHIFT   = 8;
    static const size_t MIN_LARGE_SIZE  = 1 << TREEBIN_SHIFT;
    static const size_t MAX_SMALL_SIZE  = (MIN_LARGE_SIZE - 1);
    static const size_t MAX_SMALL_REQUEST = (MAX_SMALL_SIZE - spp_chunk_align_mask - CHUNK_OVERHEAD);

    
    binmap_t   _smallmap;
    binmap_t   _treemap;
    size_t     _dvsize;
    size_t     _topsize;
    char*      _least_addr;
    mchunkptr  _dv;
    mchunkptr  _top;
    size_t     _trim_check;
    size_t     _release_checks;
    size_t     _magic;
    mchunkptr  _smallbins[(NSMALLBINS + 1) * 2];
    tbinptr    _treebins[NTREEBINS];
public:
    size_t     _footprint;
    size_t     _max_footprint;
    size_t     _footprint_limit; 
    flag_t     _mflags;

    msegment   _seg;

private:
    void*      _extp;      
    size_t     _exts;
};

typedef malloc_state*    mstate;



#if SPP_FOOTERS
static malloc_state* get_mstate_for(malloc_chunk_header *p)
{
    return (malloc_state*)(((mchunkptr)((char*)(p) +
                                        (p->chunksize())))->prev_foot ^ mparams._magic);
}
#endif






#ifdef WIN32
    #define mmap_align(S) mparams.granularity_align(S)
#else
    #define mmap_align(S) mparams.page_align(S)
#endif


static bool segment_holds(msegmentptr S, mchunkptr A)
{
    return (char*)A >= S->_base && (char*)A < S->_base + S->_size;
}


static SPP_FORCEINLINE size_t top_foot_size()
{
    return align_offset(chunk2mem((void *)0)) + 
        pad_request(sizeof(struct malloc_segment)) + 
        MIN_CHUNK_SIZE;
}



static SPP_FORCEINLINE size_t sys_alloc_padding()
{
    return  top_foot_size() + SPP_MALLOC_ALIGNMENT;
}


#define SPP_USAGE_ERROR_ACTION(m,p) SPP_ABORT




int malloc_params::_init()
{
#ifdef NEED_GLOBAL_LOCK_INIT
    if (malloc_global_mutex_status <= 0)
        init_malloc_global_mutex();
#endif

    if (_magic == 0)
    {
        size_t magic;
        size_t psize;
        size_t gsize;

#ifndef WIN32
        psize = malloc_getpagesize;
        gsize = ((SPP_DEFAULT_GRANULARITY != 0) ? SPP_DEFAULT_GRANULARITY : psize);
#else
        {
            SYSTEM_INFO system_info;
            GetSystemInfo(&system_info);
            psize = system_info.dwPageSize;
            gsize = ((SPP_DEFAULT_GRANULARITY != 0) ?
                     SPP_DEFAULT_GRANULARITY : system_info.dwAllocationGranularity);
        }
#endif

        
        if ((sizeof(size_t) != sizeof(char*)) ||
                (spp_max_size_t < MIN_CHUNK_SIZE)  ||
                (sizeof(int) < 4)  ||
                (SPP_MALLOC_ALIGNMENT < (size_t)8U) ||
                ((SPP_MALLOC_ALIGNMENT & (SPP_MALLOC_ALIGNMENT - 1)) != 0) ||
                ((MCHUNK_SIZE      & (MCHUNK_SIZE - 1))      != 0) ||
                ((gsize            & (gsize - 1))            != 0) ||
                ((psize            & (psize - 1))            != 0))
            SPP_ABORT;
        _granularity = gsize;
        _page_size = psize;
        _mmap_threshold = SPP_DEFAULT_MMAP_THRESHOLD;
        _trim_threshold = SPP_DEFAULT_TRIM_THRESHOLD;
        _default_mflags = USE_MMAP_BIT | USE_NONCONTIGUOUS_BIT;

        {
#if SPP_USE_DEV_RANDOM
            int fd;
            unsigned char buf[sizeof(size_t)];
            
            if ((fd = open("/dev/urandom", O_RDONLY)) >= 0 &&
                    read(fd, buf, sizeof(buf)) == sizeof(buf))
            {
                magic = *((size_t *) buf);
                close(fd);
            }
            else
#endif
            {
#ifdef WIN32
                magic = (size_t)(GetTickCount() ^ (size_t)0x55555555U);
#elif defined(SPP_LACKS_TIME_H)
                magic = (size_t)&magic ^ (size_t)0x55555555U;
#else
                magic = (size_t)(time(0) ^ (size_t)0x55555555U);
#endif
            }
            magic |= (size_t)8U;    
            magic &= ~(size_t)7U;   
            
            (*(volatile size_t *)(&(_magic))) = magic;
        }
    }

    return 1;
}


static const int  m_trim_threshold = -1;
static const int  m_granularity    = -2;
static const int  m_mmap_threshold = -3;


int malloc_params::change(int param_number, int value)
{
    size_t val;
    ensure_initialization();
    val = (value == -1) ? spp_max_size_t : (size_t)value;

    switch (param_number)
    {
    case m_trim_threshold:
        _trim_threshold = val;
        return 1;

    case m_granularity:
        if (val >= _page_size && ((val & (val - 1)) == 0))
        {
            _granularity = val;
            return 1;
        }
        else
            return 0;

    case m_mmap_threshold:
        _mmap_threshold = val;
        return 1;

    default:
        return 0;
    }
}

#if SPP_DEBUG



void malloc_state::do_check_any_chunk(mchunkptr p)  const
{
    assert((spp_is_aligned(chunk2mem(p))) || (p->_head == FENCEPOST_HEAD));
    assert(ok_address(p));
}


void malloc_state::do_check_top_chunk(mchunkptr p) const
{
    msegmentptr sp = segment_holding((char*)p);
    size_t  sz = p->_head & ~INUSE_BITS; 
    assert(sp != 0);
    assert((spp_is_aligned(chunk2mem(p))) || (p->_head == FENCEPOST_HEAD));
    assert(ok_address(p));
    assert(sz == _topsize);
    assert(sz > 0);
    assert(sz == ((sp->_base + sp->_size) - (char*)p) - top_foot_size());
    assert(p->pinuse());
    assert(!p->chunk_plus_offset(sz)->pinuse());
}


void malloc_state::do_check_mmapped_chunk(mchunkptr p) const
{
    size_t  sz = p->chunksize();
    size_t len = (sz + (p->_prev_foot) + SPP_MMAP_FOOT_PAD);
    assert(p->is_mmapped());
    assert(use_mmap());
    assert((spp_is_aligned(chunk2mem(p))) || (p->_head == FENCEPOST_HEAD));
    assert(ok_address(p));
    assert(!is_small(sz));
    assert((len & (mparams._page_size - 1)) == 0);
    assert(p->chunk_plus_offset(sz)->_head == FENCEPOST_HEAD);
    assert(p->chunk_plus_offset(sz + sizeof(size_t))->_head == 0);
}


void malloc_state::do_check_inuse_chunk(mchunkptr p) const
{
    do_check_any_chunk(p);
    assert(p->is_inuse());
    assert(p->next_pinuse());
    
    assert(p->is_mmapped() || p->pinuse() || (mchunkptr)p->prev_chunk()->next_chunk() == p);
    if (p->is_mmapped())
        do_check_mmapped_chunk(p);
}


void malloc_state::do_check_free_chunk(mchunkptr p) const
{
    size_t sz = p->chunksize();
    mchunkptr next = (mchunkptr)p->chunk_plus_offset(sz);
    do_check_any_chunk(p);
    assert(!p->is_inuse());
    assert(!p->next_pinuse());
    assert(!p->is_mmapped());
    if (p != _dv && p != _top)
    {
        if (sz >= MIN_CHUNK_SIZE)
        {
            assert((sz & spp_chunk_align_mask) == 0);
            assert(spp_is_aligned(chunk2mem(p)));
            assert(next->_prev_foot == sz);
            assert(p->pinuse());
            assert(next == _top || next->is_inuse());
            assert(p->_fd->_bk == p);
            assert(p->_bk->_fd == p);
        }
        else  
            assert(sz == sizeof(size_t));
    }
}


void malloc_state::do_check_malloced_chunk(void* mem, size_t s) const
{
    if (mem != 0)
    {
        mchunkptr p = mem2chunk(mem);
        size_t sz = p->_head & ~INUSE_BITS;
        do_check_inuse_chunk(p);
        assert((sz & spp_chunk_align_mask) == 0);
        assert(sz >= MIN_CHUNK_SIZE);
        assert(sz >= s);
        
        assert(p->is_mmapped() || sz < (s + MIN_CHUNK_SIZE));
    }
}


void malloc_state::do_check_tree(tchunkptr t)
{
    tchunkptr head = 0;
    tchunkptr u = t;
    bindex_t tindex = t->_index;
    size_t tsize = t->chunksize();
    bindex_t idx = compute_tree_index(tsize);
    assert(tindex == idx);
    assert(tsize >= MIN_LARGE_SIZE);
    assert(tsize >= minsize_for_tree_index(idx));
    assert((idx == NTREEBINS - 1) || (tsize < minsize_for_tree_index((idx + 1))));

    do
    {
        
        do_check_any_chunk((mchunkptr)u);
        assert(u->_index == tindex);
        assert(u->chunksize() == tsize);
        assert(!u->is_inuse());
        assert(!u->next_pinuse());
        assert(u->_fd->_bk == u);
        assert(u->_bk->_fd == u);
        if (u->_parent == 0)
        {
            assert(u->_child[0] == 0);
            assert(u->_child[1] == 0);
        }
        else
        {
            assert(head == 0); 
            head = u;
            assert(u->_parent != u);
            assert(u->_parent->_child[0] == u ||
                   u->_parent->_child[1] == u ||
                   *((tbinptr*)(u->_parent)) == u);
            if (u->_child[0] != 0)
            {
                assert(u->_child[0]->_parent == u);
                assert(u->_child[0] != u);
                do_check_tree(u->_child[0]);
            }
            if (u->_child[1] != 0)
            {
                assert(u->_child[1]->_parent == u);
                assert(u->_child[1] != u);
                do_check_tree(u->_child[1]);
            }
            if (u->_child[0] != 0 && u->_child[1] != 0)
                assert(u->_child[0]->chunksize() < u->_child[1]->chunksize());
        }
        u = u->_fd;
    }
    while (u != t);
    assert(head != 0);
}


void malloc_state::do_check_treebin(bindex_t i)
{
    tbinptr* tb = (tbinptr*)treebin_at(i);
    tchunkptr t = *tb;
    int empty = (_treemap & (1U << i)) == 0;
    if (t == 0)
        assert(empty);
    if (!empty)
        do_check_tree(t);
}


void malloc_state::do_check_smallbin(bindex_t i)
{
    sbinptr b = smallbin_at(i);
    mchunkptr p = b->_bk;
    unsigned int empty = (_smallmap & (1U << i)) == 0;
    if (p == b)
        assert(empty);
    if (!empty)
    {
        for (; p != b; p = p->_bk)
        {
            size_t size = p->chunksize();
            mchunkptr q;
            
            do_check_free_chunk(p);
            
            assert(small_index(size) == i);
            assert(p->_bk == b || p->_bk->chunksize() == p->chunksize());
            
            q = (mchunkptr)p->next_chunk();
            if (q->_head != FENCEPOST_HEAD)
                do_check_inuse_chunk(q);
        }
    }
}


int malloc_state::bin_find(mchunkptr x)
{
    size_t size = x->chunksize();
    if (is_small(size))
    {
        bindex_t sidx = small_index(size);
        sbinptr b = smallbin_at(sidx);
        if (smallmap_is_marked(sidx))
        {
            mchunkptr p = b;
            do
            {
                if (p == x)
                    return 1;
            }
            while ((p = p->_fd) != b);
        }
    }
    else
    {
        bindex_t tidx = compute_tree_index(size);
        if (treemap_is_marked(tidx))
        {
            tchunkptr t = *treebin_at(tidx);
            size_t sizebits = size << leftshift_for_tree_index(tidx);
            while (t != 0 && t->chunksize() != size)
            {
                t = t->_child[(sizebits >> (spp_size_t_bitsize - 1)) & 1];
                sizebits <<= 1;
            }
            if (t != 0)
            {
                tchunkptr u = t;
                do
                {
                    if (u == (tchunkptr)x)
                        return 1;
                }
                while ((u = u->_fd) != t);
            }
        }
    }
    return 0;
}


size_t malloc_state::traverse_and_check()
{
    size_t sum = 0;
    if (is_initialized())
    {
        msegmentptr s = (msegmentptr)&_seg;
        sum += _topsize + top_foot_size();
        while (s != 0)
        {
            mchunkptr q = align_as_chunk(s->_base);
            mchunkptr lastq = 0;
            assert(q->pinuse());
            while (segment_holds(s, q) &&
                    q != _top && q->_head != FENCEPOST_HEAD)
            {
                sum += q->chunksize();
                if (q->is_inuse())
                {
                    assert(!bin_find(q));
                    do_check_inuse_chunk(q);
                }
                else
                {
                    assert(q == _dv || bin_find(q));
                    assert(lastq == 0 || lastq->is_inuse()); 
                    do_check_free_chunk(q);
                }
                lastq = q;
                q = (mchunkptr)q->next_chunk();
            }
            s = s->_next;
        }
    }
    return sum;
}



void malloc_state::do_check_malloc_state()
{
    bindex_t i;
    size_t total;
    
    for (i = 0; i < NSMALLBINS; ++i)
        do_check_smallbin(i);
    for (i = 0; i < NTREEBINS; ++i)
        do_check_treebin(i);

    if (_dvsize != 0)
    {
        
        do_check_any_chunk(_dv);
        assert(_dvsize == _dv->chunksize());
        assert(_dvsize >= MIN_CHUNK_SIZE);
        assert(bin_find(_dv) == 0);
    }

    if (_top != 0)
    {
        
        do_check_top_chunk(_top);
        
        assert(_topsize > 0);
        assert(bin_find(_top) == 0);
    }

    total = traverse_and_check();
    assert(total <= _footprint);
    assert(_footprint <= _max_footprint);
}
#endif 






void malloc_state::insert_small_chunk(mchunkptr p, size_t s)
{
    bindex_t I  = small_index(s);
    mchunkptr B = smallbin_at(I);
    mchunkptr F = B;
    assert(s >= MIN_CHUNK_SIZE);
    if (!smallmap_is_marked(I))
        mark_smallmap(I);
    else if (rtcheck(ok_address(B->_fd)))
        F = B->_fd;
    else
        SPP_ABORT;
    B->_fd = p;
    F->_bk = p;
    p->_fd = F;
    p->_bk = B;
}


void malloc_state::unlink_small_chunk(mchunkptr p, size_t s)
{
    mchunkptr F = p->_fd;
    mchunkptr B = p->_bk;
    bindex_t I = small_index(s);
    assert(p != B);
    assert(p != F);
    assert(p->chunksize() == small_index2size(I));
    if (rtcheck(F == smallbin_at(I) || (ok_address(F) && F->_bk == p)))
    {
        if (B == F)
            clear_smallmap(I);
        else if (rtcheck(B == smallbin_at(I) ||
                         (ok_address(B) && B->_fd == p)))
        {
            F->_bk = B;
            B->_fd = F;
        }
        else
            SPP_ABORT;
    }
    else
        SPP_ABORT;
}


void malloc_state::unlink_first_small_chunk(mchunkptr B, mchunkptr p, bindex_t I)
{
    mchunkptr F = p->_fd;
    assert(p != B);
    assert(p != F);
    assert(p->chunksize() == small_index2size(I));
    if (B == F)
        clear_smallmap(I);
    else if (rtcheck(ok_address(F) && F->_bk == p))
    {
        F->_bk = B;
        B->_fd = F;
    }
    else
        SPP_ABORT;
}



void malloc_state::replace_dv(mchunkptr p, size_t s)
{
    size_t DVS = _dvsize;
    assert(is_small(DVS));
    if (DVS != 0)
    {
        mchunkptr DV = _dv;
        insert_small_chunk(DV, DVS);
    }
    _dvsize = s;
    _dv = p;
}




void malloc_state::insert_large_chunk(tchunkptr X, size_t s)
{
    tbinptr* H;
    bindex_t I = compute_tree_index(s);
    H = treebin_at(I);
    X->_index = I;
    X->_child[0] = X->_child[1] = 0;
    if (!treemap_is_marked(I))
    {
        mark_treemap(I);
        *H = X;
        X->_parent = (tchunkptr)H;
        X->_fd = X->_bk = X;
    }
    else
    {
        tchunkptr T = *H;
        size_t K = s << leftshift_for_tree_index(I);
        for (;;)
        {
            if (T->chunksize() != s)
            {
                tchunkptr* C = &(T->_child[(K >> (spp_size_t_bitsize - 1)) & 1]);
                K <<= 1;
                if (*C != 0)
                    T = *C;
                else if (rtcheck(ok_address(C)))
                {
                    *C = X;
                    X->_parent = T;
                    X->_fd = X->_bk = X;
                    break;
                }
                else
                {
                    SPP_ABORT;
                    break;
                }
            }
            else
            {
                tchunkptr F = T->_fd;
                if (rtcheck(ok_address(T) && ok_address(F)))
                {
                    T->_fd = F->_bk = X;
                    X->_fd = F;
                    X->_bk = T;
                    X->_parent = 0;
                    break;
                }
                else
                {
                    SPP_ABORT;
                    break;
                }
            }
        }
    }
}



void malloc_state::unlink_large_chunk(tchunkptr X)
{
    tchunkptr XP = X->_parent;
    tchunkptr R;
    if (X->_bk != X)
    {
        tchunkptr F = X->_fd;
        R = X->_bk;
        if (rtcheck(ok_address(F) && F->_bk == X && R->_fd == X))
        {
            F->_bk = R;
            R->_fd = F;
        }
        else
            SPP_ABORT;
    }
    else
    {
        tchunkptr* RP;
        if (((R = *(RP = &(X->_child[1]))) != 0) ||
                ((R = *(RP = &(X->_child[0]))) != 0))
        {
            tchunkptr* CP;
            while ((*(CP = &(R->_child[1])) != 0) ||
                    (*(CP = &(R->_child[0])) != 0))
                R = *(RP = CP);
            if (rtcheck(ok_address(RP)))
                *RP = 0;
            else
                SPP_ABORT;
        }
    }
    if (XP != 0)
    {
        tbinptr* H = treebin_at(X->_index);
        if (X == *H)
        {
            if ((*H = R) == 0)
                clear_treemap(X->_index);
        }
        else if (rtcheck(ok_address(XP)))
        {
            if (XP->_child[0] == X)
                XP->_child[0] = R;
            else
                XP->_child[1] = R;
        }
        else
            SPP_ABORT;
        if (R != 0)
        {
            if (rtcheck(ok_address(R)))
            {
                tchunkptr C0, C1;
                R->_parent = XP;
                if ((C0 = X->_child[0]) != 0)
                {
                    if (rtcheck(ok_address(C0)))
                    {
                        R->_child[0] = C0;
                        C0->_parent = R;
                    }
                    else
                        SPP_ABORT;
                }
                if ((C1 = X->_child[1]) != 0)
                {
                    if (rtcheck(ok_address(C1)))
                    {
                        R->_child[1] = C1;
                        C1->_parent = R;
                    }
                    else
                        SPP_ABORT;
                }
            }
            else
                SPP_ABORT;
        }
    }
}



void malloc_state::insert_chunk(mchunkptr p, size_t s)
{
    if (is_small(s))
        insert_small_chunk(p, s);
    else
    {
        tchunkptr tp = (tchunkptr)(p);
        insert_large_chunk(tp, s);
    }
}

void malloc_state::unlink_chunk(mchunkptr p, size_t s)
{
    if (is_small(s))
        unlink_small_chunk(p, s);
    else
    {
        tchunkptr tp = (tchunkptr)(p);
        unlink_large_chunk(tp);
    }
}







void* malloc_state::mmap_alloc(size_t nb)
{
    size_t mmsize = mmap_align(nb + 6 * sizeof(size_t) + spp_chunk_align_mask);
    if (_footprint_limit != 0)
    {
        size_t fp = _footprint + mmsize;
        if (fp <= _footprint || fp > _footprint_limit)
            return 0;
    }
    if (mmsize > nb)
    {
        
        char* mm = (char*)(SPP_CALL_DIRECT_MMAP(mmsize));
        if (mm != cmfail)
        {
            size_t offset = align_offset(chunk2mem(mm));
            size_t psize = mmsize - offset - SPP_MMAP_FOOT_PAD;
            mchunkptr p = (mchunkptr)(mm + offset);
            p->_prev_foot = offset;
            p->_head = psize;
            mark_inuse_foot(p, psize);
            p->chunk_plus_offset(psize)->_head = FENCEPOST_HEAD;
            p->chunk_plus_offset(psize + sizeof(size_t))->_head = 0;

            if (_least_addr == 0 || mm < _least_addr)
                _least_addr = mm;
            if ((_footprint += mmsize) > _max_footprint)
                _max_footprint = _footprint;
            assert(spp_is_aligned(chunk2mem(p)));
            check_mmapped_chunk(p);
            return chunk2mem(p);
        }
    }
    return 0;
}


mchunkptr malloc_state::mmap_resize(mchunkptr oldp, size_t nb, int flags)
{
    size_t oldsize = oldp->chunksize();
    (void)flags;      
    if (is_small(nb)) 
        return 0;

    
    if (oldsize >= nb + sizeof(size_t) &&
            (oldsize - nb) <= (mparams._granularity << 1))
        return oldp;
    else
    {
        size_t offset = oldp->_prev_foot;
        size_t oldmmsize = oldsize + offset + SPP_MMAP_FOOT_PAD;
        size_t newmmsize = mmap_align(nb + 6 * sizeof(size_t) + spp_chunk_align_mask);
        char* cp = (char*)SPP_CALL_MREMAP((char*)oldp - offset,
                                      oldmmsize, newmmsize, flags);
        if (cp != cmfail)
        {
            mchunkptr newp = (mchunkptr)(cp + offset);
            size_t psize = newmmsize - offset - SPP_MMAP_FOOT_PAD;
            newp->_head = psize;
            mark_inuse_foot(newp, psize);
            newp->chunk_plus_offset(psize)->_head = FENCEPOST_HEAD;
            newp->chunk_plus_offset(psize + sizeof(size_t))->_head = 0;

            if (cp < _least_addr)
                _least_addr = cp;
            if ((_footprint += newmmsize - oldmmsize) > _max_footprint)
                _max_footprint = _footprint;
            check_mmapped_chunk(newp);
            return newp;
        }
    }
    return 0;
}





void malloc_state::init_top(mchunkptr p, size_t psize)
{
    
    size_t offset = align_offset(chunk2mem(p));
    p = (mchunkptr)((char*)p + offset);
    psize -= offset;

    _top = p;
    _topsize = psize;
    p->_head = psize | PINUSE_BIT;
    
    p->chunk_plus_offset(psize)->_head = top_foot_size();
    _trim_check = mparams._trim_threshold; 
}


void malloc_state::init_bins()
{
    
    bindex_t i;
    for (i = 0; i < NSMALLBINS; ++i)
    {
        sbinptr bin = smallbin_at(i);
        bin->_fd = bin->_bk = bin;
    }
}

#if SPP_PROCEED_ON_ERROR


void malloc_state::reset_on_error()
{
    int i;
    ++malloc_corruption_error_count;
    
    _smallmap = _treemap = 0;
    _dvsize = _topsize = 0;
    _seg._base = 0;
    _seg._size = 0;
    _seg._next = 0;
    _top = _dv = 0;
    for (i = 0; i < NTREEBINS; ++i)
        *treebin_at(i) = 0;
    init_bins();
}
#endif


void* malloc_state::prepend_alloc(char* newbase, char* oldbase, size_t nb)
{
    mchunkptr p = align_as_chunk(newbase);
    mchunkptr oldfirst = align_as_chunk(oldbase);
    size_t psize = (char*)oldfirst - (char*)p;
    mchunkptr q = (mchunkptr)p->chunk_plus_offset(nb);
    size_t qsize = psize - nb;
    set_size_and_pinuse_of_inuse_chunk(p, nb);

    assert((char*)oldfirst > (char*)q);
    assert(oldfirst->pinuse());
    assert(qsize >= MIN_CHUNK_SIZE);

    
    if (oldfirst == _top)
    {
        size_t tsize = _topsize += qsize;
        _top = q;
        q->_head = tsize | PINUSE_BIT;
        check_top_chunk(q);
    }
    else if (oldfirst == _dv)
    {
        size_t dsize = _dvsize += qsize;
        _dv = q;
        q->set_size_and_pinuse_of_free_chunk(dsize);
    }
    else
    {
        if (!oldfirst->is_inuse())
        {
            size_t nsize = oldfirst->chunksize();
            unlink_chunk(oldfirst, nsize);
            oldfirst = (mchunkptr)oldfirst->chunk_plus_offset(nsize);
            qsize += nsize;
        }
        q->set_free_with_pinuse(qsize, oldfirst);
        insert_chunk(q, qsize);
        check_free_chunk(q);
    }

    check_malloced_chunk(chunk2mem(p), nb);
    return chunk2mem(p);
}


void malloc_state::add_segment(char* tbase, size_t tsize, flag_t mmapped)
{
    
    char* old_top = (char*)_top;
    msegmentptr oldsp = segment_holding(old_top);
    char* old_end = oldsp->_base + oldsp->_size;
    size_t ssize = pad_request(sizeof(struct malloc_segment));
    char* rawsp = old_end - (ssize + 4 * sizeof(size_t) + spp_chunk_align_mask);
    size_t offset = align_offset(chunk2mem(rawsp));
    char* asp = rawsp + offset;
    char* csp = (asp < (old_top + MIN_CHUNK_SIZE)) ? old_top : asp;
    mchunkptr sp = (mchunkptr)csp;
    msegmentptr ss = (msegmentptr)(chunk2mem(sp));
    mchunkptr tnext = (mchunkptr)sp->chunk_plus_offset(ssize);
    mchunkptr p = tnext;
    int nfences = 0;

    
    init_top((mchunkptr)tbase, tsize - top_foot_size());

    
    assert(spp_is_aligned(ss));
    set_size_and_pinuse_of_inuse_chunk(sp, ssize);
    *ss = _seg; 
    _seg._base = tbase;
    _seg._size = tsize;
    _seg._sflags = mmapped;
    _seg._next = ss;

    
    for (;;)
    {
        mchunkptr nextp = (mchunkptr)p->chunk_plus_offset(sizeof(size_t));
        p->_head = FENCEPOST_HEAD;
        ++nfences;
        if ((char*)(&(nextp->_head)) < old_end)
            p = nextp;
        else
            break;
    }
    assert(nfences >= 2);

    
    if (csp != old_top)
    {
        mchunkptr q = (mchunkptr)old_top;
        size_t psize = csp - old_top;
        mchunkptr tn = (mchunkptr)q->chunk_plus_offset(psize);
        q->set_free_with_pinuse(psize, tn);
        insert_chunk(q, psize);
    }

    check_top_chunk(_top);
}




void* malloc_state::sys_alloc(size_t nb)
{
    char* tbase = cmfail;
    size_t tsize = 0;
    flag_t mmap_flag = 0;
    size_t asize; 

    mparams.ensure_initialization();

    
    if (use_mmap() && nb >= mparams._mmap_threshold && _topsize != 0)
    {
        void* mem = mmap_alloc(nb);
        if (mem != 0)
            return mem;
    }

    asize = mparams.granularity_align(nb + sys_alloc_padding());
    if (asize <= nb)
        return 0; 
    if (_footprint_limit != 0)
    {
        size_t fp = _footprint + asize;
        if (fp <= _footprint || fp > _footprint_limit)
            return 0;
    }

    

    if (SPP_HAVE_MMAP && tbase == cmfail)
    {
        
        char* mp = (char*)(SPP_CALL_MMAP(asize));
        if (mp != cmfail)
        {
            tbase = mp;
            tsize = asize;
            mmap_flag = USE_MMAP_BIT;
        }
    }

    if (tbase != cmfail)
    {

        if ((_footprint += tsize) > _max_footprint)
            _max_footprint = _footprint;

        if (!is_initialized())
        {
            
            if (_least_addr == 0 || tbase < _least_addr)
                _least_addr = tbase;
            _seg._base = tbase;
            _seg._size = tsize;
            _seg._sflags = mmap_flag;
            _magic = mparams._magic;
            _release_checks = SPP_MAX_RELEASE_CHECK_RATE;
            init_bins();

            
            mchunkptr mn = (mchunkptr)mem2chunk(this)->next_chunk();
            init_top(mn, (size_t)((tbase + tsize) - (char*)mn) - top_foot_size());
        }

        else
        {
            
            msegmentptr sp = &_seg;
            
            while (sp != 0 && tbase != sp->_base + sp->_size)
                sp = (SPP_NO_SEGMENT_TRAVERSAL) ? 0 : sp->_next;
            if (sp != 0 &&
                    !sp->is_extern_segment() &&
                    (sp->_sflags & USE_MMAP_BIT) == mmap_flag &&
                    segment_holds(sp, _top))
            {
                
                sp->_size += tsize;
                init_top(_top, _topsize + tsize);
            }
            else
            {
                if (tbase < _least_addr)
                    _least_addr = tbase;
                sp = &_seg;
                while (sp != 0 && sp->_base != tbase + tsize)
                    sp = (SPP_NO_SEGMENT_TRAVERSAL) ? 0 : sp->_next;
                if (sp != 0 &&
                        !sp->is_extern_segment() &&
                        (sp->_sflags & USE_MMAP_BIT) == mmap_flag)
                {
                    char* oldbase = sp->_base;
                    sp->_base = tbase;
                    sp->_size += tsize;
                    return prepend_alloc(tbase, oldbase, nb);
                }
                else
                    add_segment(tbase, tsize, mmap_flag);
            }
        }

        if (nb < _topsize)
        {
            
            size_t rsize = _topsize -= nb;
            mchunkptr p = _top;
            mchunkptr r = _top = (mchunkptr)p->chunk_plus_offset(nb);
            r->_head = rsize | PINUSE_BIT;
            set_size_and_pinuse_of_inuse_chunk(p, nb);
            check_top_chunk(_top);
            check_malloced_chunk(chunk2mem(p), nb);
            return chunk2mem(p);
        }
    }

    SPP_MALLOC_FAILURE_ACTION;
    return 0;
}




size_t malloc_state::release_unused_segments()
{
    size_t released = 0;
    int nsegs = 0;
    msegmentptr pred = &_seg;
    msegmentptr sp = pred->_next;
    while (sp != 0)
    {
        char* base = sp->_base;
        size_t size = sp->_size;
        msegmentptr next = sp->_next;
        ++nsegs;
        if (sp->is_mmapped_segment() && !sp->is_extern_segment())
        {
            mchunkptr p = align_as_chunk(base);
            size_t psize = p->chunksize();
            
            if (!p->is_inuse() && (char*)p + psize >= base + size - top_foot_size())
            {
                tchunkptr tp = (tchunkptr)p;
                assert(segment_holds(sp, p));
                if (p == _dv)
                {
                    _dv = 0;
                    _dvsize = 0;
                }
                else
                    unlink_large_chunk(tp);
                if (SPP_CALL_MUNMAP(base, size) == 0)
                {
                    released += size;
                    _footprint -= size;
                    
                    sp = pred;
                    sp->_next = next;
                }
                else
                {
                    
                    insert_large_chunk(tp, psize);
                }
            }
        }
        if (SPP_NO_SEGMENT_TRAVERSAL) 
            break;
        pred = sp;
        sp = next;
    }
    
    _release_checks = (((size_t) nsegs > (size_t) SPP_MAX_RELEASE_CHECK_RATE) ?
                       (size_t) nsegs : (size_t) SPP_MAX_RELEASE_CHECK_RATE);
    return released;
}

int malloc_state::sys_trim(size_t pad)
{
    size_t released = 0;
    mparams.ensure_initialization();
    if (pad < MAX_REQUEST && is_initialized())
    {
        pad += top_foot_size(); 

        if (_topsize > pad)
        {
            
            size_t unit = mparams._granularity;
            size_t extra = ((_topsize - pad + (unit - 1)) / unit -
                            1) * unit;
            msegmentptr sp = segment_holding((char*)_top);

            if (!sp->is_extern_segment())
            {
                if (sp->is_mmapped_segment())
                {
                    if (SPP_HAVE_MMAP &&
                        sp->_size >= extra &&
                        !has_segment_link(sp))
                    {
                        
                        size_t newsize = sp->_size - extra;
                        (void)newsize; 
                        
                        if ((SPP_CALL_MREMAP(sp->_base, sp->_size, newsize, 0) != mfail) ||
                            (SPP_CALL_MUNMAP(sp->_base + newsize, extra) == 0))
                            released = extra;
                    }
                }
            }

            if (released != 0)
            {
                sp->_size -= released;
                _footprint -= released;
                init_top(_top, _topsize - released);
                check_top_chunk(_top);
            }
        }

        
        if (SPP_HAVE_MMAP)
            released += release_unused_segments();

        
        if (released == 0 && _topsize > _trim_check)
            _trim_check = spp_max_size_t;
    }

    return (released != 0) ? 1 : 0;
}


void malloc_state::dispose_chunk(mchunkptr p, size_t psize)
{
    mchunkptr next = (mchunkptr)p->chunk_plus_offset(psize);
    if (!p->pinuse())
    {
        mchunkptr prev;
        size_t prevsize = p->_prev_foot;
        if (p->is_mmapped())
        {
            psize += prevsize + SPP_MMAP_FOOT_PAD;
            if (SPP_CALL_MUNMAP((char*)p - prevsize, psize) == 0)
                _footprint -= psize;
            return;
        }
        prev = (mchunkptr)p->chunk_minus_offset(prevsize);
        psize += prevsize;
        p = prev;
        if (rtcheck(ok_address(prev)))
        {
            
            if (p != _dv)
                unlink_chunk(p, prevsize);
            else if ((next->_head & INUSE_BITS) == INUSE_BITS)
            {
                _dvsize = psize;
                p->set_free_with_pinuse(psize, next);
                return;
            }
        }
        else
        {
            SPP_ABORT;
            return;
        }
    }
    if (rtcheck(ok_address(next)))
    {
        if (!next->cinuse())
        {
            
            if (next == _top)
            {
                size_t tsize = _topsize += psize;
                _top = p;
                p->_head = tsize | PINUSE_BIT;
                if (p == _dv)
                {
                    _dv = 0;
                    _dvsize = 0;
                }
                return;
            }
            else if (next == _dv)
            {
                size_t dsize = _dvsize += psize;
                _dv = p;
                p->set_size_and_pinuse_of_free_chunk(dsize);
                return;
            }
            else
            {
                size_t nsize = next->chunksize();
                psize += nsize;
                unlink_chunk(next, nsize);
                p->set_size_and_pinuse_of_free_chunk(psize);
                if (p == _dv)
                {
                    _dvsize = psize;
                    return;
                }
            }
        }
        else
            p->set_free_with_pinuse(psize, next);
        insert_chunk(p, psize);
    }
    else
        SPP_ABORT;
}




void* malloc_state::tmalloc_large(size_t nb)
{
    tchunkptr v = 0;
    size_t rsize = -nb; 
    tchunkptr t;
    bindex_t idx = compute_tree_index(nb);
    if ((t = *treebin_at(idx)) != 0)
    {
        
        size_t sizebits = nb << leftshift_for_tree_index(idx);
        tchunkptr rst = 0;  
        for (;;)
        {
            tchunkptr rt;
            size_t trem = t->chunksize() - nb;
            if (trem < rsize)
            {
                v = t;
                if ((rsize = trem) == 0)
                    break;
            }
            rt = t->_child[1];
            t = t->_child[(sizebits >> (spp_size_t_bitsize - 1)) & 1];
            if (rt != 0 && rt != t)
                rst = rt;
            if (t == 0)
            {
                t = rst; 
                break;
            }
            sizebits <<= 1;
        }
    }
    if (t == 0 && v == 0)
    {
        
        binmap_t leftbits = left_bits(idx2bit(idx)) & _treemap;
        if (leftbits != 0)
        {
            binmap_t leastbit = least_bit(leftbits);
            bindex_t i = compute_bit2idx(leastbit);
            t = *treebin_at(i);
        }
    }

    while (t != 0)
    {
        
        size_t trem = t->chunksize() - nb;
        if (trem < rsize)
        {
            rsize = trem;
            v = t;
        }
        t = t->leftmost_child();
    }

    
    if (v != 0 && rsize < (size_t)(_dvsize - nb))
    {
        if (rtcheck(ok_address(v)))
        {
            
            mchunkptr r = (mchunkptr)v->chunk_plus_offset(nb);
            assert(v->chunksize() == rsize + nb);
            if (rtcheck(ok_next(v, r)))
            {
                unlink_large_chunk(v);
                if (rsize < MIN_CHUNK_SIZE)
                    set_inuse_and_pinuse(v, (rsize + nb));
                else
                {
                    set_size_and_pinuse_of_inuse_chunk(v, nb);
                    r->set_size_and_pinuse_of_free_chunk(rsize);
                    insert_chunk(r, rsize);
                }
                return chunk2mem(v);
            }
        }
        SPP_ABORT;
    }
    return 0;
}


void* malloc_state::tmalloc_small(size_t nb)
{
    tchunkptr t, v;
    size_t rsize;
    binmap_t leastbit = least_bit(_treemap);
    bindex_t i = compute_bit2idx(leastbit);
    v = t = *treebin_at(i);
    rsize = t->chunksize() - nb;

    while ((t = t->leftmost_child()) != 0)
    {
        size_t trem = t->chunksize() - nb;
        if (trem < rsize)
        {
            rsize = trem;
            v = t;
        }
    }

    if (rtcheck(ok_address(v)))
    {
        mchunkptr r = (mchunkptr)v->chunk_plus_offset(nb);
        assert(v->chunksize() == rsize + nb);
        if (rtcheck(ok_next(v, r)))
        {
            unlink_large_chunk(v);
            if (rsize < MIN_CHUNK_SIZE)
                set_inuse_and_pinuse(v, (rsize + nb));
            else
            {
                set_size_and_pinuse_of_inuse_chunk(v, nb);
                r->set_size_and_pinuse_of_free_chunk(rsize);
                replace_dv(r, rsize);
            }
            return chunk2mem(v);
        }
    }

    SPP_ABORT;
    return 0;
}



void* malloc_state::_malloc(size_t bytes)
{
    if (1)
    {
        void* mem;
        size_t nb;
        if (bytes <= MAX_SMALL_REQUEST)
        {
            bindex_t idx;
            binmap_t smallbits;
            nb = (bytes < MIN_REQUEST) ? MIN_CHUNK_SIZE : pad_request(bytes);
            idx = small_index(nb);
            smallbits = _smallmap >> idx;

            if ((smallbits & 0x3U) != 0)
            {
                
                mchunkptr b, p;
                idx += ~smallbits & 1;       
                b = smallbin_at(idx);
                p = b->_fd;
                assert(p->chunksize() == small_index2size(idx));
                unlink_first_small_chunk(b, p, idx);
                set_inuse_and_pinuse(p, small_index2size(idx));
                mem = chunk2mem(p);
                check_malloced_chunk(mem, nb);
                goto postaction;
            }

            else if (nb > _dvsize)
            {
                if (smallbits != 0)
                {
                    
                    mchunkptr b, p, r;
                    size_t rsize;
                    binmap_t leftbits = (smallbits << idx) & left_bits(malloc_state::idx2bit(idx));
                    binmap_t leastbit = least_bit(leftbits);
                    bindex_t i = compute_bit2idx(leastbit);
                    b = smallbin_at(i);
                    p = b->_fd;
                    assert(p->chunksize() == small_index2size(i));
                    unlink_first_small_chunk(b, p, i);
                    rsize = small_index2size(i) - nb;
                    
                    if (sizeof(size_t) != 4 && rsize < MIN_CHUNK_SIZE)
                        set_inuse_and_pinuse(p, small_index2size(i));
                    else
                    {
                        set_size_and_pinuse_of_inuse_chunk(p, nb);
                        r = (mchunkptr)p->chunk_plus_offset(nb);
                        r->set_size_and_pinuse_of_free_chunk(rsize);
                        replace_dv(r, rsize);
                    }
                    mem = chunk2mem(p);
                    check_malloced_chunk(mem, nb);
                    goto postaction;
                }

                else if (_treemap != 0 && (mem = tmalloc_small(nb)) != 0)
                {
                    check_malloced_chunk(mem, nb);
                    goto postaction;
                }
            }
        }
        else if (bytes >= MAX_REQUEST)
            nb = spp_max_size_t; 
        else
        {
            nb = pad_request(bytes);
            if (_treemap != 0 && (mem = tmalloc_large(nb)) != 0)
            {
                check_malloced_chunk(mem, nb);
                goto postaction;
            }
        }

        if (nb <= _dvsize)
        {
            size_t rsize = _dvsize - nb;
            mchunkptr p = _dv;
            if (rsize >= MIN_CHUNK_SIZE)
            {
                
                mchunkptr r = _dv = (mchunkptr)p->chunk_plus_offset(nb);
                _dvsize = rsize;
                r->set_size_and_pinuse_of_free_chunk(rsize);
                set_size_and_pinuse_of_inuse_chunk(p, nb);
            }
            else   
            {
                size_t dvs = _dvsize;
                _dvsize = 0;
                _dv = 0;
                set_inuse_and_pinuse(p, dvs);
            }
            mem = chunk2mem(p);
            check_malloced_chunk(mem, nb);
            goto postaction;
        }

        else if (nb < _topsize)
        {
            
            size_t rsize = _topsize -= nb;
            mchunkptr p = _top;
            mchunkptr r = _top = (mchunkptr)p->chunk_plus_offset(nb);
            r->_head = rsize | PINUSE_BIT;
            set_size_and_pinuse_of_inuse_chunk(p, nb);
            mem = chunk2mem(p);
            check_top_chunk(_top);
            check_malloced_chunk(mem, nb);
            goto postaction;
        }

        mem = sys_alloc(nb);

postaction:
        return mem;
    }

    return 0;
}



void malloc_state::_free(mchunkptr p)
{
    if (1)
    {
        check_inuse_chunk(p);
        if (rtcheck(ok_address(p) && ok_inuse(p)))
        {
            size_t psize = p->chunksize();
            mchunkptr next = (mchunkptr)p->chunk_plus_offset(psize);
            if (!p->pinuse())
            {
                size_t prevsize = p->_prev_foot;
                if (p->is_mmapped())
                {
                    psize += prevsize + SPP_MMAP_FOOT_PAD;
                    if (SPP_CALL_MUNMAP((char*)p - prevsize, psize) == 0)
                        _footprint -= psize;
                    goto postaction;
                }
                else
                {
                    mchunkptr prev = (mchunkptr)p->chunk_minus_offset(prevsize);
                    psize += prevsize;
                    p = prev;
                    if (rtcheck(ok_address(prev)))
                    {
                        
                        if (p != _dv)
                            unlink_chunk(p, prevsize);
                        else if ((next->_head & INUSE_BITS) == INUSE_BITS)
                        {
                            _dvsize = psize;
                            p->set_free_with_pinuse(psize, next);
                            goto postaction;
                        }
                    }
                    else
                        goto erroraction;
                }
            }

            if (rtcheck(ok_next(p, next) && ok_pinuse(next)))
            {
                if (!next->cinuse())
                {
                    
                    if (next == _top)
                    {
                        size_t tsize = _topsize += psize;
                        _top = p;
                        p->_head = tsize | PINUSE_BIT;
                        if (p == _dv)
                        {
                            _dv = 0;
                            _dvsize = 0;
                        }
                        if (should_trim(tsize))
                            sys_trim(0);
                        goto postaction;
                    }
                    else if (next == _dv)
                    {
                        size_t dsize = _dvsize += psize;
                        _dv = p;
                        p->set_size_and_pinuse_of_free_chunk(dsize);
                        goto postaction;
                    }
                    else
                    {
                        size_t nsize = next->chunksize();
                        psize += nsize;
                        unlink_chunk(next, nsize);
                        p->set_size_and_pinuse_of_free_chunk(psize);
                        if (p == _dv)
                        {
                            _dvsize = psize;
                            goto postaction;
                        }
                    }
                }
                else
                    p->set_free_with_pinuse(psize, next);

                if (is_small(psize))
                {
                    insert_small_chunk(p, psize);
                    check_free_chunk(p);
                }
                else
                {
                    tchunkptr tp = (tchunkptr)p;
                    insert_large_chunk(tp, psize);
                    check_free_chunk(p);
                    if (--_release_checks == 0)
                        release_unused_segments();
                }
                goto postaction;
            }
        }
erroraction:
        SPP_USAGE_ERROR_ACTION(this, p);
postaction:
        ;
    }
}




mchunkptr malloc_state::try_realloc_chunk(mchunkptr p, size_t nb, int can_move)
{
    mchunkptr newp = 0;
    size_t oldsize = p->chunksize();
    mchunkptr next = (mchunkptr)p->chunk_plus_offset(oldsize);
    if (rtcheck(ok_address(p) && ok_inuse(p) &&
                ok_next(p, next) && ok_pinuse(next)))
    {
        if (p->is_mmapped())
            newp = mmap_resize(p, nb, can_move);
        else if (oldsize >= nb)
        {
            
            size_t rsize = oldsize - nb;
            if (rsize >= MIN_CHUNK_SIZE)
            {
                
                mchunkptr r = (mchunkptr)p->chunk_plus_offset(nb);
                set_inuse(p, nb);
                set_inuse(r, rsize);
                dispose_chunk(r, rsize);
            }
            newp = p;
        }
        else if (next == _top)
        {
            
            if (oldsize + _topsize > nb)
            {
                size_t newsize = oldsize + _topsize;
                size_t newtopsize = newsize - nb;
                mchunkptr newtop = (mchunkptr)p->chunk_plus_offset(nb);
                set_inuse(p, nb);
                newtop->_head = newtopsize | PINUSE_BIT;
                _top = newtop;
                _topsize = newtopsize;
                newp = p;
            }
        }
        else if (next == _dv)
        {
            
            size_t dvs = _dvsize;
            if (oldsize + dvs >= nb)
            {
                size_t dsize = oldsize + dvs - nb;
                if (dsize >= MIN_CHUNK_SIZE)
                {
                    mchunkptr r = (mchunkptr)p->chunk_plus_offset(nb);
                    mchunkptr n = (mchunkptr)r->chunk_plus_offset(dsize);
                    set_inuse(p, nb);
                    r->set_size_and_pinuse_of_free_chunk(dsize);
                    n->clear_pinuse();
                    _dvsize = dsize;
                    _dv = r;
                }
                else
                {
                    
                    size_t newsize = oldsize + dvs;
                    set_inuse(p, newsize);
                    _dvsize = 0;
                    _dv = 0;
                }
                newp = p;
            }
        }
        else if (!next->cinuse())
        {
            
            size_t nextsize = next->chunksize();
            if (oldsize + nextsize >= nb)
            {
                size_t rsize = oldsize + nextsize - nb;
                unlink_chunk(next, nextsize);
                if (rsize < MIN_CHUNK_SIZE)
                {
                    size_t newsize = oldsize + nextsize;
                    set_inuse(p, newsize);
                }
                else
                {
                    mchunkptr r = (mchunkptr)p->chunk_plus_offset(nb);
                    set_inuse(p, nb);
                    set_inuse(r, rsize);
                    dispose_chunk(r, rsize);
                }
                newp = p;
            }
        }
    }
    else
        SPP_USAGE_ERROR_ACTION(m, chunk2mem(p));
    return newp;
}

void* malloc_state::internal_memalign(size_t alignment, size_t bytes)
{
    void* mem = 0;
    if (alignment < MIN_CHUNK_SIZE) 
        alignment = MIN_CHUNK_SIZE;
    if ((alignment & (alignment - 1)) != 0)
    {
        
        size_t a = SPP_MALLOC_ALIGNMENT << 1;
        while (a < alignment)
            a <<= 1;
        alignment = a;
    }
    if (bytes >= MAX_REQUEST - alignment)
        SPP_MALLOC_FAILURE_ACTION;
    else
    {
        size_t nb = request2size(bytes);
        size_t req = nb + alignment + MIN_CHUNK_SIZE - CHUNK_OVERHEAD;
        mem = internal_malloc(req);
        if (mem != 0)
        {
            mchunkptr p = mem2chunk(mem);
            if ((((size_t)(mem)) & (alignment - 1)) != 0)
            {
                
                
                char* br = (char*)mem2chunk((void *)(((size_t)((char*)mem + alignment - 1)) &
                                                     -alignment));
                char* pos = ((size_t)(br - (char*)(p)) >= MIN_CHUNK_SIZE) ?
                            br : br + alignment;
                mchunkptr newp = (mchunkptr)pos;
                size_t leadsize = pos - (char*)(p);
                size_t newsize = p->chunksize() - leadsize;

                if (p->is_mmapped())
                {
                    
                    newp->_prev_foot = p->_prev_foot + leadsize;
                    newp->_head = newsize;
                }
                else
                {
                    
                    set_inuse(newp, newsize);
                    set_inuse(p, leadsize);
                    dispose_chunk(p, leadsize);
                }
                p = newp;
            }

            
            if (!p->is_mmapped())
            {
                size_t size = p->chunksize();
                if (size > nb + MIN_CHUNK_SIZE)
                {
                    size_t remainder_size = size - nb;
                    mchunkptr remainder = (mchunkptr)p->chunk_plus_offset(nb);
                    set_inuse(p, nb);
                    set_inuse(remainder, remainder_size);
                    dispose_chunk(remainder, remainder_size);
                }
            }

            mem = chunk2mem(p);
            assert(p->chunksize() >= nb);
            assert(((size_t)mem & (alignment - 1)) == 0);
            check_inuse_chunk(p);
        }
    }
    return mem;
}


void** malloc_state::ialloc(size_t n_elements, size_t* sizes, int opts,
                            void* chunks[])
{

    size_t    element_size;   
    size_t    contents_size;  
    size_t    array_size;     
    void*     mem;            
    mchunkptr p;              
    size_t    remainder_size; 
    void**    marray;         
    mchunkptr array_chunk;    
    flag_t    was_enabled;    
    size_t    size;
    size_t    i;

    mparams.ensure_initialization();
    
    if (chunks != 0)
    {
        if (n_elements == 0)
            return chunks; 
        marray = chunks;
        array_size = 0;
    }
    else
    {
        
        if (n_elements == 0)
            return (void**)internal_malloc(0);
        marray = 0;
        array_size = request2size(n_elements * (sizeof(void*)));
    }

    
    if (opts & 0x1)
    {
        
        element_size = request2size(*sizes);
        contents_size = n_elements * element_size;
    }
    else
    {
        
        element_size = 0;
        contents_size = 0;
        for (i = 0; i != n_elements; ++i)
            contents_size += request2size(sizes[i]);
    }

    size = contents_size + array_size;

    
    was_enabled = use_mmap();
    disable_mmap();
    mem = internal_malloc(size - CHUNK_OVERHEAD);
    if (was_enabled)
        enable_mmap();
    if (mem == 0)
        return 0;

    p = mem2chunk(mem);
    remainder_size = p->chunksize();

    assert(!p->is_mmapped());

    if (opts & 0x2)
    {
        
        memset((size_t*)mem, 0, remainder_size - sizeof(size_t) - array_size);
    }

    
    if (marray == 0)
    {
        size_t  array_chunk_size;
        array_chunk = (mchunkptr)p->chunk_plus_offset(contents_size);
        array_chunk_size = remainder_size - contents_size;
        marray = (void**)(chunk2mem(array_chunk));
        set_size_and_pinuse_of_inuse_chunk(array_chunk, array_chunk_size);
        remainder_size = contents_size;
    }

    
    for (i = 0; ; ++i)
    {
        marray[i] = chunk2mem(p);
        if (i != n_elements - 1)
        {
            if (element_size != 0)
                size = element_size;
            else
                size = request2size(sizes[i]);
            remainder_size -= size;
            set_size_and_pinuse_of_inuse_chunk(p, size);
            p = (mchunkptr)p->chunk_plus_offset(size);
        }
        else
        {
            
            set_size_and_pinuse_of_inuse_chunk(p, remainder_size);
            break;
        }
    }

#if SPP_DEBUG
    if (marray != chunks)
    {
        
        if (element_size != 0)
            assert(remainder_size == element_size);
        else
            assert(remainder_size == request2size(sizes[i]));
        check_inuse_chunk(mem2chunk(marray));
    }
    for (i = 0; i != n_elements; ++i)
        check_inuse_chunk(mem2chunk(marray[i]));

#endif

    return marray;
}


size_t malloc_state::internal_bulk_free(void* array[], size_t nelem)
{
    size_t unfreed = 0;
    if (1)
    {
        void** a;
        void** fence = &(array[nelem]);
        for (a = array; a != fence; ++a)
        {
            void* mem = *a;
            if (mem != 0)
            {
                mchunkptr p = mem2chunk(mem);
                size_t psize = p->chunksize();
#if SPP_FOOTERS
                if (get_mstate_for(p) != m)
                {
                    ++unfreed;
                    continue;
                }
#endif
                check_inuse_chunk(p);
                *a = 0;
                if (rtcheck(ok_address(p) && ok_inuse(p)))
                {
                    void ** b = a + 1; 
                    mchunkptr next = (mchunkptr)p->next_chunk();
                    if (b != fence && *b == chunk2mem(next))
                    {
                        size_t newsize = next->chunksize() + psize;
                        set_inuse(p, newsize);
                        *b = chunk2mem(p);
                    }
                    else
                        dispose_chunk(p, psize);
                }
                else
                {
                    SPP_ABORT;
                    break;
                }
            }
        }
        if (should_trim(_topsize))
            sys_trim(0);
    }
    return unfreed;
}

void malloc_state::init(char* tbase, size_t tsize)
{
    _seg._base = _least_addr = tbase;
    _seg._size = _footprint = _max_footprint = tsize;
    _magic    = mparams._magic;
    _release_checks = SPP_MAX_RELEASE_CHECK_RATE;
    _mflags   = mparams._default_mflags;
    _extp     = 0;
    _exts     = 0;
    disable_contiguous();
    init_bins();
    mchunkptr mn = (mchunkptr)mem2chunk(this)->next_chunk();
    init_top(mn, (size_t)((tbase + tsize) - (char*)mn) - top_foot_size());
    check_top_chunk(_top);
}


#if SPP_MALLOC_INSPECT_ALL
void malloc_state::internal_inspect_all(void(*handler)(void *start, void *end,
                                        size_t used_bytes,
                                        void* callback_arg),
                                        void* arg)
{
    if (is_initialized())
    {
        mchunkptr top = top;
        msegmentptr s;
        for (s = &seg; s != 0; s = s->next)
        {
            mchunkptr q = align_as_chunk(s->base);
            while (segment_holds(s, q) && q->head != FENCEPOST_HEAD)
            {
                mchunkptr next = (mchunkptr)q->next_chunk();
                size_t sz = q->chunksize();
                size_t used;
                void* start;
                if (q->is_inuse())
                {
                    used = sz - CHUNK_OVERHEAD; 
                    start = chunk2mem(q);
                }
                else
                {
                    used = 0;
                    if (is_small(sz))
                    {
                        
                        start = (void*)((char*)q + sizeof(struct malloc_chunk));
                    }
                    else
                        start = (void*)((char*)q + sizeof(struct malloc_tree_chunk));
                }
                if (start < (void*)next)  
                    handler(start, next, used, arg);
                if (q == top)
                    break;
                q = next;
            }
        }
    }
}
#endif 





static mstate init_user_mstate(char* tbase, size_t tsize)
{
    size_t msize = pad_request(sizeof(malloc_state));
    mchunkptr msp = align_as_chunk(tbase);
    mstate m = (mstate)(chunk2mem(msp));
    memset(m, 0, msize);
    msp->_head = (msize | INUSE_BITS);
    m->init(tbase, tsize);
    return m;
}

SPP_API mspace create_mspace(size_t capacity, int locked)
{
    mstate m = 0;
    size_t msize;
    mparams.ensure_initialization();
    msize = pad_request(sizeof(malloc_state));
    if (capacity < (size_t) - (msize + top_foot_size() + mparams._page_size))
    {
        size_t rs = ((capacity == 0) ? mparams._granularity :
                     (capacity + top_foot_size() + msize));
        size_t tsize = mparams.granularity_align(rs);
        char* tbase = (char*)(SPP_CALL_MMAP(tsize));
        if (tbase != cmfail)
        {
            m = init_user_mstate(tbase, tsize);
            m->_seg._sflags = USE_MMAP_BIT;
            m->set_lock(locked);
        }
    }
    return (mspace)m;
}

SPP_API size_t destroy_mspace(mspace msp)
{
    size_t freed = 0;
    mstate ms = (mstate)msp;
    if (ms->ok_magic())
    {
        msegmentptr sp = &ms->_seg;
        while (sp != 0)
        {
            char* base = sp->_base;
            size_t size = sp->_size;
            flag_t flag = sp->_sflags;
            (void)base; 
            sp = sp->_next;
            if ((flag & USE_MMAP_BIT) && !(flag & EXTERN_BIT) &&
                SPP_CALL_MUNMAP(base, size) == 0)
                freed += size;
        }
    }
    else
        SPP_USAGE_ERROR_ACTION(ms, ms);
    return freed;
}


SPP_API void* mspace_malloc(mspace msp, size_t bytes)
{
    mstate ms = (mstate)msp;
    if (!ms->ok_magic())
    {
        SPP_USAGE_ERROR_ACTION(ms, ms);
        return 0;
    }
    return ms->_malloc(bytes);
}

SPP_API void mspace_free(mspace msp, void* mem)
{
    if (mem != 0)
    {
        mchunkptr p  = mem2chunk(mem);
#if SPP_FOOTERS
        mstate fm = get_mstate_for(p);
        (void)msp; 
#else
        mstate fm = (mstate)msp;
#endif
        if (!fm->ok_magic())
        {
            SPP_USAGE_ERROR_ACTION(fm, p);
            return;
        }
        fm->_free(p);
    }
}

SPP_API inline void* mspace_calloc(mspace msp, size_t n_elements, size_t elem_size)
{
    void* mem;
    size_t req = 0;
    mstate ms = (mstate)msp;
    if (!ms->ok_magic())
    {
        SPP_USAGE_ERROR_ACTION(ms, ms);
        return 0;
    }
    if (n_elements != 0)
    {
        req = n_elements * elem_size;
        if (((n_elements | elem_size) & ~(size_t)0xffff) &&
                (req / n_elements != elem_size))
            req = spp_max_size_t; 
    }
    mem = ms->internal_malloc(req);
    if (mem != 0 && mem2chunk(mem)->calloc_must_clear())
        memset(mem, 0, req);
    return mem;
}

SPP_API inline void* mspace_realloc(mspace msp, void* oldmem, size_t bytes)
{
    void* mem = 0;
    if (oldmem == 0)
        mem = mspace_malloc(msp, bytes);
    else if (bytes >= MAX_REQUEST)
        SPP_MALLOC_FAILURE_ACTION;
#ifdef REALLOC_ZERO_BYTES_FREES
    else if (bytes == 0)
        mspace_free(msp, oldmem);
#endif
    else
    {
        size_t nb = request2size(bytes);
        mchunkptr oldp = mem2chunk(oldmem);
#if ! SPP_FOOTERS
        mstate m = (mstate)msp;
#else
        mstate m = get_mstate_for(oldp);
        if (!m->ok_magic())
        {
            SPP_USAGE_ERROR_ACTION(m, oldmem);
            return 0;
        }
#endif
        if (1)
        {
            mchunkptr newp = m->try_realloc_chunk(oldp, nb, 1);
            if (newp != 0)
            {
                m->check_inuse_chunk(newp);
                mem = chunk2mem(newp);
            }
            else
            {
                mem = mspace_malloc(m, bytes);
                if (mem != 0)
                {
                    size_t oc = oldp->chunksize() - oldp->overhead_for();
                    memcpy(mem, oldmem, (oc < bytes) ? oc : bytes);
                    mspace_free(m, oldmem);
                }
            }
        }
    }
    return mem;
}

#if 0

SPP_API mspace create_mspace_with_base(void* base, size_t capacity, int locked)
{
    mstate m = 0;
    size_t msize;
    mparams.ensure_initialization();
    msize = pad_request(sizeof(malloc_state));
    if (capacity > msize + top_foot_size() &&
        capacity < (size_t) - (msize + top_foot_size() + mparams._page_size))
    {
        m = init_user_mstate((char*)base, capacity);
        m->_seg._sflags = EXTERN_BIT;
        m->set_lock(locked);
    }
    return (mspace)m;
}

SPP_API int mspace_track_large_chunks(mspace msp, int enable)
{
    int ret = 0;
    mstate ms = (mstate)msp;
    if (1)
    {
        if (!ms->use_mmap())
            ret = 1;
        if (!enable)
            ms->enable_mmap();
        else
            ms->disable_mmap();
    }
    return ret;
}

SPP_API void* mspace_realloc_in_place(mspace msp, void* oldmem, size_t bytes)
{
    void* mem = 0;
    if (oldmem != 0)
    {
        if (bytes >= MAX_REQUEST)
            SPP_MALLOC_FAILURE_ACTION;
        else
        {
            size_t nb = request2size(bytes);
            mchunkptr oldp = mem2chunk(oldmem);
#if ! SPP_FOOTERS
            mstate m = (mstate)msp;
#else
            mstate m = get_mstate_for(oldp);
            (void)msp; 
            if (!m->ok_magic())
            {
                SPP_USAGE_ERROR_ACTION(m, oldmem);
                return 0;
            }
#endif
            if (1)
            {
                mchunkptr newp = m->try_realloc_chunk(oldp, nb, 0);
                if (newp == oldp)
                {
                    m->check_inuse_chunk(newp);
                    mem = oldmem;
                }
            }
        }
    }
    return mem;
}

SPP_API void* mspace_memalign(mspace msp, size_t alignment, size_t bytes)
{
    mstate ms = (mstate)msp;
    if (!ms->ok_magic())
    {
        SPP_USAGE_ERROR_ACTION(ms, ms);
        return 0;
    }
    if (alignment <= SPP_MALLOC_ALIGNMENT)
        return mspace_malloc(msp, bytes);
    return ms->internal_memalign(alignment, bytes);
}

SPP_API void** mspace_independent_calloc(mspace msp, size_t n_elements,
                                        size_t elem_size, void* chunks[])
{
    size_t sz = elem_size; 
    mstate ms = (mstate)msp;
    if (!ms->ok_magic())
    {
        SPP_USAGE_ERROR_ACTION(ms, ms);
        return 0;
    }
    return ms->ialloc(n_elements, &sz, 3, chunks);
}

SPP_API void** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                          size_t sizes[], void* chunks[])
{
    mstate ms = (mstate)msp;
    if (!ms->ok_magic())
    {
        SPP_USAGE_ERROR_ACTION(ms, ms);
        return 0;
    }
    return ms->ialloc(n_elements, sizes, 0, chunks);
}

#endif

SPP_API inline size_t mspace_bulk_free(mspace msp, void* array[], size_t nelem)
{
    return ((mstate)msp)->internal_bulk_free(array, nelem);
}

#if SPP_MALLOC_INSPECT_ALL
SPP_API void mspace_inspect_all(mspace msp,
                                void(*handler)(void *start,
                                               void *end,
                                               size_t used_bytes,
                                               void* callback_arg),
                                void* arg)
{
    mstate ms = (mstate)msp;
    if (ms->ok_magic())
        internal_inspect_all(ms, handler, arg);
    else
        SPP_USAGE_ERROR_ACTION(ms, ms);
}
#endif

SPP_API inline int mspace_trim(mspace msp, size_t pad)
{
    int result = 0;
    mstate ms = (mstate)msp;
    if (ms->ok_magic())
        result = ms->sys_trim(pad);
    else
        SPP_USAGE_ERROR_ACTION(ms, ms);
    return result;
}

SPP_API inline size_t mspace_footprint(mspace msp)
{
    size_t result = 0;
    mstate ms = (mstate)msp;
    if (ms->ok_magic())
        result = ms->_footprint;
    else
        SPP_USAGE_ERROR_ACTION(ms, ms);
    return result;
}

SPP_API inline size_t mspace_max_footprint(mspace msp)
{
    size_t result = 0;
    mstate ms = (mstate)msp;
    if (ms->ok_magic())
        result = ms->_max_footprint;
    else
        SPP_USAGE_ERROR_ACTION(ms, ms);
    return result;
}

SPP_API inline size_t mspace_footprint_limit(mspace msp)
{
    size_t result = 0;
    mstate ms = (mstate)msp;
    if (ms->ok_magic())
    {
        size_t maf = ms->_footprint_limit;
        result = (maf == 0) ? spp_max_size_t : maf;
    }
    else
        SPP_USAGE_ERROR_ACTION(ms, ms);
    return result;
}

SPP_API inline size_t mspace_set_footprint_limit(mspace msp, size_t bytes)
{
    size_t result = 0;
    mstate ms = (mstate)msp;
    if (ms->ok_magic())
    {
        if (bytes == 0)
            result = mparams.granularity_align(1); 
        if (bytes == spp_max_size_t)
            result = 0;                    
        else
            result = mparams.granularity_align(bytes);
        ms->_footprint_limit = result;
    }
    else
        SPP_USAGE_ERROR_ACTION(ms, ms);
    return result;
}

SPP_API inline size_t mspace_usable_size(const void* mem)
{
    if (mem != 0)
    {
        mchunkptr p = mem2chunk(mem);
        if (p->is_inuse())
            return p->chunksize() - p->overhead_for();
    }
    return 0;
}

SPP_API inline int mspace_mallopt(int param_number, int value)
{
    return mparams.change(param_number, value);
}

} 


#endif 

#endif 
