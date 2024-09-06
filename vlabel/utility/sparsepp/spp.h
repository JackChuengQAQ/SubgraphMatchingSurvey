#if !defined(sparsepp_h_guard_)
#define sparsepp_h_guard_











































#include <cassert>
#include <cstring>
#include <string>
#include <limits>                           
#include <algorithm>                        
#include <iterator>                         
#include <functional>                       
#include <memory>                           
#include <cstdlib>                          
#include <cstddef>                          
#include <new>                              
#include <stdexcept>                        
#include <utility>                          
#include <cstdio>
#include <iosfwd>
#include <ios>

#include "spp_stdint.h"  
#include "spp_traits.h"
#include "spp_utils.h"

#ifdef SPP_INCLUDE_SPP_ALLOC
    #include "spp_dlalloc.h"
#endif

#if !defined(SPP_NO_CXX11_HDR_INITIALIZER_LIST)
    #include <initializer_list>
#endif

#if (SPP_GROUP_SIZE == 32)
    #define SPP_SHIFT_ 5
    #define SPP_MASK_  0x1F
    typedef uint32_t group_bm_type;
#elif (SPP_GROUP_SIZE == 64)
    #define SPP_SHIFT_ 6
    #define SPP_MASK_  0x3F
    typedef uint64_t group_bm_type;
#else
    #error "SPP_GROUP_SIZE must be either 32 or 64"
#endif

namespace spp_ {




template <class E>
inline void throw_exception(const E& exception)
{
#if !defined(SPP_NO_EXCEPTIONS)
    throw exception;
#else
    assert(0);
    abort();
#endif
}





template <class T>
struct cvt
{
    typedef T type;
};

template <class K, class V>
struct cvt<std::pair<const K, V> >
{
    typedef std::pair<K, V> type;
};

template <class K, class V>
struct cvt<const std::pair<const K, V> >
{
    typedef const std::pair<K, V> type;
};




#ifdef SPP_NO_CXX11_RVALUE_REFERENCES
    #define MK_MOVE_IT(p) (p)
#else
    #define MK_MOVE_IT(p) std::make_move_iterator(p)
#endif





#ifdef SPP_NO_CXX11_STATIC_ASSERT
    template <bool> struct SppCompileAssert { };
    #define SPP_COMPILE_ASSERT(expr, msg) \
      SPP_ATTRIBUTE_UNUSED typedef SppCompileAssert<(bool(expr))> spp_bogus_[bool(expr) ? 1 : -1]
#else
    #define SPP_COMPILE_ASSERT static_assert
#endif

namespace sparsehash_internal
{



























    

    template<typename Ignored>
    inline bool read_data_internal(Ignored* , FILE* fp,
                                   void* data, size_t length)
    {
        return fread(data, length, 1, fp) == 1;
    }

    template<typename Ignored>
    inline bool write_data_internal(Ignored* , FILE* fp,
                                    const void* data, size_t length)
    {
        return fwrite(data, length, 1, fp) == 1;
    }

    

    
    
    
    
    template<typename ISTREAM>
    inline bool read_data_internal_for_istream(ISTREAM* fp,
                                               void* data, size_t length)
    {
        return fp->read(reinterpret_cast<char*>(data),
                        static_cast<std::streamsize>(length)).good();
    }
    template<typename Ignored>
    inline bool read_data_internal(Ignored* , std::istream* fp,
                                   void* data, size_t length)
    {
        return read_data_internal_for_istream(fp, data, length);
    }

    template<typename OSTREAM>
    inline bool write_data_internal_for_ostream(OSTREAM* fp,
                                                const void* data, size_t length)
    {
        return fp->write(reinterpret_cast<const char*>(data),
                         static_cast<std::streamsize>(length)).good();
    }
    template<typename Ignored>
    inline bool write_data_internal(Ignored* , std::ostream* fp,
                                    const void* data, size_t length)
    {
        return write_data_internal_for_ostream(fp, data, length);
    }

    

    
    
    template <typename INPUT>
    inline bool read_data_internal(INPUT* fp, void* ,
                                   void* data, size_t length)
    {
        return static_cast<size_t>(fp->Read(data, length)) == length;
    }

    
    
    template <typename OUTPUT>
    inline bool write_data_internal(OUTPUT* fp, void* ,
                                    const void* data, size_t length)
    {
        return static_cast<size_t>(fp->Write(data, length)) == length;
    }

    

    template <typename INPUT>
    inline bool read_data(INPUT* fp, void* data, size_t length)
    {
        return read_data_internal(fp, fp, data, length);
    }

    template <typename OUTPUT>
    inline bool write_data(OUTPUT* fp, const void* data, size_t length)
    {
        return write_data_internal(fp, fp, data, length);
    }

    
    
    
    
    
    
    template <typename INPUT, typename IntType>
    bool read_bigendian_number(INPUT* fp, IntType* value, size_t length)
    {
        *value = 0;
        unsigned char byte;
        
        SPP_COMPILE_ASSERT(static_cast<IntType>(-1) > static_cast<IntType>(0), "serializing_int_requires_an_unsigned_type");
        for (size_t i = 0; i < length; ++i)
        {
            if (!read_data(fp, &byte, sizeof(byte)))
                return false;
            *value |= static_cast<IntType>(byte) << ((length - 1 - i) * 8);
        }
        return true;
    }

    template <typename OUTPUT, typename IntType>
    bool write_bigendian_number(OUTPUT* fp, IntType value, size_t length)
    {
        unsigned char byte;
        
        SPP_COMPILE_ASSERT(static_cast<IntType>(-1) > static_cast<IntType>(0), "serializing_int_requires_an_unsigned_type");
        for (size_t i = 0; i < length; ++i)
        {
            byte = (sizeof(value) <= length-1 - i)
                ? static_cast<unsigned char>(0) : static_cast<unsigned char>((value >> ((length-1 - i) * 8)) & 255);
            if (!write_data(fp, &byte, sizeof(byte))) return false;
        }
        return true;
    }

    
    
    
    
    
    
    template <typename value_type> struct pod_serializer
    {
        template <typename INPUT>
        bool operator()(INPUT* fp, value_type* value) const
        {
            return read_data(fp, value, sizeof(*value));
        }

        template <typename OUTPUT>
        bool operator()(OUTPUT* fp, const value_type& value) const
        {
            return write_data(fp, &value, sizeof(value));
        }
    };


    
    
    
    
    
    
    template<typename Key, typename HashFunc, typename SizeType, int HT_MIN_BUCKETS>
    class sh_hashtable_settings : public HashFunc
    {
    private:
#ifndef SPP_MIX_HASH
        template <class T, int sz> struct Mixer
        {
            inline T operator()(T h) const { return h; }
        };
#else
        template <class T, int sz> struct Mixer
        {
            inline T operator()(T h) const;
        };

         template <class T> struct Mixer<T, 4>
        {
            inline T operator()(T h) const
            {
                
                
                h = (h ^ 61) ^ (h >> 16);
                h = h + (h << 3);
                h = h ^ (h >> 4);
                h = h * 0x27d4eb2d;
                h = h ^ (h >> 15);
                return h;
            }
        };

        template <class T> struct Mixer<T, 8>
        {
            inline T operator()(T h) const
            {
                
                
                h = (~h) + (h << 21);              
                h = h ^ (h >> 24);
                h = (h + (h << 3)) + (h << 8);     
                h = h ^ (h >> 14);
                h = (h + (h << 2)) + (h << 4);     
                h = h ^ (h >> 28);
                h = h + (h << 31);
                return h;
            }
        };
#endif

    public:
        typedef Key key_type;
        typedef HashFunc hasher;
        typedef SizeType size_type;

    public:
        sh_hashtable_settings(const hasher& hf,
                              const float ht_occupancy_flt,
                              const float ht_empty_flt)
            : hasher(hf),
              enlarge_threshold_(0),
              shrink_threshold_(0),
              consider_shrink_(false),
              num_ht_copies_(0)
        {
            set_enlarge_factor(ht_occupancy_flt);
            set_shrink_factor(ht_empty_flt);
        }

        size_t hash(const key_type& v) const
        {
            size_t h = hasher::operator()(v);
            Mixer<size_t, sizeof(size_t)> mixer;

            return mixer(h);
        }

        float enlarge_factor() const            { return enlarge_factor_; }
        void set_enlarge_factor(float f)        { enlarge_factor_ = f;    }
        float shrink_factor() const             { return shrink_factor_;  }
        void set_shrink_factor(float f)         { shrink_factor_ = f;     }

        size_type enlarge_threshold() const     { return enlarge_threshold_; }
        void set_enlarge_threshold(size_type t) { enlarge_threshold_ = t; }
        size_type shrink_threshold() const      { return shrink_threshold_; }
        void set_shrink_threshold(size_type t)  { shrink_threshold_ = t; }

        size_type enlarge_size(size_type x) const { return static_cast<size_type>(x * enlarge_factor_); }
        size_type shrink_size(size_type x) const { return static_cast<size_type>(x * shrink_factor_); }

        bool consider_shrink() const            { return consider_shrink_; }
        void set_consider_shrink(bool t)        { consider_shrink_ = t; }

        unsigned int num_ht_copies() const      { return num_ht_copies_; }
        void inc_num_ht_copies()                { ++num_ht_copies_; }

        
        void reset_thresholds(size_type num_buckets)
        {
            set_enlarge_threshold(enlarge_size(num_buckets));
            set_shrink_threshold(shrink_size(num_buckets));
            
            set_consider_shrink(false);
        }

        
        
        
        void set_resizing_parameters(float shrink, float grow)
        {
            assert(shrink >= 0);
            assert(grow <= 1);
            if (shrink > grow/2.0f)
                shrink = grow / 2.0f;     
            set_shrink_factor(shrink);
            set_enlarge_factor(grow);
        }

        
        
        
        size_type min_buckets(size_type num_elts, size_type min_buckets_wanted)
        {
            float enlarge = enlarge_factor();
            size_type sz = HT_MIN_BUCKETS;             
            while (sz < min_buckets_wanted ||
                   num_elts >= static_cast<size_type>(sz * enlarge))
            {
                
                
                
                if (static_cast<size_type>(sz * 2) < sz)
                    throw_exception(std::length_error("resize overflow"));  
                sz *= 2;
            }
            return sz;
        }

    private:
        size_type enlarge_threshold_;  
        size_type shrink_threshold_;   
        float enlarge_factor_;         
        float shrink_factor_;          
        bool consider_shrink_;         

        unsigned int num_ht_copies_;   
    };

}  

#undef SPP_COMPILE_ASSERT




































































































template <class tabletype>
class table_iterator
{
public:
    typedef table_iterator iterator;

    typedef std::random_access_iterator_tag      iterator_category;
    typedef typename tabletype::value_type       value_type;
    typedef typename tabletype::difference_type  difference_type;
    typedef typename tabletype::size_type        size_type;

    explicit table_iterator(tabletype *tbl = 0, size_type p = 0) :
        table(tbl), pos(p)
    { }

    
    void check() const
    {
        assert(table);
        assert(pos <= table->size());
    }

    
    
    iterator& operator+=(size_type t) { pos += t; check(); return *this; }
    iterator& operator-=(size_type t) { pos -= t; check(); return *this; }
    iterator& operator++()            { ++pos; check(); return *this; }
    iterator& operator--()            { --pos; check(); return *this; }
    iterator operator++(int)
    {
        iterator tmp(*this);     
        ++pos; check(); return tmp;
    }

    iterator operator--(int)
    {
        iterator tmp(*this);     
        --pos; check(); return tmp;
    }

    iterator operator+(difference_type i) const
    {
        iterator tmp(*this);
        tmp += i; return tmp;
    }

    iterator operator-(difference_type i) const
    {
        iterator tmp(*this);
        tmp -= i; return tmp;
    }

    difference_type operator-(iterator it) const
    {
        
        assert(table == it.table);
        return pos - it.pos;
    }

    
    bool operator==(const iterator& it) const
    {
        return table == it.table && pos == it.pos;
    }

    bool operator<(const iterator& it) const
    {
        assert(table == it.table);              
        return pos < it.pos;
    }

    bool operator!=(const iterator& it) const { return !(*this == it); }
    bool operator<=(const iterator& it) const { return !(it < *this); }
    bool operator>(const iterator& it) const { return it < *this; }
    bool operator>=(const iterator& it) const { return !(*this < it); }

    
    tabletype *table;              
    size_type pos;                 
};



template <class tabletype>
class const_table_iterator
{
public:
    typedef table_iterator<tabletype> iterator;
    typedef const_table_iterator const_iterator;

    typedef std::random_access_iterator_tag iterator_category;
    typedef typename tabletype::value_type value_type;
    typedef typename tabletype::difference_type difference_type;
    typedef typename tabletype::size_type size_type;
    typedef typename tabletype::const_reference reference;  
    typedef typename tabletype::const_pointer pointer;

    
    const_table_iterator(const tabletype *tbl, size_type p)
        : table(tbl), pos(p) { }

    
    const_table_iterator() : table(NULL), pos(0) { }

    
    
    const_table_iterator(const iterator &from)
        : table(from.table), pos(from.pos) { }

    
    

    
    
    reference operator*() const       { return (*table)[pos]; }
    pointer operator->() const        { return &(operator*()); }

    
    void check() const
    {
        assert(table);
        assert(pos <= table->size());
    }

    
    
    const_iterator& operator+=(size_type t) { pos += t; check(); return *this; }
    const_iterator& operator-=(size_type t) { pos -= t; check(); return *this; }
    const_iterator& operator++()            { ++pos; check(); return *this; }
    const_iterator& operator--()            { --pos; check(); return *this; }
    const_iterator operator++(int)          
    {
        const_iterator tmp(*this); 
        ++pos; check(); 
        return tmp; 
    }
    const_iterator operator--(int)          
    {
        const_iterator tmp(*this); 
        --pos; check(); 
        return tmp;
    }
    const_iterator operator+(difference_type i) const
    {
        const_iterator tmp(*this);
        tmp += i;
        return tmp;
    }
    const_iterator operator-(difference_type i) const
    {
        const_iterator tmp(*this);
        tmp -= i;
        return tmp;
    }
    difference_type operator-(const_iterator it) const
    {
        
        assert(table == it.table);
        return pos - it.pos;
    }
    reference operator[](difference_type n) const
    {
        return *(*this + n);            
    }

    
    bool operator==(const const_iterator& it) const
    {
        return table == it.table && pos == it.pos;
    }

    bool operator<(const const_iterator& it) const
    {
        assert(table == it.table);              
        return pos < it.pos;
    }
    bool operator!=(const const_iterator& it) const { return !(*this == it); }
    bool operator<=(const const_iterator& it) const { return !(it < *this); }
    bool operator>(const const_iterator& it) const { return it < *this; }
    bool operator>=(const const_iterator& it) const { return !(*this < it); }

    
    const tabletype *table;        
    size_type pos;                 
};






















template <class T, class row_it, class col_it, class iter_type>
class Two_d_iterator
{
public:
    typedef Two_d_iterator iterator;
    typedef iter_type      iterator_category;
    typedef T              value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T*             pointer;
    typedef T&             reference;

    explicit Two_d_iterator(row_it curr) : row_current(curr), col_current(0)
    {
        if (row_current && !row_current->is_marked())
        {
            col_current = row_current->ne_begin();
            advance_past_end();                 
        }
    }

    explicit Two_d_iterator(row_it curr, col_it col) : row_current(curr), col_current(col)
    {
        assert(col);
    }

    
    Two_d_iterator() :  row_current(0), col_current(0) { }

    
    
    
    template <class T2, class row_it2, class col_it2, class iter_type2>
    Two_d_iterator(const Two_d_iterator<T2, row_it2, col_it2, iter_type2>& it) :
        row_current (*(row_it *)&it.row_current),
        col_current (*(col_it *)&it.col_current)
    { }

    
    

    value_type& operator*() const  { return *(col_current); }
    value_type* operator->() const { return &(operator*()); }

    
    
    
    
    void advance_past_end()
    {
        
        while (col_current == row_current->ne_end())
        {
            
            
            ++row_current;                                
            if (!row_current->is_marked())                
                col_current = row_current->ne_begin();
            else
                break;                                    
        }
    }

    friend size_t operator-(iterator l, iterator f)
    {
        if (f.row_current->is_marked())
            return 0;

        size_t diff(0);
        while (f != l)
        {
            ++diff;
            ++f;
        }
        return diff;
    }

    iterator& operator++()
    {
        
        ++col_current;
        advance_past_end();                              
        return *this;
    }

    iterator& operator--()
    {
        while (row_current->is_marked() ||
               col_current == row_current->ne_begin())
        {
            --row_current;
            col_current = row_current->ne_end();             
        }
        --col_current;
        return *this;
    }
    iterator operator++(int)       { iterator tmp(*this); ++*this; return tmp; }
    iterator operator--(int)       { iterator tmp(*this); --*this; return tmp; }


    
    bool operator==(const iterator& it) const
    {
        return (row_current == it.row_current &&
                (!row_current || row_current->is_marked() || col_current == it.col_current));
    }

    bool operator!=(const iterator& it) const { return !(*this == it); }

    
    
    
    row_it row_current;
    col_it col_current;
};




template <class T, class row_it, class col_it, class iter_type, class Alloc>
class Two_d_destructive_iterator : public Two_d_iterator<T, row_it, col_it, iter_type>
{
public:
    typedef Two_d_destructive_iterator iterator;

    Two_d_destructive_iterator(Alloc &alloc, row_it curr) :
        _alloc(alloc)
    {
        this->row_current = curr;
        this->col_current = 0;
        if (this->row_current && !this->row_current->is_marked())
        {
            this->col_current = this->row_current->ne_begin();
            advance_past_end();                 
        }
    }

    
    
    
    
    void advance_past_end()
    {
        
        while (this->col_current == this->row_current->ne_end())
        {
            this->row_current->clear(_alloc, true);  

            
            
            ++this->row_current;                          
            if (!this->row_current->is_marked())          
                this->col_current = this->row_current->ne_begin();
            else
                break;                                    
        }
    }

    iterator& operator++()
    {
        
        ++this->col_current;
        advance_past_end();                              
        return *this;
    }

private:
    Two_d_destructive_iterator& operator=(const Two_d_destructive_iterator &o);

    Alloc &_alloc;
};




#if defined(SPP_POPCNT_CHECK)
static inline bool spp_popcount_check()
{
    int cpuInfo[4] = { -1 };
    spp_cpuid(cpuInfo, 1);
    if (cpuInfo[2] & (1 << 23))
        return true;   
    return false;
}
#endif

#if defined(SPP_POPCNT_CHECK) && defined(SPP_POPCNT)

static inline uint32_t spp_popcount(uint32_t i)
{
    static const bool s_ok = spp_popcount_check();
    return s_ok ? SPP_POPCNT(i) : s_spp_popcount_default(i);
}

#else

static inline uint32_t spp_popcount(uint32_t i)
{
#if defined(SPP_POPCNT)
    return static_cast<uint32_t>(SPP_POPCNT(i));
#else
    return s_spp_popcount_default(i);
#endif
}

#endif

#if defined(SPP_POPCNT_CHECK) && defined(SPP_POPCNT64)

static inline uint32_t spp_popcount(uint64_t i)
{
    static const bool s_ok = spp_popcount_check();
    return s_ok ? (uint32_t)SPP_POPCNT64(i) : s_spp_popcount_default(i);
}

#else

static inline uint32_t spp_popcount(uint64_t i)
{
#if defined(SPP_POPCNT64)
    return static_cast<uint32_t>(SPP_POPCNT64(i));
#elif 1
    return s_spp_popcount_default(i);
#endif
}

#endif
























template <class T, class Alloc>
class sparsegroup
{
public:
    
    typedef T                                              value_type;
    typedef Alloc                                          allocator_type;
    typedef value_type&                                    reference;
    typedef const value_type&                              const_reference;
    typedef value_type*                                    pointer;
    typedef const value_type*                              const_pointer;

    typedef uint8_t                                        size_type;        

    
    
    
    typedef pointer                                        ne_iterator;
    typedef const_pointer                                  const_ne_iterator;
    typedef std::reverse_iterator<ne_iterator>             reverse_ne_iterator;
    typedef std::reverse_iterator<const_ne_iterator>       const_reverse_ne_iterator;

    
    
    ne_iterator               ne_begin()         { return reinterpret_cast<pointer>(_group); }
    const_ne_iterator         ne_begin() const   { return reinterpret_cast<pointer>(_group); }
    const_ne_iterator         ne_cbegin() const  { return reinterpret_cast<pointer>(_group); }
    ne_iterator               ne_end()           { return reinterpret_cast<pointer>(_group + _num_items()); }
    const_ne_iterator         ne_end() const     { return reinterpret_cast<pointer>(_group + _num_items()); }
    const_ne_iterator         ne_cend() const    { return reinterpret_cast<pointer>(_group + _num_items()); }
    reverse_ne_iterator       ne_rbegin()        { return reverse_ne_iterator(ne_end()); }
    const_reverse_ne_iterator ne_rbegin() const  { return const_reverse_ne_iterator(ne_cend());  }
    const_reverse_ne_iterator ne_crbegin() const { return const_reverse_ne_iterator(ne_cend());  }
    reverse_ne_iterator       ne_rend()          { return reverse_ne_iterator(ne_begin()); }
    const_reverse_ne_iterator ne_rend() const    { return const_reverse_ne_iterator(ne_cbegin());  }
    const_reverse_ne_iterator ne_crend() const   { return const_reverse_ne_iterator(ne_cbegin());  }

private:
    
    
    typedef typename spp_::cvt<T>::type                    mutable_value_type;
    typedef mutable_value_type &                           mutable_reference;
    typedef mutable_value_type *                           mutable_pointer;
    typedef const mutable_value_type *                     const_mutable_pointer;

    bool _bmtest(size_type i) const   { return !!(_bitmap & (static_cast<group_bm_type>(1) << i)); }
    void _bmset(size_type i)          { _bitmap |= static_cast<group_bm_type>(1) << i; }
    void _bmclear(size_type i)        { _bitmap &= ~(static_cast<group_bm_type>(1) << i); }

    bool _bme_test(size_type i) const { return !!(_bm_erased & (static_cast<group_bm_type>(1) << i)); }
    void _bme_set(size_type i)        { _bm_erased |= static_cast<group_bm_type>(1) << i; }
    void _bme_clear(size_type i)      { _bm_erased &= ~(static_cast<group_bm_type>(1) << i); }

    bool _bmtest_strict(size_type i) const
    { return !!((_bitmap | _bm_erased) & (static_cast<group_bm_type>(1) << i)); }


    static uint32_t _sizing(uint32_t n)
    {
#if !defined(SPP_ALLOC_SZ) || (SPP_ALLOC_SZ == 0)
        
        
        struct alloc_batch_size
        {
            
            
            
            
            SPP_CXX14_CONSTEXPR alloc_batch_size()
                : data()
            {
                uint8_t group_sz          = SPP_GROUP_SIZE / 4;
                uint8_t group_start_alloc = SPP_GROUP_SIZE / 8; 
                uint8_t alloc_sz          = group_start_alloc;
                for (int i=0; i<4; ++i)
                {
                    for (int j=0; j<group_sz; ++j)
                    {
                        if (j && j % group_start_alloc == 0)
                            alloc_sz += group_start_alloc;
                        data[i * group_sz + j] = alloc_sz;
                    }
                    if (group_start_alloc > 2)
                        group_start_alloc /= 2;
                    alloc_sz += group_start_alloc;
                }
            }
            uint8_t data[SPP_GROUP_SIZE];
        };

        static alloc_batch_size s_alloc_batch_sz;
        return n ? static_cast<uint32_t>(s_alloc_batch_sz.data[n-1]) : 0; 

#elif (SPP_ALLOC_SZ == 1)
        
        
        return n;
#else
        
        
        static size_type sz_minus_1 = SPP_ALLOC_SZ - 1;
        return (n + sz_minus_1) & ~sz_minus_1;
#endif
    }

    pointer _allocate_group(allocator_type &alloc, uint32_t n )
    {
        
        

        uint32_t num_alloc = (uint8_t)_sizing(n);
        _set_num_alloc(num_alloc);
        pointer retval = alloc.allocate(static_cast<size_type>(num_alloc));
        if (retval == NULL)
        {
            
            throw_exception(std::bad_alloc());
        }
        return retval;
    }

    void _free_group(allocator_type &alloc, uint32_t num_alloc)
    {
        if (_group)
        {
            uint32_t num_buckets = _num_items();
            if (num_buckets)
            {
                mutable_pointer end_it = (mutable_pointer)(_group + num_buckets);
                for (mutable_pointer p = (mutable_pointer)_group; p != end_it; ++p)
                    p->~mutable_value_type();
            }
            alloc.deallocate(_group, (typename allocator_type::size_type)num_alloc);
            _group = NULL;
        }
    }

    
    sparsegroup &operator=(const sparsegroup& x);

    static size_type _pos_to_offset(group_bm_type bm, size_type pos)
    {
        
        
        return static_cast<size_type>(spp_popcount(bm & ((static_cast<group_bm_type>(1) << pos) - 1)));
    }

public:

    
    size_type pos_to_offset(size_type pos) const
    {
        return _pos_to_offset(_bitmap, pos);
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4146)
#endif

    
    
    
    
    
    
    static size_type offset_to_pos(group_bm_type bm, size_type offset)
    {
        for (; offset > 0; offset--)
            bm &= (bm-1);  

        
        
        
        
        bm = (bm & -bm) - 1;
        return  static_cast<size_type>(spp_popcount(bm));
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    size_type offset_to_pos(size_type offset) const
    {
        return offset_to_pos(_bitmap, offset);
    }

public:
    
    explicit sparsegroup() :
        _group(0), _bitmap(0), _bm_erased(0)
    {
        _set_num_items(0);
        _set_num_alloc(0);
    }

    sparsegroup(const sparsegroup& x) :
        _group(0), _bitmap(x._bitmap), _bm_erased(x._bm_erased)
    {
        _set_num_items(0);
        _set_num_alloc(0);
         assert(_group == 0); 
    }

    sparsegroup(const sparsegroup& x, allocator_type& a) :
        _group(0), _bitmap(x._bitmap), _bm_erased(x._bm_erased)
    {
        _set_num_items(0);
        _set_num_alloc(0);

        uint32_t num_items = x._num_items();
        if (num_items)
        {
            _group = _allocate_group(a, num_items );
            _set_num_items(num_items);
            std::uninitialized_copy(x._group, x._group + num_items, _group);
        }
    }

    ~sparsegroup() { assert(_group == 0); }

    void destruct(allocator_type& a) { _free_group(a, _num_alloc()); }

    
    void swap(sparsegroup& x)
    {
        using std::swap;

        swap(_group, x._group);
        swap(_bitmap, x._bitmap);
        swap(_bm_erased, x._bm_erased);
#ifdef SPP_STORE_NUM_ITEMS
        swap(_num_buckets,   x._num_buckets);
        swap(_num_allocated, x._num_allocated);
#endif
    }

    
    void clear(allocator_type &alloc, bool erased)
    {
        _free_group(alloc, _num_alloc());
        _bitmap = 0;
        if (erased)
            _bm_erased = 0;
        _set_num_items(0);
        _set_num_alloc(0);
    }

    
    
    size_type size() const           { return static_cast<size_type>(SPP_GROUP_SIZE); }
    size_type max_size() const       { return static_cast<size_type>(SPP_GROUP_SIZE); }

    bool empty() const               { return false; }

    
    size_type num_nonempty() const   { return (size_type)_num_items(); }

    
    
    
    reference unsafe_get(size_type i) const
    {
        
        return (reference)_group[pos_to_offset(i)];
    }

    typedef std::pair<pointer, bool> SetResult;

private:
    
    typedef spp_::true_type  realloc_ok_type;
    typedef spp_::false_type realloc_not_ok_type;

    
    
    
    

#if 1
    typedef typename if_<((spp_::is_same<allocator_type, libc_allocator<value_type> >::value ||
                           spp_::is_same<allocator_type,  spp_allocator<value_type> >::value) &&
                          spp_::is_relocatable<value_type>::value), realloc_ok_type, realloc_not_ok_type>::type
             check_alloc_type;
#else
    typedef typename if_<spp_::is_same<allocator_type, spp_allocator<value_type> >::value,
                         typename if_<spp_::is_relocatable<value_type>::value, spp_reloc_type, spp_not_reloc_type>::type,
                         typename if_<(spp_::is_same<allocator_type, libc_allocator<value_type> >::value &&
                                       spp_::is_relocatable<value_type>::value), libc_reloc_type, generic_alloc_type>::type >::type 
        check_alloc_type;
#endif


    
    
    
    

    
    
    
    
    

    
    void _init_val(mutable_value_type *p, reference val)
    {
#if !defined(SPP_NO_CXX11_RVALUE_REFERENCES)
        ::new (p) value_type(std::move((mutable_reference)val));
#else
        ::new (p) value_type((mutable_reference)val);
#endif
    }

    
    void _init_val(mutable_value_type *p, const_reference val)
    {
        ::new (p) value_type(val);
    }

    
    void _set_val(value_type *p, reference val)
    {
#if !defined(SPP_NO_CXX11_RVALUE_REFERENCES)
        *(mutable_pointer)p = std::move((mutable_reference)val);
#else
        using std::swap;
        swap(*(mutable_pointer)p, *(mutable_pointer)&val);
#endif
    }

    
    void _set_val(value_type *p, const_reference val)
    {
        *(mutable_pointer)p = *(const_mutable_pointer)&val;
    }

    
    
    
    
    template <class Val>
    void _set_aux(allocator_type &alloc, size_type offset, Val &val, realloc_ok_type)
    {
        

        uint32_t  num_items = _num_items();
        uint32_t  num_alloc = _sizing(num_items);

        if (num_items == num_alloc)
        {
            num_alloc = _sizing(num_items + 1);
            _group = alloc.reallocate(_group, num_alloc);
            _set_num_alloc(num_alloc);
        }

        for (uint32_t i = num_items; i > offset; --i)
            memcpy(static_cast<void *>(_group + i), _group + i-1, sizeof(*_group));

        _init_val((mutable_pointer)(_group + offset), val);
    }

    
    
    
    
    template <class Val>
    void _set_aux(allocator_type &alloc, size_type offset, Val &val, realloc_not_ok_type)
    {
        uint32_t  num_items = _num_items();
        uint32_t  num_alloc = _sizing(num_items);

        
        if (num_items < num_alloc)
        {
            
            _init_val((mutable_pointer)&_group[num_items], val);
            std::rotate((mutable_pointer)(_group + offset),
                        (mutable_pointer)(_group + num_items),
                        (mutable_pointer)(_group + num_items + 1));
            return;
        }

        
        pointer p = _allocate_group(alloc, _sizing(num_items + 1));
        if (offset)
            std::uninitialized_copy(MK_MOVE_IT((mutable_pointer)_group),
                                    MK_MOVE_IT((mutable_pointer)(_group + offset)),
                                    (mutable_pointer)p);
        if (num_items > offset)
            std::uninitialized_copy(MK_MOVE_IT((mutable_pointer)(_group + offset)),
                                    MK_MOVE_IT((mutable_pointer)(_group + num_items)),
                                    (mutable_pointer)(p + offset + 1));
        _init_val((mutable_pointer)(p + offset), val);
        _free_group(alloc, num_alloc);
        _group = p;
    }

    
    template <class Val>
    void _set(allocator_type &alloc, size_type i, size_type offset, Val &val)
    {
        if (!_bmtest(i))
        {
            _set_aux(alloc, offset, val, check_alloc_type());
            _incr_num_items();
            _bmset(i);
        }
        else
            _set_val(&_group[offset], val);
    }

public:

    
    
    template <class Val>
    pointer set(allocator_type &alloc, size_type i, Val &val)
    {
        _bme_clear(i); 

        size_type offset = pos_to_offset(i);
        _set(alloc, i, offset, val);            
        return (pointer)(_group + offset);
    }

    
    
    bool test(size_type i) const
    {
        return _bmtest(i);
    }

    
    
    bool test_strict(size_type i) const
    {
        return _bmtest_strict(i);
    }

private:
    
    
    
    void _group_erase_aux(allocator_type &alloc, size_type offset, realloc_ok_type)
    {
        
        uint32_t  num_items = _num_items();
        uint32_t  num_alloc = _sizing(num_items);

        if (num_items == 1)
        {
            assert(offset == 0);
            _free_group(alloc, num_alloc);
            _set_num_alloc(0);
            return;
        }

        _group[offset].~value_type();

        for (size_type i = offset; i < num_items - 1; ++i)
            memcpy(static_cast<void *>(_group + i), _group + i + 1, sizeof(*_group));

        if (_sizing(num_items - 1) != num_alloc)
        {
            num_alloc = _sizing(num_items - 1);
            assert(num_alloc);            
            _set_num_alloc(num_alloc);
            _group = alloc.reallocate(_group, num_alloc);
        }
    }

    
    
    
    void _group_erase_aux(allocator_type &alloc, size_type offset, realloc_not_ok_type)
    {
        uint32_t  num_items = _num_items();
        uint32_t  num_alloc   = _sizing(num_items);

        if (_sizing(num_items - 1) != num_alloc)
        {
            pointer p = 0;
            if (num_items > 1)
            {
                p = _allocate_group(alloc, num_items - 1);
                if (offset)
                    std::uninitialized_copy(MK_MOVE_IT((mutable_pointer)(_group)),
                                            MK_MOVE_IT((mutable_pointer)(_group + offset)),
                                            (mutable_pointer)(p));
                if (static_cast<uint32_t>(offset + 1) < num_items)
                    std::uninitialized_copy(MK_MOVE_IT((mutable_pointer)(_group + offset + 1)),
                                            MK_MOVE_IT((mutable_pointer)(_group + num_items)),
                                            (mutable_pointer)(p + offset));
            }
            else
            {
                assert(offset == 0);
                _set_num_alloc(0);
            }
            _free_group(alloc, num_alloc);
            _group = p;
        }
        else
        {
            std::rotate((mutable_pointer)(_group + offset),
                        (mutable_pointer)(_group + offset + 1),
                        (mutable_pointer)(_group + num_items));
            ((mutable_pointer)(_group + num_items - 1))->~mutable_value_type();
        }
    }

    void _group_erase(allocator_type &alloc, size_type offset)
    {
        _group_erase_aux(alloc, offset, check_alloc_type());
    }

public:
    template <class twod_iter>
    bool erase_ne(allocator_type &alloc, twod_iter &it)
    {
        assert(_group && it.col_current != ne_end());
        size_type offset = (size_type)(it.col_current - ne_begin());
        size_type pos    = offset_to_pos(offset);

        if (_num_items() <= 1)
        {
            clear(alloc, false);
            it.col_current = 0;
        }
        else
        {
            _group_erase(alloc, offset);
            _decr_num_items();
            _bmclear(pos);

            
            it.col_current = reinterpret_cast<pointer>(_group) + offset;
        }
        _bme_set(pos);  
        it.advance_past_end();
        return true;
    }


    
    
    
    
    
    void erase(allocator_type &alloc, size_type i)
    {
        if (_bmtest(i))
        {
            
            if (_num_items() == 1)
                clear(alloc, false);
            else
            {
                _group_erase(alloc, pos_to_offset(i));
                _decr_num_items();
                _bmclear(i);
            }
            _bme_set(i); 
        }
    }

    
    
    
    
    
    template <typename OUTPUT> bool write_metadata(OUTPUT *fp) const
    {
        
        
        
        if (!sparsehash_internal::write_data(fp, &_bitmap, sizeof(_bitmap)))
            return false;

        return true;
    }

    
    template <typename INPUT> bool read_metadata(allocator_type &alloc, INPUT *fp)
    {
        clear(alloc, true);

        if (!sparsehash_internal::read_data(fp, &_bitmap, sizeof(_bitmap)))
            return false;

        
        
        uint32_t num_items = spp_popcount(_bitmap); 
        _set_num_items(num_items);
        _group = num_items ? _allocate_group(alloc, num_items) : 0;
        return true;
    }

    
    template <typename INPUT> bool read_nopointer_data(INPUT *fp)
    {
        for (ne_iterator it = ne_begin(); it != ne_end(); ++it)
            if (!sparsehash_internal::read_data(fp, &(*it), sizeof(*it)))
                return false;
        return true;
    }

    
    
    
    
    template <typename OUTPUT> bool write_nopointer_data(OUTPUT *fp) const
    {
        for (const_ne_iterator it = ne_begin(); it != ne_end(); ++it)
            if (!sparsehash_internal::write_data(fp, &(*it), sizeof(*it)))
                return false;
        return true;
    }


    
    
    
    
    
    
    bool operator==(const sparsegroup& x) const
    {
        return (_bitmap == x._bitmap &&
                _bm_erased == x._bm_erased &&
                std::equal(_group, _group + _num_items(), x._group));
    }

    bool operator<(const sparsegroup& x) const
    {
        
        return std::lexicographical_compare(_group, _group + _num_items(),
                                            x._group, x._group + x._num_items());
    }

    bool operator!=(const sparsegroup& x) const { return !(*this == x); }
    bool operator<=(const sparsegroup& x) const { return !(x < *this); }
    bool operator> (const sparsegroup& x) const { return x < *this; }
    bool operator>=(const sparsegroup& x) const { return !(*this < x); }

    void mark()            { _group = (value_type *)static_cast<uintptr_t>(-1); }
    bool is_marked() const { return _group == (value_type *)static_cast<uintptr_t>(-1); }

private:
    
    template <class A>
    class alloc_impl : public A
    {
    public:
        typedef typename A::pointer pointer;
        typedef typename A::size_type size_type;

        
        explicit alloc_impl(const A& a) : A(a) { }

        
        
        pointer realloc_or_die(pointer , size_type )
        {
            throw_exception(std::runtime_error("realloc_or_die is only supported for spp::spp_allocator\n"));
            return NULL;
        }
    };

    
    
    
    template <class A>
    class alloc_impl<spp_::libc_allocator<A> > : public spp_::libc_allocator<A>
    {
    public:
        typedef typename spp_::libc_allocator<A>::pointer pointer;
        typedef typename spp_::libc_allocator<A>::size_type size_type;

        explicit alloc_impl(const spp_::libc_allocator<A>& a)
            : spp_::libc_allocator<A>(a)
        { }

        pointer realloc_or_die(pointer ptr, size_type n)
        {
            pointer retval = this->reallocate(ptr, n);
            if (retval == NULL) 
            {
                
                throw_exception(std::bad_alloc());
            }
            return retval;
        }
    };

    
    
    
    template <class A>
    class alloc_impl<spp_::spp_allocator<A> > : public spp_::spp_allocator<A>
    {
    public:
        typedef typename spp_::spp_allocator<A>::pointer pointer;
        typedef typename spp_::spp_allocator<A>::size_type size_type;

        explicit alloc_impl(const spp_::spp_allocator<A>& a)
            : spp_::spp_allocator<A>(a)
        { }

        pointer realloc_or_die(pointer ptr, size_type n)
        {
            pointer retval = this->reallocate(ptr, n);
            if (retval == NULL) 
            {
                
                throw_exception(std::bad_alloc());
            }
            return retval;
        }
    };


#ifdef SPP_STORE_NUM_ITEMS
    uint32_t _num_items() const           { return (uint32_t)_num_buckets; }
    void     _set_num_items(uint32_t val) { _num_buckets = static_cast<size_type>(val); }
    void     _incr_num_items()            { ++_num_buckets; }
    void     _decr_num_items()            { --_num_buckets; }
    uint32_t _num_alloc() const           { return (uint32_t)_num_allocated; }
    void     _set_num_alloc(uint32_t val) { _num_allocated = static_cast<size_type>(val); }
#else
    uint32_t _num_items() const           { return spp_popcount(_bitmap); }
    void     _set_num_items(uint32_t )    { }
    void     _incr_num_items()            { }
    void     _decr_num_items()            { }
    uint32_t _num_alloc() const           { return _sizing(_num_items()); }
    void     _set_num_alloc(uint32_t val) { }
#endif

    
    
    value_type *         _group;                             
    group_bm_type        _bitmap;
    group_bm_type        _bm_erased;                         

#ifdef SPP_STORE_NUM_ITEMS
    size_type            _num_buckets;
    size_type            _num_allocated;
#endif
};



template <class T, class Alloc>
class sparsetable
{
public:
    typedef T                                             value_type;
    typedef Alloc                                         allocator_type;
    typedef sparsegroup<value_type, allocator_type>       group_type;

private:
    typedef typename Alloc::template rebind<group_type>::other group_alloc_type;
    typedef typename group_alloc_type::size_type          group_size_type;

public:
    
    
    typedef typename allocator_type::size_type            size_type;
    typedef typename allocator_type::difference_type      difference_type;
    typedef value_type&                                   reference;
    typedef const value_type&                             const_reference;
    typedef value_type*                                   pointer;
    typedef const value_type*                             const_pointer;

    typedef group_type&                                   GroupsReference;
    typedef const group_type&                             GroupsConstReference;

    typedef typename group_type::ne_iterator              ColIterator;
    typedef typename group_type::const_ne_iterator        ColConstIterator;

    typedef table_iterator<sparsetable<T, allocator_type> >        iterator;       
    typedef const_table_iterator<sparsetable<T, allocator_type> >  const_iterator; 
    typedef std::reverse_iterator<const_iterator>         const_reverse_iterator;
    typedef std::reverse_iterator<iterator>               reverse_iterator;

    
    
    
    typedef Two_d_iterator<T,
                           group_type *,
                           ColIterator,
                           std::bidirectional_iterator_tag> ne_iterator;

    typedef Two_d_iterator<const T,
                           const group_type *,
                           ColConstIterator,
                           std::bidirectional_iterator_tag> const_ne_iterator;

    
    
    
    typedef Two_d_destructive_iterator<T,
                                       group_type *,
                                       ColIterator,
                                       std::input_iterator_tag,
                                       allocator_type>     destructive_iterator;

    typedef std::reverse_iterator<ne_iterator>               reverse_ne_iterator;
    typedef std::reverse_iterator<const_ne_iterator>         const_reverse_ne_iterator;


    
    
    iterator               begin()         { return iterator(this, 0); }
    const_iterator         begin() const   { return const_iterator(this, 0); }
    const_iterator         cbegin() const  { return const_iterator(this, 0); }
    iterator               end()           { return iterator(this, size()); }
    const_iterator         end() const     { return const_iterator(this, size()); }
    const_iterator         cend() const    { return const_iterator(this, size()); }
    reverse_iterator       rbegin()        { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const  { return const_reverse_iterator(cend()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
    reverse_iterator       rend()          { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const    { return const_reverse_iterator(cbegin()); }
    const_reverse_iterator crend() const   { return const_reverse_iterator(cbegin()); }

    
    
    ne_iterator       ne_begin()           { return ne_iterator      (_first_group); }
    const_ne_iterator ne_begin() const     { return const_ne_iterator(_first_group); }
    const_ne_iterator ne_cbegin() const    { return const_ne_iterator(_first_group); }
    ne_iterator       ne_end()             { return ne_iterator      (_last_group); }
    const_ne_iterator ne_end() const       { return const_ne_iterator(_last_group); }
    const_ne_iterator ne_cend() const      { return const_ne_iterator(_last_group); }

    reverse_ne_iterator       ne_rbegin()        { return reverse_ne_iterator(ne_end()); }
    const_reverse_ne_iterator ne_rbegin() const  { return const_reverse_ne_iterator(ne_end());  }
    const_reverse_ne_iterator ne_crbegin() const { return const_reverse_ne_iterator(ne_end());  }
    reverse_ne_iterator       ne_rend()          { return reverse_ne_iterator(ne_begin()); }
    const_reverse_ne_iterator ne_rend() const    { return const_reverse_ne_iterator(ne_begin()); }
    const_reverse_ne_iterator ne_crend() const   { return const_reverse_ne_iterator(ne_begin()); }

    destructive_iterator destructive_begin()
    {
        return destructive_iterator(_alloc, _first_group);
    }

    destructive_iterator destructive_end()
    {
        return destructive_iterator(_alloc, _last_group);
    }

    
    static group_size_type num_groups(size_type num)
    {
        
        return num == 0 ? (group_size_type)0 :
            (group_size_type)(((num-1) / SPP_GROUP_SIZE) + 1);
    }

    typename group_type::size_type pos_in_group(size_type i) const
    {
        return static_cast<typename group_type::size_type>(i & SPP_MASK_);
    }

    size_type group_num(size_type i) const
    {
        return (size_type)(i >> SPP_SHIFT_);
    }

    GroupsReference which_group(size_type i)
    {
        return _first_group[group_num(i)];
    }

    GroupsConstReference which_group(size_type i) const
    {
        return _first_group[group_num(i)];
    }

    void _alloc_group_array(group_size_type sz, group_type *&first, group_type *&last)
    {
        if (sz)
        {
            first = _group_alloc.allocate((size_type)(sz + 1)); 
            first[sz].mark();                      
            last = first + sz;
        }
    }

    void _free_group_array(group_type *&first, group_type *&last)
    {
        if (first)
        {
            _group_alloc.deallocate(first, (group_size_type)(last - first + 1)); 
            first = last = 0;
        }
    }

    void _allocate_groups(size_type sz)
    {
        if (sz)
        {
            _alloc_group_array(sz, _first_group, _last_group);
            std::uninitialized_fill(_first_group, _last_group, group_type());
        }
    }

    void _free_groups()
    {
        if (_first_group)
        {
            for (group_type *g = _first_group; g != _last_group; ++g)
                g->destruct(_alloc);
            _free_group_array(_first_group, _last_group);
        }
    }

    void _cleanup()
    {
        _free_groups();    
        _table_size  = 0;
        _num_buckets = 0;
    }

    void _init()
    {
        _first_group = 0;
        _last_group  = 0;
        _table_size  = 0;
        _num_buckets = 0;
    }

    void _copy(const sparsetable &o)
    {
        _table_size = o._table_size;
        _num_buckets = o._num_buckets;
        _alloc = o._alloc;                
        _group_alloc = o._group_alloc;    

        group_size_type sz = (group_size_type)(o._last_group - o._first_group);
        if (sz)
        {
            _alloc_group_array(sz, _first_group, _last_group);
            for (group_size_type i=0; i<sz; ++i)
                new (_first_group + i) group_type(o._first_group[i], _alloc);
        }
    }

public:
    
    explicit sparsetable(size_type sz = 0, const allocator_type &alloc = allocator_type()) :
        _first_group(0),
        _last_group(0),
        _table_size(sz),
        _num_buckets(0),
        _group_alloc(alloc),
        _alloc(alloc)
                       
                       
    {
        _allocate_groups(num_groups(sz));
    }

    ~sparsetable()
    {
        _free_groups();
    }

    sparsetable(const sparsetable &o)
    {
        _init();
        _copy(o);
    }

    sparsetable& operator=(const sparsetable &o)
    {
        _cleanup();
        _copy(o);
        return *this;
    }


#if !defined(SPP_NO_CXX11_RVALUE_REFERENCES)
    sparsetable(sparsetable&& o)
    {
        _init();
        this->swap(o);
    }

    sparsetable(sparsetable&& o, const allocator_type &alloc)
    {
        _init();
        this->swap(o);
        _alloc = alloc; 
    }

    sparsetable& operator=(sparsetable&& o)
    {
        _cleanup();
        this->swap(o);
        return *this;
    }
#endif

    
    void swap(sparsetable& o)
    {
        using std::swap;

        swap(_first_group, o._first_group);
        swap(_last_group,  o._last_group);
        swap(_table_size,  o._table_size);
        swap(_num_buckets, o._num_buckets);
        if (_alloc != o._alloc)
            swap(_alloc, o._alloc);
        if (_group_alloc != o._group_alloc)
            swap(_group_alloc, o._group_alloc);
    }

    
    void clear()
    {
        _free_groups();
        _num_buckets = 0;
        _table_size = 0;
    }

    inline allocator_type get_allocator() const
    {
        return _alloc;
    }


    
    
    
    
    
    size_type size() const           { return _table_size; }
    size_type max_size() const       { return _alloc.max_size(); }
    bool empty() const               { return _table_size == 0; }
    size_type num_nonempty() const   { return _num_buckets; }

    
    void resize(size_type new_size)
    {
        group_size_type sz = num_groups(new_size);
        group_size_type old_sz = (group_size_type)(_last_group - _first_group);

        if (sz != old_sz)
        {
            
            
            group_type *first = 0, *last = 0;
            if (sz)
            {
                _alloc_group_array(sz, first, last);
                memcpy(static_cast<void *>(first), _first_group, sizeof(*first) * (std::min)(sz, old_sz));
            }

            if (sz < old_sz)
            {
                for (group_type *g = _first_group + sz; g != _last_group; ++g)
                    g->destruct(_alloc);
            }
            else
                std::uninitialized_fill(first + old_sz, last, group_type());

            _free_group_array(_first_group, _last_group);
            _first_group = first;
            _last_group  = last;
        }
#if 0
        
        
        
        if (new_size < _table_size)
        {
            
            if (pos_in_group(new_size) > 0)     
                groups.back().erase(_alloc, groups.back().begin() + pos_in_group(new_size),
                                    groups.back().end());
            _num_buckets = 0;                   
            for (const group_type *group = _first_group; group != _last_group; ++group)
                _num_buckets += group->num_nonempty();
        }
#endif
        _table_size = new_size;
    }

    
    
    bool test(size_type i) const
    {
        
        return which_group(i).test(pos_in_group(i));
    }

    
    
    bool test_strict(size_type i) const
    {
        
        return which_group(i).test_strict(pos_in_group(i));
    }

    friend struct GrpPos;

    struct GrpPos
    {
        typedef typename sparsetable::ne_iterator ne_iter;
        GrpPos(const sparsetable &table, size_type i) :
            grp(table.which_group(i)), pos(table.pos_in_group(i)) {}

        bool test_strict() const { return grp.test_strict(pos); }
        bool test() const        { return grp.test(pos); }
        typename sparsetable::reference unsafe_get() const { return  grp.unsafe_get(pos); }
        ne_iter get_iter(typename sparsetable::reference ref)
        {
            return ne_iter((group_type *)&grp, &ref);
        }

        void erase(sparsetable &table) 
        {
            assert(table._num_buckets);
            ((group_type &)grp).erase(table._alloc, pos);
            --table._num_buckets;
        }

    private:
        GrpPos* operator=(const GrpPos&);

        const group_type &grp;
        typename group_type::size_type pos;
    };

    bool test(iterator pos) const
    {
        return which_group(pos.pos).test(pos_in_group(pos.pos));
    }

    bool test(const_iterator pos) const
    {
        return which_group(pos.pos).test(pos_in_group(pos.pos));
    }

    
    
    
    
    reference unsafe_get(size_type i) const
    {
        assert(i < _table_size);
        
        return which_group(i).unsafe_get(pos_in_group(i));
    }

    
    const_ne_iterator get_iter(size_type i) const
    {
        

        size_type grp_idx = group_num(i);

        return const_ne_iterator(_first_group + grp_idx,
                                 (_first_group[grp_idx].ne_begin() +
                                  _first_group[grp_idx].pos_to_offset(pos_in_group(i))));
    }

    const_ne_iterator get_iter(size_type i, ColIterator col_it) const
    {
        return const_ne_iterator(_first_group + group_num(i), col_it);
    }

    
    ne_iterator get_iter(size_type i)
    {
        

        size_type grp_idx = group_num(i);

        return ne_iterator(_first_group + grp_idx,
                           (_first_group[grp_idx].ne_begin() +
                            _first_group[grp_idx].pos_to_offset(pos_in_group(i))));
    }

    ne_iterator get_iter(size_type i, ColIterator col_it)
    {
        return ne_iterator(_first_group + group_num(i), col_it);
    }

    
    size_type get_pos(const const_ne_iterator& it) const
    {
        difference_type current_row = it.row_current - _first_group;
        difference_type current_col = (it.col_current - _first_group[current_row].ne_begin());
        return ((current_row * SPP_GROUP_SIZE) +
                _first_group[current_row].offset_to_pos(current_col));
    }

    
    
    template <class Val>
    reference set(size_type i, Val &val)
    {
        assert(i < _table_size);
        group_type &group = which_group(i);
        typename group_type::size_type old_numbuckets = group.num_nonempty();
        pointer p(group.set(_alloc, pos_in_group(i), val));
        _num_buckets += group.num_nonempty() - old_numbuckets;
        return *p;
    }

    
    void move(size_type i, reference val)
    {
        assert(i < _table_size);
        which_group(i).set(_alloc, pos_in_group(i), val);
        ++_num_buckets;
    }

    
    
    void erase(size_type i)
    {
        assert(i < _table_size);

        GroupsReference grp(which_group(i));
        typename group_type::size_type old_numbuckets = grp.num_nonempty();
        grp.erase(_alloc, pos_in_group(i));
        _num_buckets += grp.num_nonempty() - old_numbuckets;
    }

    void erase(iterator pos)
    {
        erase(pos.pos);
    }

    void erase(iterator start_it, iterator end_it)
    {
        
        
        for (; start_it != end_it; ++start_it)
            erase(start_it);
    }

    const_ne_iterator erase(const_ne_iterator it)
    {
        ne_iterator res(it);
        if (res.row_current->erase_ne(_alloc, res))
            _num_buckets--;
        return res;
    }

    const_ne_iterator erase(const_ne_iterator f, const_ne_iterator l)
    {
        size_t diff = l - f;
        while (diff--)
            f = erase(f);
        return f;
    }

    
    
    

private:
    
    typedef unsigned long MagicNumberType;
    static const MagicNumberType MAGIC_NUMBER = 0x24687531;

    
    
    
    
    
    
    
    
    template <typename OUTPUT, typename IntType>
    static bool write_32_or_64(OUTPUT* fp, IntType value)
    {
        if (value < 0xFFFFFFFFULL)        
        {
            if (!sparsehash_internal::write_bigendian_number(fp, value, 4))
                return false;
        }
        else
        {
            if (!sparsehash_internal::write_bigendian_number(fp, 0xFFFFFFFFUL, 4))
                return false;
            if (!sparsehash_internal::write_bigendian_number(fp, value, 8))
                return false;
        }
        return true;
    }

    template <typename INPUT, typename IntType>
    static bool read_32_or_64(INPUT* fp, IntType *value)
    {
        
        MagicNumberType first4 = 0;   
        if (!sparsehash_internal::read_bigendian_number(fp, &first4, 4))
            return false;

        if (first4 < 0xFFFFFFFFULL)
        {
            *value = first4;
        }
        else
        {
            if (!sparsehash_internal::read_bigendian_number(fp, value, 8))
                return false;
        }
        return true;
    }

public:
    
    

    template <typename OUTPUT>
    bool write_metadata(OUTPUT *fp) const
    {
        if (!write_32_or_64(fp, MAGIC_NUMBER))  return false;
        if (!write_32_or_64(fp, _table_size))  return false;
        if (!write_32_or_64(fp, _num_buckets))  return false;

        for (const group_type *group = _first_group; group != _last_group; ++group)
            if (group->write_metadata(fp) == false)
                return false;
        return true;
    }

    
    template <typename INPUT>
    bool read_metadata(INPUT *fp)
    {
        size_type magic_read = 0;
        if (!read_32_or_64(fp, &magic_read))  return false;
        if (magic_read != MAGIC_NUMBER)
        {
            clear();                        
            return false;
        }

        if (!read_32_or_64(fp, &_table_size))  return false;
        if (!read_32_or_64(fp, &_num_buckets))  return false;

        resize(_table_size);                    
        for (group_type *group = _first_group; group != _last_group; ++group)
            if (group->read_metadata(_alloc, fp) == false)
                return false;
        return true;
    }

    
    
    
    
    bool write_nopointer_data(FILE *fp) const
    {
        for (const_ne_iterator it = ne_begin(); it != ne_end(); ++it)
            if (!fwrite(&*it, sizeof(*it), 1, fp))
                return false;
        return true;
    }

    
    bool read_nopointer_data(FILE *fp)
    {
        for (ne_iterator it = ne_begin(); it != ne_end(); ++it)
            if (!fread(reinterpret_cast<void*>(&(*it)), sizeof(*it), 1, fp))
                return false;
        return true;
    }

    
    
    
    
    

    typedef sparsehash_internal::pod_serializer<value_type> NopointerSerializer;

    
    template <typename ValueSerializer, typename OUTPUT>
    bool serialize(ValueSerializer serializer, OUTPUT *fp)
    {
        if (!write_metadata(fp))
            return false;
        for (const_ne_iterator it = ne_begin(); it != ne_end(); ++it)
            if (!serializer(fp, *it))
                return false;
        return true;
    }

    
    template <typename ValueSerializer, typename INPUT>
    bool unserialize(ValueSerializer serializer, INPUT *fp)
    {
        clear();
        if (!read_metadata(fp))
            return false;
        for (ne_iterator it = ne_begin(); it != ne_end(); ++it)
            if (!serializer(fp, &*it))
                return false;
        return true;
    }

    
    
    
    bool operator==(const sparsetable& x) const
    {
        return (_table_size == x._table_size &&
                _num_buckets == x._num_buckets &&
                _first_group == x._first_group);
    }

    bool operator<(const sparsetable& x) const
    {
        return std::lexicographical_compare(begin(), end(), x.begin(), x.end());
    }
    bool operator!=(const sparsetable& x) const { return !(*this == x); }
    bool operator<=(const sparsetable& x) const { return !(x < *this); }
    bool operator>(const sparsetable& x)  const { return x < *this; }
    bool operator>=(const sparsetable& x) const { return !(*this < x); }


private:
    
    
    group_type *     _first_group;
    group_type *     _last_group;
    size_type        _table_size;          
    size_type        _num_buckets;         
    group_alloc_type _group_alloc;
    allocator_type   _alloc;
};

























#define JUMP_(key, num_probes)    ( num_probes )




template <class Value, class Key, class HashFcn,
          class ExtractKey, class SetKey, class EqualKey, class Alloc>
class sparse_hashtable
{
public:
    typedef Key                                        key_type;
    typedef Value                                      value_type;
    typedef HashFcn                                    hasher; 
    typedef EqualKey                                   key_equal;
    typedef Alloc                                      allocator_type;

    typedef typename allocator_type::size_type         size_type;
    typedef typename allocator_type::difference_type   difference_type;
    typedef value_type&                                reference;
    typedef const value_type&                          const_reference;
    typedef value_type*                                pointer;
    typedef const value_type*                          const_pointer;

    
    typedef sparsetable<value_type, allocator_type>   Table;
    typedef typename Table::ne_iterator               ne_it;
    typedef typename Table::const_ne_iterator         cne_it;
    typedef typename Table::destructive_iterator      dest_it;
    typedef typename Table::ColIterator               ColIterator;

    typedef ne_it                                     iterator;
    typedef cne_it                                    const_iterator;
    typedef dest_it                                   destructive_iterator;

    
    
    typedef iterator                                  local_iterator;
    typedef const_iterator                            const_local_iterator;

    
    
    static const int HT_OCCUPANCY_PCT; 

    
    
    
    
    static const int HT_EMPTY_PCT; 

    
    
    
    
    
    static const size_type HT_MIN_BUCKETS = 4;

    
    
    
    
    static const size_type HT_DEFAULT_STARTING_BUCKETS = 32;

    
    
    iterator       begin()        { return _mk_iterator(table.ne_begin());  }
    iterator       end()          { return _mk_iterator(table.ne_end());    }
    const_iterator begin() const  { return _mk_const_iterator(table.ne_cbegin()); }
    const_iterator end() const    { return _mk_const_iterator(table.ne_cend());   }
    const_iterator cbegin() const { return _mk_const_iterator(table.ne_cbegin()); }
    const_iterator cend() const   { return _mk_const_iterator(table.ne_cend());   }

    
    
    
    
    
    
    local_iterator begin(size_type i)
    {
        return _mk_iterator(table.test(i) ? table.get_iter(i) : table.ne_end());
    }

    local_iterator end(size_type i)
    {
        local_iterator it = begin(i);
        if (table.test(i))
            ++it;
        return _mk_iterator(it);
    }

    const_local_iterator begin(size_type i) const
    {
        return _mk_const_iterator(table.test(i) ? table.get_iter(i) : table.ne_cend());
    }

    const_local_iterator end(size_type i) const
    {
        const_local_iterator it = begin(i);
        if (table.test(i))
            ++it;
        return _mk_const_iterator(it);
    }

    const_local_iterator cbegin(size_type i) const { return begin(i); }
    const_local_iterator cend(size_type i)   const { return end(i); }

    
    
    destructive_iterator destructive_begin()       { return _mk_destructive_iterator(table.destructive_begin()); }
    destructive_iterator destructive_end()         { return _mk_destructive_iterator(table.destructive_end());   }


    
    
    hasher hash_funct() const               { return settings; }
    key_equal key_eq() const                { return key_info; }
    allocator_type get_allocator() const    { return table.get_allocator(); }

    
    unsigned int num_table_copies() const { return settings.num_ht_copies(); }

private:
    
    
    
    
    
    enum MoveDontCopyT {MoveDontCopy, MoveDontGrow};

    
    
    iterator             _mk_iterator(ne_it it) const               { return it; }
    const_iterator       _mk_const_iterator(cne_it it) const        { return it; }
    destructive_iterator _mk_destructive_iterator(dest_it it) const { return it; }

public:
    size_type size() const              { return table.num_nonempty(); }
    size_type max_size() const          { return table.max_size(); }
    bool empty() const                  { return size() == 0; }
    size_type bucket_count() const      { return table.size(); }
    size_type max_bucket_count() const  { return max_size(); }
    
    
    size_type bucket_size(size_type i) const
    {
        return (size_type)(begin(i) == end(i) ? 0 : 1);
    }

private:
    
    
    static const size_type ILLEGAL_BUCKET = size_type(-1);

    
    
    
    
    bool _maybe_shrink()
    {
        assert((bucket_count() & (bucket_count()-1)) == 0); 
        assert(bucket_count() >= HT_MIN_BUCKETS);
        bool retval = false;

        
        
        
        
        
        
        const size_type num_remain = table.num_nonempty();
        const size_type shrink_threshold = settings.shrink_threshold();
        if (shrink_threshold > 0 && num_remain < shrink_threshold &&
            bucket_count() > HT_DEFAULT_STARTING_BUCKETS)
        {
            const float shrink_factor = settings.shrink_factor();
            size_type sz = (size_type)(bucket_count() / 2);    
            while (sz > HT_DEFAULT_STARTING_BUCKETS &&
                   num_remain < static_cast<size_type>(sz * shrink_factor))
            {
                sz /= 2;                            
            }
            sparse_hashtable tmp(MoveDontCopy, *this, sz);
            swap(tmp);                            
            retval = true;
        }
        settings.set_consider_shrink(false);   
        return retval;
    }

    
    
    
    
    bool _resize_delta(size_type delta)
    {
        bool did_resize = false;
        if (settings.consider_shrink())
        {
            
            if (_maybe_shrink())
                did_resize = true;
        }
        if (table.num_nonempty() >=
            (std::numeric_limits<size_type>::max)() - delta)
        {
            throw_exception(std::length_error("resize overflow"));
        }

        size_type num_occupied = (size_type)(table.num_nonempty() + num_deleted);

        if (bucket_count() >= HT_MIN_BUCKETS &&
             (num_occupied + delta) <= settings.enlarge_threshold())
            return did_resize;                       

        
        
        
        
        
        const size_type needed_size =
                  settings.min_buckets((size_type)(num_occupied + delta), (size_type)0);

        if (needed_size <= bucket_count())      
            return did_resize;

        size_type resize_to = settings.min_buckets((size_type)(num_occupied + delta), bucket_count());

        if (resize_to < needed_size &&    
            resize_to < (std::numeric_limits<size_type>::max)() / 2)
        {
            
            
            
            
            
            
            
            const size_type target =
                static_cast<size_type>(settings.shrink_size((size_type)(resize_to*2)));
            if (table.num_nonempty() + delta >= target)
            {
                
                resize_to *= 2;
            }
        }

        sparse_hashtable tmp(MoveDontCopy, *this, resize_to);
        swap(tmp);                             
        return true;
    }

    
    
    void _copy_from(const sparse_hashtable &ht, size_type min_buckets_wanted)
    {
        clear();            

        
        const size_type resize_to = settings.min_buckets(ht.size(), min_buckets_wanted);

        if (resize_to > bucket_count())
        {
            
            table.resize(resize_to);               
            settings.reset_thresholds(bucket_count());
        }

        
        
        
        assert((bucket_count() & (bucket_count()-1)) == 0);      
        for (const_iterator it = ht.begin(); it != ht.end(); ++it)
        {
            size_type num_probes = 0;              
            size_type bucknum;
            const size_type bucket_count_minus_one = bucket_count() - 1;
            for (bucknum = hash(get_key(*it)) & bucket_count_minus_one;
                 table.test(bucknum);                                   
                 bucknum = (bucknum + JUMP_(key, num_probes)) & bucket_count_minus_one)
            {
                ++num_probes;
                assert(num_probes < bucket_count()
                       && "Hashtable is full: an error in key_equal<> or hash<>");
            }
            table.set(bucknum, *it);               
        }
        settings.inc_num_ht_copies();
    }

    
    
    
    
    void _move_from(MoveDontCopyT mover, sparse_hashtable &ht,
                   size_type min_buckets_wanted)
    {
        clear();

        
        size_type resize_to;
        if (mover == MoveDontGrow)
            resize_to = ht.bucket_count();       
        else                                     
            resize_to = settings.min_buckets(ht.size(), min_buckets_wanted);
        if (resize_to > bucket_count())
        {
            
            table.resize(resize_to);               
            settings.reset_thresholds(bucket_count());
        }

        
        
        
        assert((bucket_count() & (bucket_count()-1)) == 0);      
        const size_type bucket_count_minus_one = (const size_type)(bucket_count() - 1);

        
        for (destructive_iterator it = ht.destructive_begin();
              it != ht.destructive_end(); ++it)
        {
            size_type num_probes = 0;
            size_type bucknum;
            for (bucknum = hash(get_key(*it)) & bucket_count_minus_one;
                 table.test(bucknum);                          
                 bucknum = (size_type)((bucknum + JUMP_(key, num_probes)) & (bucket_count()-1)))
            {
                ++num_probes;
                assert(num_probes < bucket_count()
                       && "Hashtable is full: an error in key_equal<> or hash<>");
            }
            table.move(bucknum, *it);    
        }
        settings.inc_num_ht_copies();
    }


    
public:
    
    
    
    
    void resize(size_type req_elements)
    {
        
        if (settings.consider_shrink() || req_elements == 0)
            _maybe_shrink();
        if (req_elements > table.num_nonempty())    
            _resize_delta((size_type)(req_elements - table.num_nonempty()));
    }

    
    
    
    
    
    void get_resizing_parameters(float* shrink, float* grow) const
    {
        *shrink = settings.shrink_factor();
        *grow = settings.enlarge_factor();
    }

    float get_shrink_factor() const  { return settings.shrink_factor(); }
    float get_enlarge_factor() const { return settings.enlarge_factor(); }

    void set_resizing_parameters(float shrink, float grow) 
    {
        settings.set_resizing_parameters(shrink, grow);
        settings.reset_thresholds(bucket_count());
    }

    void set_shrink_factor(float shrink)
    {
        set_resizing_parameters(shrink, get_enlarge_factor());
    }

    void set_enlarge_factor(float grow)
    {
        set_resizing_parameters(get_shrink_factor(), grow);
    }

    
    
    
    
    
    explicit sparse_hashtable(size_type expected_max_items_in_table = 0,
                              const HashFcn& hf = HashFcn(),
                              const EqualKey& eql = EqualKey(),
                              const ExtractKey& ext = ExtractKey(),
                              const SetKey& set = SetKey(),
                              const allocator_type& alloc = allocator_type())
        : settings(hf),
          key_info(ext, set, eql),
          num_deleted(0),
          table((expected_max_items_in_table == 0
                 ? HT_DEFAULT_STARTING_BUCKETS
                 : settings.min_buckets(expected_max_items_in_table, 0)),
                alloc)
    {
        settings.reset_thresholds(bucket_count());
    }

    
    
    
    
    
    sparse_hashtable(const sparse_hashtable& ht,
                     size_type min_buckets_wanted = HT_DEFAULT_STARTING_BUCKETS)
        : settings(ht.settings),
          key_info(ht.key_info),
          num_deleted(0),
          table(0)
    {
        settings.reset_thresholds(bucket_count());
        _copy_from(ht, min_buckets_wanted);
    }

#if !defined(SPP_NO_CXX11_RVALUE_REFERENCES)

    sparse_hashtable(sparse_hashtable&& o, const allocator_type& alloc = allocator_type()) :
        settings(o.settings),
        key_info(o.key_info),
        num_deleted(0),
        table(HT_DEFAULT_STARTING_BUCKETS, alloc)
    {
        settings.reset_thresholds(bucket_count());
        this->swap(o);
    }

    sparse_hashtable& operator=(sparse_hashtable&& o)
    {
        this->swap(o);
        return *this;
    }
#endif

    sparse_hashtable(MoveDontCopyT mover,
                     sparse_hashtable& ht,
                     size_type min_buckets_wanted = HT_DEFAULT_STARTING_BUCKETS)
        : settings(ht.settings),
          key_info(ht.key_info),
          num_deleted(0),
          table(min_buckets_wanted, ht.table.get_allocator())
          
    {
        settings.reset_thresholds(bucket_count());
        _move_from(mover, ht, min_buckets_wanted);
    }

    sparse_hashtable& operator=(const sparse_hashtable& ht)
    {
        if (&ht == this)
            return *this;        
        settings = ht.settings;
        key_info = ht.key_info;
        num_deleted = ht.num_deleted;

        
        _copy_from(ht, HT_MIN_BUCKETS);

        
        return *this;
    }

    
    void swap(sparse_hashtable& ht)
    {
        using std::swap;

        swap(settings, ht.settings);
        swap(key_info, ht.key_info);
        swap(num_deleted, ht.num_deleted);
        table.swap(ht.table);
        settings.reset_thresholds(bucket_count());  
        ht.settings.reset_thresholds(ht.bucket_count());
        
    }

    
    void clear()
    {
        if (!empty() || num_deleted != 0)
        {
            table.clear();
            table = Table(HT_DEFAULT_STARTING_BUCKETS, table.get_allocator());
        }
        settings.reset_thresholds(bucket_count());
        num_deleted = 0;
    }

    
private:

    enum pos_type { pt_empty = 0, pt_erased, pt_full };
    
    class Position
    {
    public:

        Position() : _t(pt_empty) {}
        Position(pos_type t, size_type idx) : _t(t), _idx(idx) {}

        pos_type  _t;
        size_type _idx;
    };

    
    
    
    
    
    
    Position _find_position(const key_type &key) const
    {
        size_type num_probes = 0;                    
        const size_type bucket_count_minus_one = (const size_type)(bucket_count() - 1);
        size_type bucknum = hash(key) & bucket_count_minus_one;
        Position pos;

        while (1)
        {
            
            
            typename Table::GrpPos grp_pos(table, bucknum);

            if (!grp_pos.test_strict())
            {
                
                return pos._t ? pos : Position(pt_empty, bucknum);
            }
            else if (grp_pos.test())
            {
                reference ref(grp_pos.unsafe_get());

                if (equals(key, get_key(ref)))
                    return Position(pt_full, bucknum);
            }
            else if (pos._t == pt_empty)
            {
                
                pos._t   = pt_erased;
                pos._idx = bucknum;
            }

            ++num_probes;                        
            bucknum = (size_type)((bucknum + JUMP_(key, num_probes)) & bucket_count_minus_one);
            assert(num_probes < bucket_count()
                   && "Hashtable is full: an error in key_equal<> or hash<>");
        }
    }

public:
    
    
    
    iterator find(const key_type& key)
    {
        size_type num_probes = 0;              
        const size_type bucket_count_minus_one = bucket_count() - 1;
        size_type bucknum = hash(key) & bucket_count_minus_one;

        while (1)                        
        {
            typename Table::GrpPos grp_pos(table, bucknum);

            if (!grp_pos.test_strict())
                return end();            
            if (grp_pos.test())
            {
                reference ref(grp_pos.unsafe_get());

                if (equals(key, get_key(ref)))
                    return grp_pos.get_iter(ref);
            }
            ++num_probes;                        
            bucknum = (bucknum + JUMP_(key, num_probes)) & bucket_count_minus_one;
            assert(num_probes < bucket_count()
                   && "Hashtable is full: an error in key_equal<> or hash<>");
        }
    }

    
    
    const_iterator find(const key_type& key) const
    {
        size_type num_probes = 0;              
        const size_type bucket_count_minus_one = bucket_count() - 1;
        size_type bucknum = hash(key) & bucket_count_minus_one;

        while (1)                        
        {
            typename Table::GrpPos grp_pos(table, bucknum);

            if (!grp_pos.test_strict())
                return end();            
            else if (grp_pos.test())
            {
                reference ref(grp_pos.unsafe_get());

                if (equals(key, get_key(ref)))
                    return _mk_const_iterator(table.get_iter(bucknum, &ref));
            }
            ++num_probes;                        
            bucknum = (bucknum + JUMP_(key, num_probes)) & bucket_count_minus_one;
            assert(num_probes < bucket_count()
                   && "Hashtable is full: an error in key_equal<> or hash<>");
        }
    }

    
    
    
    size_type bucket(const key_type& key) const
    {
        Position pos = _find_position(key);
        return pos._idx;
    }

    
    
    size_type count(const key_type &key) const
    {
        Position pos = _find_position(key);
        return (size_type)(pos._t == pt_full ? 1 : 0);
    }

    
    
    std::pair<iterator,iterator> equal_range(const key_type& key)
    {
        iterator pos = find(key);      
        if (pos == end())
            return std::pair<iterator,iterator>(pos, pos);
        else
        {
            const iterator startpos = pos++;
            return std::pair<iterator,iterator>(startpos, pos);
        }
    }

    std::pair<const_iterator,const_iterator> equal_range(const key_type& key) const
    {
        const_iterator pos = find(key);      
        if (pos == end())
            return std::pair<const_iterator,const_iterator>(pos, pos);
        else
        {
            const const_iterator startpos = pos++;
            return std::pair<const_iterator,const_iterator>(startpos, pos);
        }
    }


    
private:
    
    template <class T>
    reference _insert_at(T& obj, size_type pos, bool erased)
    {
        if (size() >= max_size())
        {
            throw_exception(std::length_error("insert overflow"));
        }
        if (erased)
        {
            assert(num_deleted);
            --num_deleted;
        }
        return table.set(pos, obj);
    }

    
    template <class T>
    std::pair<iterator, bool> _insert_noresize(T& obj)
    {
        Position pos = _find_position(get_key(obj));
        bool already_there = (pos._t == pt_full);

        if (!already_there)
        {
            reference ref(_insert_at(obj, pos._idx, pos._t == pt_erased));
            return std::pair<iterator, bool>(_mk_iterator(table.get_iter(pos._idx, &ref)), true);
        }
        return std::pair<iterator,bool>(_mk_iterator(table.get_iter(pos._idx)), false);
    }

    
    
    template <class ForwardIterator>
    void _insert(ForwardIterator f, ForwardIterator l, std::forward_iterator_tag )
    {
        int64_t dist = std::distance(f, l);
        if (dist < 0 ||  static_cast<size_t>(dist) >= (std::numeric_limits<size_type>::max)())
            throw_exception(std::length_error("insert-range overflow"));

        _resize_delta(static_cast<size_type>(dist));

        for (; dist > 0; --dist, ++f)
            _insert_noresize(*f);
    }

    
    template <class InputIterator>
    void _insert(InputIterator f, InputIterator l, std::input_iterator_tag )
    {
        for (; f != l; ++f)
            _insert(*f);
    }

public:

#if !defined(SPP_NO_CXX11_VARIADIC_TEMPLATES)
    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        _resize_delta(1);
        value_type obj(std::forward<Args>(args)...);
        return _insert_noresize(obj);
    }
#endif

    
    std::pair<iterator, bool> insert(const_reference obj)
    {
        _resize_delta(1);                      
        return _insert_noresize(obj);
    }

#if !defined(SPP_NO_CXX11_RVALUE_REFERENCES)
    template< class P >
    std::pair<iterator, bool> insert(P &&obj)
    {
        _resize_delta(1);                      
        value_type val(std::forward<P>(obj));
        return _insert_noresize(val);
    }
#endif

    
    template <class InputIterator>
    void insert(InputIterator f, InputIterator l)
    {
        
        _insert(f, l,
               typename std::iterator_traits<InputIterator>::iterator_category());
    }

    
    
#if !defined(SPP_NO_CXX11_VARIADIC_TEMPLATES)
    template <class DefaultValue, class KT>
    value_type& find_or_insert(KT&& key)
#else
    template <class DefaultValue>
    value_type& find_or_insert(const key_type& key)
#endif
    {
        size_type num_probes = 0;              
        const size_type bucket_count_minus_one = bucket_count() - 1;
        size_type bucknum = hash(key) & bucket_count_minus_one;
        DefaultValue default_value;
        size_type erased_pos = 0;
        bool erased = false;

        while (1)                        
        {
            typename Table::GrpPos grp_pos(table, bucknum);

            if (!grp_pos.test_strict())
            {
                
#if !defined(SPP_NO_CXX11_VARIADIC_TEMPLATES)
                auto&& def(default_value(std::forward<KT>(key)));
#else
                value_type def(default_value(key));
#endif                
                if (_resize_delta(1))
                {
                    
                    
                    return *(_insert_noresize(def).first);
                }
                else
                {
                    
                    return _insert_at(def, erased ? erased_pos : bucknum, erased);
                }
            }
            if (grp_pos.test())
            {
                reference ref(grp_pos.unsafe_get());

                if (equals(key, get_key(ref)))
                    return ref;
            }
            else if (!erased)
            {
                
                erased_pos = bucknum;
                erased = true;
            }

            ++num_probes;                        
            bucknum = (bucknum + JUMP_(key, num_probes)) & bucket_count_minus_one;
            assert(num_probes < bucket_count()
                   && "Hashtable is full: an error in key_equal<> or hash<>");
        }
    }

    size_type erase(const key_type& key)
    {
        size_type num_probes = 0;              
        const size_type bucket_count_minus_one = bucket_count() - 1;
        size_type bucknum = hash(key) & bucket_count_minus_one;

        while (1)                        
        {
            typename Table::GrpPos grp_pos(table, bucknum);

            if (!grp_pos.test_strict())
                return 0;            
            if (grp_pos.test())
            {
                reference ref(grp_pos.unsafe_get());

                if (equals(key, get_key(ref)))
                {
                    grp_pos.erase(table);
                    ++num_deleted;
                    settings.set_consider_shrink(true); 
                    return 1;                           
                }
            }
            ++num_probes;                        
            bucknum = (bucknum + JUMP_(key, num_probes)) & bucket_count_minus_one;
            assert(num_probes < bucket_count()
                   && "Hashtable is full: an error in key_equal<> or hash<>");
        }
    }

    const_iterator erase(const_iterator pos)
    {
        if (pos == cend())
            return cend();                 

        const_iterator nextpos = table.erase(pos);
        ++num_deleted;
        settings.set_consider_shrink(true);
        return nextpos;
    }

    const_iterator erase(const_iterator f, const_iterator l)
    {
        if (f == cend())
            return cend();                

        size_type num_before = table.num_nonempty();
        const_iterator nextpos = table.erase(f, l);
        num_deleted += num_before - table.num_nonempty();
        settings.set_consider_shrink(true);
        return nextpos;
    }

    
    
    
    void set_deleted_key(const key_type&)
    {
    }

    void clear_deleted_key()
    {
    }

    bool operator==(const sparse_hashtable& ht) const
    {
        if (this == &ht)
            return true;

        if (size() != ht.size())
            return false;

        for (const_iterator it = begin(); it != end(); ++it)
        {
            const_iterator it2 = ht.find(get_key(*it));
            if ((it2 == ht.end()) || (*it != *it2))
                return false;
        }

        return true;
    }

    bool operator!=(const sparse_hashtable& ht) const
    {
        return !(*this == ht);
    }


    
    
    
    
    
    
    
    
    
    
    
    
    
    template <typename OUTPUT>
    bool write_metadata(OUTPUT *fp)
    {
        return table.write_metadata(fp);
    }

    template <typename INPUT>
    bool read_metadata(INPUT *fp)
    {
        num_deleted = 0;            
        const bool result = table.read_metadata(fp);
        settings.reset_thresholds(bucket_count());
        return result;
    }

    
    template <typename OUTPUT>
    bool write_nopointer_data(OUTPUT *fp)
    {
        return table.write_nopointer_data(fp);
    }

    
    template <typename INPUT>
    bool read_nopointer_data(INPUT *fp)
    {
        return table.read_nopointer_data(fp);
    }

    
    
    
    
    

    typedef sparsehash_internal::pod_serializer<value_type> NopointerSerializer;

    
    template <typename ValueSerializer, typename OUTPUT>
    bool serialize(ValueSerializer serializer, OUTPUT *fp)
    {
        return table.serialize(serializer, fp);
    }

    
    template <typename ValueSerializer, typename INPUT>
    bool unserialize(ValueSerializer serializer, INPUT *fp)
    {
        num_deleted = 0;            
        const bool result = table.unserialize(serializer, fp);
        settings.reset_thresholds(bucket_count());
        return result;
    }

private:

    
    
    
    
    
    struct Settings :
        sparsehash_internal::sh_hashtable_settings<key_type, hasher,
                                                   size_type, HT_MIN_BUCKETS>
    {
        explicit Settings(const hasher& hf)
            : sparsehash_internal::sh_hashtable_settings<key_type, hasher, size_type,
              HT_MIN_BUCKETS>
              (hf, HT_OCCUPANCY_PCT / 100.0f, HT_EMPTY_PCT / 100.0f) {}
    };

    
    
     
    class KeyInfo : public ExtractKey, public SetKey, public EqualKey
    {
    public:
        KeyInfo(const ExtractKey& ek, const SetKey& sk, const EqualKey& eq)
            : ExtractKey(ek), SetKey(sk), EqualKey(eq)
        {
        }

        
        typename ExtractKey::result_type get_key(const_reference v) const
        {
            return ExtractKey::operator()(v);
        }

        bool equals(const key_type& a, const key_type& b) const
        {
            return EqualKey::operator()(a, b);
        }
    };

    
    size_t hash(const key_type& v) const
    {
        return settings.hash(v);
    }

    bool equals(const key_type& a, const key_type& b) const
    {
        return key_info.equals(a, b);
    }

    typename ExtractKey::result_type get_key(const_reference v) const
    {
        return key_info.get_key(v);
    }

private:
    
    
    Settings  settings;
    KeyInfo   key_info;
    size_type num_deleted;
    Table     table;         
};

#undef JUMP_


template <class V, class K, class HF, class ExK, class SetK, class EqK, class A>
const typename sparse_hashtable<V,K,HF,ExK,SetK,EqK,A>::size_type
sparse_hashtable<V,K,HF,ExK,SetK,EqK,A>::ILLEGAL_BUCKET;




template <class V, class K, class HF, class ExK, class SetK, class EqK, class A>
const int sparse_hashtable<V,K,HF,ExK,SetK,EqK,A>::HT_OCCUPANCY_PCT = 50;




template <class V, class K, class HF, class ExK, class SetK, class EqK, class A>
const int sparse_hashtable<V,K,HF,ExK,SetK,EqK,A>::HT_EMPTY_PCT
= static_cast<int>(0.4 *
                   sparse_hashtable<V,K,HF,ExK,SetK,EqK,A>::HT_OCCUPANCY_PCT);





template <class Key, class T,
          class HashFcn  = spp_hash<Key>,
          class EqualKey = std::equal_to<Key>,
          class Alloc    = SPP_DEFAULT_ALLOCATOR<std::pair<const Key, T> > >
class sparse_hash_map
{
public:
    typedef typename std::pair<const Key, T> value_type;

private:
    
    struct SelectKey
    {
        typedef const Key& result_type;

        inline const Key& operator()(const value_type& p) const
        {
            return p.first;
        }
    };

    struct SetKey
    {
        inline void operator()(value_type* value, const Key& new_key) const
        {
            *const_cast<Key*>(&value->first) = new_key;
        }
    };

    
    struct DefaultValue
    {
#if !defined(SPP_NO_CXX11_VARIADIC_TEMPLATES)
        template <class KT>
        inline value_type operator()(KT&& key)  const
        {
            return { std::forward<KT>(key), T() };
        }
#else
        inline value_type operator()(const Key& key)  const
        {
            return std::make_pair(key, T());
        }
#endif
    };

    
    typedef sparse_hashtable<value_type, Key, HashFcn, SelectKey,
                             SetKey, EqualKey, Alloc> ht;

public:
    typedef typename ht::key_type             key_type;
    typedef T                                 data_type;
    typedef T                                 mapped_type;
    typedef typename ht::hasher               hasher;
    typedef typename ht::key_equal            key_equal;
    typedef Alloc                             allocator_type;

    typedef typename ht::size_type            size_type;
    typedef typename ht::difference_type      difference_type;
    typedef typename ht::pointer              pointer;
    typedef typename ht::const_pointer        const_pointer;
    typedef typename ht::reference            reference;
    typedef typename ht::const_reference      const_reference;

    typedef typename ht::iterator             iterator;
    typedef typename ht::const_iterator       const_iterator;
    typedef typename ht::local_iterator       local_iterator;
    typedef typename ht::const_local_iterator const_local_iterator;

    
    iterator       begin()                         { return rep.begin(); }
    iterator       end()                           { return rep.end(); }
    const_iterator begin() const                   { return rep.cbegin(); }
    const_iterator end() const                     { return rep.cend(); }
    const_iterator cbegin() const                  { return rep.cbegin(); }
    const_iterator cend() const                    { return rep.cend(); }

    
    local_iterator begin(size_type i)              { return rep.begin(i); }
    local_iterator end(size_type i)                { return rep.end(i); }
    const_local_iterator begin(size_type i) const  { return rep.begin(i); }
    const_local_iterator end(size_type i) const    { return rep.end(i); }
    const_local_iterator cbegin(size_type i) const { return rep.cbegin(i); }
    const_local_iterator cend(size_type i) const   { return rep.cend(i); }

    
    
    allocator_type get_allocator() const           { return rep.get_allocator(); }
    hasher hash_funct() const                      { return rep.hash_funct(); }
    hasher hash_function() const                   { return hash_funct(); }
    key_equal key_eq() const                       { return rep.key_eq(); }


    
    
    explicit sparse_hash_map(size_type n = 0,
                             const hasher& hf = hasher(),
                             const key_equal& eql = key_equal(),
                             const allocator_type& alloc = allocator_type())
        : rep(n, hf, eql, SelectKey(), SetKey(), alloc)
    {
    }

    explicit sparse_hash_map(const allocator_type& alloc) :
        rep(0, hasher(), key_equal(), SelectKey(), SetKey(), alloc)
    {
    }

    sparse_hash_map(size_type n, const allocator_type& alloc) :
        rep(n, hasher(), key_equal(), SelectKey(), SetKey(), alloc)
    {
    }

    sparse_hash_map(size_type n, const hasher& hf, const allocator_type& alloc) :
        rep(n, hf, key_equal(), SelectKey(), SetKey(), alloc)
    {
    }

    template <class InputIterator>
    sparse_hash_map(InputIterator f, InputIterator l,
                    size_type n = 0,
                    const hasher& hf = hasher(),
                    const key_equal& eql = key_equal(),
                    const allocator_type& alloc = allocator_type())
        : rep(n, hf, eql, SelectKey(), SetKey(), alloc)
    {
        rep.insert(f, l);
    }

    template <class InputIterator>
    sparse_hash_map(InputIterator f, InputIterator l,
                    size_type n, const allocator_type& alloc)
        : rep(n, hasher(), key_equal(), SelectKey(), SetKey(), alloc)
    {
        rep.insert(f, l);
    }

    template <class InputIterator>
    sparse_hash_map(InputIterator f, InputIterator l,
                    size_type n, const hasher& hf, const allocator_type& alloc)
        : rep(n, hf, key_equal(), SelectKey(), SetKey(), alloc)
    {
        rep.insert(f, l);
    }

    sparse_hash_map(const sparse_hash_map &o) :
        rep(o.rep)
    {}

    sparse_hash_map(const sparse_hash_map &o,
                    const allocator_type& alloc) :
        rep(o.rep, alloc)
    {}

#if !defined(SPP_NO_CXX11_RVALUE_REFERENCES)
    sparse_hash_map(sparse_hash_map &&o) :
        rep(std::move(o.rep))
    {}

    sparse_hash_map(sparse_hash_map &&o,
                    const allocator_type& alloc) :
        rep(std::move(o.rep), alloc)
    {}

    sparse_hash_map& operator=(sparse_hash_map &&o) = default;
#endif

#if !defined(SPP_NO_CXX11_HDR_INITIALIZER_LIST)
    sparse_hash_map(std::initializer_list<value_type> init,
                    size_type n = 0,
                    const hasher& hf = hasher(),
                    const key_equal& eql = key_equal(),
                    const allocator_type& alloc = allocator_type())
        : rep(n, hf, eql, SelectKey(), SetKey(), alloc)
    {
        rep.insert(init.begin(), init.end());
    }

    sparse_hash_map(std::initializer_list<value_type> init,
                    size_type n, const allocator_type& alloc) :
        rep(n, hasher(), key_equal(), SelectKey(), SetKey(), alloc)
    {
        rep.insert(init.begin(), init.end());
    }

    sparse_hash_map(std::initializer_list<value_type> init,
                    size_type n, const hasher& hf, const allocator_type& alloc) :
        rep(n, hf, key_equal(), SelectKey(), SetKey(), alloc)
    {
        rep.insert(init.begin(), init.end());
    }

    sparse_hash_map& operator=(std::initializer_list<value_type> init)
    {
        rep.clear();
        rep.insert(init.begin(), init.end());
        return *this;
    }

    void insert(std::initializer_list<value_type> init)
    {
        rep.insert(init.begin(), init.end());
    }
#endif

    sparse_hash_map& operator=(const sparse_hash_map &o)
    {
        rep = o.rep;
        return *this;
    }

    void clear()                        { rep.clear(); }
    void swap(sparse_hash_map& hs)      { rep.swap(hs.rep); }

    
    
    size_type size() const              { return rep.size(); }
    size_type max_size() const          { return rep.max_size(); }
    bool empty() const                  { return rep.empty(); }
    size_type bucket_count() const      { return rep.bucket_count(); }
    size_type max_bucket_count() const  { return rep.max_bucket_count(); }

    size_type bucket_size(size_type i) const    { return rep.bucket_size(i); }
    size_type bucket(const key_type& key) const { return rep.bucket(key); }
    float     load_factor() const       { return size() * 1.0f / bucket_count(); }

    float max_load_factor() const      { return rep.get_enlarge_factor(); }
    void  max_load_factor(float grow)  { rep.set_enlarge_factor(grow); }

    float min_load_factor() const      { return rep.get_shrink_factor(); }
    void  min_load_factor(float shrink){ rep.set_shrink_factor(shrink); }

    void set_resizing_parameters(float shrink, float grow)
    {
        rep.set_resizing_parameters(shrink, grow);
    }

    void resize(size_type cnt)        { rep.resize(cnt); }
    void rehash(size_type cnt)        { resize(cnt); } 
    void reserve(size_type cnt)       { resize(cnt); } 

    
    
    iterator find(const key_type& key)                 { return rep.find(key); }
    const_iterator find(const key_type& key) const     { return rep.find(key); }
    bool contains(const key_type& key) const           { return rep.find(key) != rep.end(); }

#if !defined(SPP_NO_CXX11_VARIADIC_TEMPLATES)
    template <class KT>
    mapped_type& operator[](KT&& key)
    {
        return rep.template find_or_insert<DefaultValue>(std::forward<KT>(key)).second;
    }
#else
    mapped_type& operator[](const key_type& key)
    {
        return rep.template find_or_insert<DefaultValue>(key).second;
    }
#endif

    size_type count(const key_type& key) const         { return rep.count(key); }

    std::pair<iterator, iterator>
    equal_range(const key_type& key)             { return rep.equal_range(key); }

    std::pair<const_iterator, const_iterator>
    equal_range(const key_type& key) const       { return rep.equal_range(key); }

    mapped_type& at(const key_type& key)
    {
        iterator it = rep.find(key);
        if (it == rep.end())
            throw_exception(std::out_of_range("at: key not present"));
        return it->second;
    }

    const mapped_type& at(const key_type& key) const
    {
        const_iterator it = rep.find(key);
        if (it == rep.cend())
            throw_exception(std::out_of_range("at: key not present"));
        return it->second;
    }

#if !defined(SPP_NO_CXX11_VARIADIC_TEMPLATES)
    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        return rep.emplace(std::forward<Args>(args)...);
    }

    template <class... Args>
    iterator emplace_hint(const_iterator , Args&&... args)
    {
        return rep.emplace(std::forward<Args>(args)...).first;
    }
#endif

    
    
    std::pair<iterator, bool>
    insert(const value_type& obj)                    { return rep.insert(obj); }

#if !defined(SPP_NO_CXX11_RVALUE_REFERENCES)
    template< class P >
    std::pair<iterator, bool> insert(P&& obj)        { return rep.insert(std::forward<P>(obj)); }
#endif

    template <class InputIterator>
    void insert(InputIterator f, InputIterator l)    { rep.insert(f, l); }

    void insert(const_iterator f, const_iterator l)  { rep.insert(f, l); }

    iterator insert(iterator , const value_type& obj) { return insert(obj).first; }
    iterator insert(const_iterator , const value_type& obj) { return insert(obj).first; }

    
    
    
    void set_deleted_key(const key_type& key)   { rep.set_deleted_key(key); }
    void clear_deleted_key()                    { rep.clear_deleted_key();  }
    key_type deleted_key() const                { return rep.deleted_key(); }

    
    
    size_type erase(const key_type& key)               { return rep.erase(key); }
    iterator  erase(iterator it)                       { return rep.erase(it); }
    iterator  erase(iterator f, iterator l)            { return rep.erase(f, l); }
    iterator  erase(const_iterator it)                 { return rep.erase(it); }
    iterator  erase(const_iterator f, const_iterator l){ return rep.erase(f, l); }

    
    
    bool operator==(const sparse_hash_map& hs) const   { return rep == hs.rep; }
    bool operator!=(const sparse_hash_map& hs) const   { return rep != hs.rep; }


    
    
    
    
    

    
    
    
    
    
    typedef typename ht::NopointerSerializer NopointerSerializer;

    
    
    
    
    
    
    
    
    
    template <typename ValueSerializer, typename OUTPUT>
    bool serialize(ValueSerializer serializer, OUTPUT* fp)
    {
        return rep.serialize(serializer, fp);
    }

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    template <typename ValueSerializer, typename INPUT>
    bool unserialize(ValueSerializer serializer, INPUT* fp)
    {
        return rep.unserialize(serializer, fp);
    }

    
    
    
    template <typename OUTPUT>
    bool write_metadata(OUTPUT *fp)       { return rep.write_metadata(fp); }

    template <typename INPUT>
    bool read_metadata(INPUT *fp)         { return rep.read_metadata(fp); }

    template <typename OUTPUT>
    bool write_nopointer_data(OUTPUT *fp) { return rep.write_nopointer_data(fp); }

    template <typename INPUT>
    bool read_nopointer_data(INPUT *fp)   { return rep.read_nopointer_data(fp); }


private:
    
    
    ht rep;
};





template <class Value,
          class HashFcn  = spp_hash<Value>,
          class EqualKey = std::equal_to<Value>,
          class Alloc    = SPP_DEFAULT_ALLOCATOR<Value> >
class sparse_hash_set
{
private:
    
    struct Identity
    {
        typedef const Value& result_type;
        inline const Value& operator()(const Value& v) const { return v; }
    };

    struct SetKey
    {
        inline void operator()(Value* value, const Value& new_key) const
        {
            *value = new_key;
        }
    };

    typedef sparse_hashtable<Value, Value, HashFcn, Identity, SetKey,
                             EqualKey, Alloc> ht;

public:
    typedef typename ht::key_type              key_type;
    typedef typename ht::value_type            value_type;
    typedef typename ht::hasher                hasher;
    typedef typename ht::key_equal             key_equal;
    typedef Alloc                              allocator_type;

    typedef typename ht::size_type             size_type;
    typedef typename ht::difference_type       difference_type;
    typedef typename ht::const_pointer         pointer;
    typedef typename ht::const_pointer         const_pointer;
    typedef typename ht::const_reference       reference;
    typedef typename ht::const_reference       const_reference;

    typedef typename ht::const_iterator        iterator;
    typedef typename ht::const_iterator        const_iterator;
    typedef typename ht::const_local_iterator  local_iterator;
    typedef typename ht::const_local_iterator  const_local_iterator;


    
    iterator       begin() const             { return rep.begin(); }
    iterator       end() const               { return rep.end(); }
    const_iterator cbegin() const            { return rep.cbegin(); }
    const_iterator cend() const              { return rep.cend(); }

    
    local_iterator begin(size_type i) const  { return rep.begin(i); }
    local_iterator end(size_type i) const    { return rep.end(i); }
    local_iterator cbegin(size_type i) const { return rep.cbegin(i); }
    local_iterator cend(size_type i) const   { return rep.cend(i); }


    
    
    allocator_type get_allocator() const     { return rep.get_allocator(); }
    hasher         hash_funct() const        { return rep.hash_funct(); }
    hasher         hash_function() const     { return hash_funct(); }  
    key_equal      key_eq() const            { return rep.key_eq(); }


    
    
    explicit sparse_hash_set(size_type n = 0,
                             const hasher& hf = hasher(),
                             const key_equal& eql = key_equal(),
                             const allocator_type& alloc = allocator_type()) :
        rep(n, hf, eql, Identity(), SetKey(), alloc)
    {
    }

    explicit sparse_hash_set(const allocator_type& alloc) :
        rep(0, hasher(), key_equal(), Identity(), SetKey(), alloc)
    {
    }

    sparse_hash_set(size_type n, const allocator_type& alloc) :
        rep(n, hasher(), key_equal(), Identity(), SetKey(), alloc)
    {
    }

    sparse_hash_set(size_type n, const hasher& hf,
                    const allocator_type& alloc) :
        rep(n, hf, key_equal(), Identity(), SetKey(), alloc)
    {
    }

    template <class InputIterator>
    sparse_hash_set(InputIterator f, InputIterator l,
                    size_type n = 0,
                    const hasher& hf = hasher(),
                    const key_equal& eql = key_equal(),
                    const allocator_type& alloc = allocator_type())
        : rep(n, hf, eql, Identity(), SetKey(), alloc)
    {
        rep.insert(f, l);
    }

    template <class InputIterator>
    sparse_hash_set(InputIterator f, InputIterator l,
                    size_type n, const allocator_type& alloc)
        : rep(n, hasher(), key_equal(), Identity(), SetKey(), alloc)
    {
        rep.insert(f, l);
    }

    template <class InputIterator>
    sparse_hash_set(InputIterator f, InputIterator l,
                    size_type n, const hasher& hf, const allocator_type& alloc)
        : rep(n, hf, key_equal(), Identity(), SetKey(), alloc)
    {
        rep.insert(f, l);
    }

    sparse_hash_set(const sparse_hash_set &o) :
        rep(o.rep)
    {}

    sparse_hash_set(const sparse_hash_set &o,
                    const allocator_type& alloc) :
        rep(o.rep, alloc)
    {}

#if !defined(SPP_NO_CXX11_RVALUE_REFERENCES)
    sparse_hash_set(sparse_hash_set &&o) :
        rep(std::move(o.rep))
    {}

    sparse_hash_set(sparse_hash_set &&o,
                    const allocator_type& alloc) :
        rep(std::move(o.rep), alloc)
    {}
#endif

#if !defined(SPP_NO_CXX11_HDR_INITIALIZER_LIST)
    sparse_hash_set(std::initializer_list<value_type> init,
                    size_type n = 0,
                    const hasher& hf = hasher(),
                    const key_equal& eql = key_equal(),
                    const allocator_type& alloc = allocator_type()) :
        rep(n, hf, eql, Identity(), SetKey(), alloc)
    {
        rep.insert(init.begin(), init.end());
    }

    sparse_hash_set(std::initializer_list<value_type> init,
                    size_type n, const allocator_type& alloc) :
        rep(n, hasher(), key_equal(), Identity(), SetKey(), alloc)
    {
        rep.insert(init.begin(), init.end());
    }

    sparse_hash_set(std::initializer_list<value_type> init,
                    size_type n, const hasher& hf,
                    const allocator_type& alloc) :
        rep(n, hf, key_equal(), Identity(), SetKey(), alloc)
    {
        rep.insert(init.begin(), init.end());
    }

    sparse_hash_set& operator=(std::initializer_list<value_type> init)
    {
        rep.clear();
        rep.insert(init.begin(), init.end());
        return *this;
    }

    void insert(std::initializer_list<value_type> init)
    {
        rep.insert(init.begin(), init.end());
    }

#endif

    sparse_hash_set& operator=(const sparse_hash_set &o)
    {
        rep = o.rep;
        return *this;
    }

    void clear()                        { rep.clear(); }
    void swap(sparse_hash_set& hs)      { rep.swap(hs.rep); }


    
    
    size_type size() const              { return rep.size(); }
    size_type max_size() const          { return rep.max_size(); }
    bool empty() const                  { return rep.empty(); }
    size_type bucket_count() const      { return rep.bucket_count(); }
    size_type max_bucket_count() const  { return rep.max_bucket_count(); }

    size_type bucket_size(size_type i) const    { return rep.bucket_size(i); }
    size_type bucket(const key_type& key) const { return rep.bucket(key); }

    float     load_factor() const       { return size() * 1.0f / bucket_count(); }

    float max_load_factor() const      { return rep.get_enlarge_factor(); }
    void  max_load_factor(float grow)  { rep.set_enlarge_factor(grow); }

    float min_load_factor() const      { return rep.get_shrink_factor(); }
    void  min_load_factor(float shrink){ rep.set_shrink_factor(shrink); }

    void set_resizing_parameters(float shrink, float grow)
    {
        rep.set_resizing_parameters(shrink, grow);
    }

    void resize(size_type cnt)        { rep.resize(cnt); }
    void rehash(size_type cnt)        { resize(cnt); } 
    void reserve(size_type cnt)       { resize(cnt); } 

    
    
    iterator find(const key_type& key) const     { return rep.find(key); }
    bool contains(const key_type& key) const     { return rep.find(key) != rep.end(); }

    size_type count(const key_type& key) const   { return rep.count(key); }

    std::pair<iterator, iterator>
    equal_range(const key_type& key) const       { return rep.equal_range(key); }

#if !defined(SPP_NO_CXX11_VARIADIC_TEMPLATES)
    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        return rep.emplace(std::forward<Args>(args)...);
    }

    template <class... Args>
    iterator emplace_hint(const_iterator , Args&&... args)
    {
        return rep.emplace(std::forward<Args>(args)...).first;
    }
#endif

    
    
    std::pair<iterator, bool> insert(const value_type& obj)
    {
        std::pair<typename ht::iterator, bool> p = rep.insert(obj);
        return std::pair<iterator, bool>(p.first, p.second);   
    }

#if !defined(SPP_NO_CXX11_RVALUE_REFERENCES)
    template<class P>
    std::pair<iterator, bool> insert(P&& obj)        { return rep.insert(std::forward<P>(obj)); }
#endif

    template <class InputIterator>
    void insert(InputIterator f, InputIterator l)    { rep.insert(f, l); }

    void insert(const_iterator f, const_iterator l)  { rep.insert(f, l); }

    iterator insert(iterator , const value_type& obj) { return insert(obj).first; }

    
    
    void set_deleted_key(const key_type& key) { rep.set_deleted_key(key); }
    void clear_deleted_key()                  { rep.clear_deleted_key();  }
    key_type deleted_key() const              { return rep.deleted_key(); }

    
    
    size_type erase(const key_type& key)      { return rep.erase(key); }
    iterator  erase(iterator it)              { return rep.erase(it); }
    iterator  erase(iterator f, iterator l)   { return rep.erase(f, l); }

    
    
    bool operator==(const sparse_hash_set& hs) const { return rep == hs.rep; }
    bool operator!=(const sparse_hash_set& hs) const { return rep != hs.rep; }


    
    
    
    
    

    
    
    
    
    
    typedef typename ht::NopointerSerializer NopointerSerializer;

    
    
    
    
    
    
    
    
    
    template <typename ValueSerializer, typename OUTPUT>
    bool serialize(ValueSerializer serializer, OUTPUT* fp)
    {
        return rep.serialize(serializer, fp);
    }

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    template <typename ValueSerializer, typename INPUT>
    bool unserialize(ValueSerializer serializer, INPUT* fp)
    {
        return rep.unserialize(serializer, fp);
    }

    
    
    
    template <typename OUTPUT>
    bool write_metadata(OUTPUT *fp)       { return rep.write_metadata(fp); }

    template <typename INPUT>
    bool read_metadata(INPUT *fp)         { return rep.read_metadata(fp); }

    template <typename OUTPUT>
    bool write_nopointer_data(OUTPUT *fp) { return rep.write_nopointer_data(fp); }

    template <typename INPUT>
    bool read_nopointer_data(INPUT *fp)   { return rep.read_nopointer_data(fp); }

private:
    
    
    ht rep;
};

} 





template <class T, class Alloc>
inline void swap(spp_::sparsegroup<T,Alloc> &x, spp_::sparsegroup<T,Alloc> &y)
{
    x.swap(y);
}

template <class T, class Alloc>
inline void swap(spp_::sparsetable<T,Alloc> &x, spp_::sparsetable<T,Alloc> &y)
{
    x.swap(y);
}

template <class V, class K, class HF, class ExK, class SetK, class EqK, class A>
inline void swap(spp_::sparse_hashtable<V,K,HF,ExK,SetK,EqK,A> &x,
                 spp_::sparse_hashtable<V,K,HF,ExK,SetK,EqK,A> &y)
{
    x.swap(y);
}

template <class Key, class T, class HashFcn, class EqualKey, class Alloc>
inline void swap(spp_::sparse_hash_map<Key, T, HashFcn, EqualKey, Alloc>& hm1,
                 spp_::sparse_hash_map<Key, T, HashFcn, EqualKey, Alloc>& hm2)
{
    hm1.swap(hm2);
}

template <class Val, class HashFcn, class EqualKey, class Alloc>
inline void swap(spp_::sparse_hash_set<Val, HashFcn, EqualKey, Alloc>& hs1,
                 spp_::sparse_hash_set<Val, HashFcn, EqualKey, Alloc>& hs2)
{
    hs1.swap(hs2);
}

#endif 
