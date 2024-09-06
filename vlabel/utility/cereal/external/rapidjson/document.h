













#ifndef CEREAL_RAPIDJSON_DOCUMENT_H_
#define CEREAL_RAPIDJSON_DOCUMENT_H_



#include "reader.h"
#include "internal/meta.h"
#include "internal/strfunc.h"
#include "memorystream.h"
#include "encodedstream.h"
#include <new>      
#include <limits>

CEREAL_RAPIDJSON_DIAG_PUSH
#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_OFF(padded)
CEREAL_RAPIDJSON_DIAG_OFF(switch-enum)
CEREAL_RAPIDJSON_DIAG_OFF(c++98-compat)
#elif defined(_MSC_VER)
CEREAL_RAPIDJSON_DIAG_OFF(4127) 
CEREAL_RAPIDJSON_DIAG_OFF(4244) 
#endif

#ifdef __GNUC__
CEREAL_RAPIDJSON_DIAG_OFF(effc++)
#endif 

#ifndef CEREAL_RAPIDJSON_NOMEMBERITERATORCLASS
#include <iterator> 
#endif

#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
#include <utility> 
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN


template <typename Encoding, typename Allocator>
class GenericValue;

template <typename Encoding, typename Allocator, typename StackAllocator>
class GenericDocument;



template <typename Encoding, typename Allocator>
struct GenericMember {
    GenericValue<Encoding, Allocator> name;     
    GenericValue<Encoding, Allocator> value;    

    
    friend inline void swap(GenericMember& a, GenericMember& b) CEREAL_RAPIDJSON_NOEXCEPT {
        a.name.Swap(b.name);
        a.value.Swap(b.value);
    }
};




#ifndef CEREAL_RAPIDJSON_NOMEMBERITERATORCLASS



template <bool Const, typename Encoding, typename Allocator>
class GenericMemberIterator {

    friend class GenericValue<Encoding,Allocator>;
    template <bool, typename, typename> friend class GenericMemberIterator;

    typedef GenericMember<Encoding,Allocator> PlainType;
    typedef typename internal::MaybeAddConst<Const,PlainType>::Type ValueType;

public:
    
    typedef GenericMemberIterator Iterator;
    
    typedef GenericMemberIterator<true,Encoding,Allocator>  ConstIterator;
    
    typedef GenericMemberIterator<false,Encoding,Allocator> NonConstIterator;

    
    
    typedef ValueType      value_type;
    typedef ValueType *    pointer;
    typedef ValueType &    reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::random_access_iterator_tag iterator_category;
    

    
    typedef pointer         Pointer;
    
    typedef reference       Reference;
    
    typedef difference_type DifferenceType;

    
    
    GenericMemberIterator() : ptr_() {}

    
    
    GenericMemberIterator(const NonConstIterator & it) : ptr_(it.ptr_) {}
    Iterator& operator=(const NonConstIterator & it) { ptr_ = it.ptr_; return *this; }

    
    
    Iterator& operator++(){ ++ptr_; return *this; }
    Iterator& operator--(){ --ptr_; return *this; }
    Iterator  operator++(int){ Iterator old(*this); ++ptr_; return old; }
    Iterator  operator--(int){ Iterator old(*this); --ptr_; return old; }
    

    
    
    Iterator operator+(DifferenceType n) const { return Iterator(ptr_+n); }
    Iterator operator-(DifferenceType n) const { return Iterator(ptr_-n); }

    Iterator& operator+=(DifferenceType n) { ptr_+=n; return *this; }
    Iterator& operator-=(DifferenceType n) { ptr_-=n; return *this; }
    

    
    
    bool operator==(ConstIterator that) const { return ptr_ == that.ptr_; }
    bool operator!=(ConstIterator that) const { return ptr_ != that.ptr_; }
    bool operator<=(ConstIterator that) const { return ptr_ <= that.ptr_; }
    bool operator>=(ConstIterator that) const { return ptr_ >= that.ptr_; }
    bool operator< (ConstIterator that) const { return ptr_ < that.ptr_; }
    bool operator> (ConstIterator that) const { return ptr_ > that.ptr_; }
    

    
    
    Reference operator*() const { return *ptr_; }
    Pointer   operator->() const { return ptr_; }
    Reference operator[](DifferenceType n) const { return ptr_[n]; }
    

    
    DifferenceType operator-(ConstIterator that) const { return ptr_-that.ptr_; }

private:
    
    explicit GenericMemberIterator(Pointer p) : ptr_(p) {}

    Pointer ptr_; 
};

#else 



template <bool Const, typename Encoding, typename Allocator>
class GenericMemberIterator;


template <typename Encoding, typename Allocator>
class GenericMemberIterator<false,Encoding,Allocator> {
    
    typedef GenericMember<Encoding,Allocator>* Iterator;
};

template <typename Encoding, typename Allocator>
class GenericMemberIterator<true,Encoding,Allocator> {
    
    typedef const GenericMember<Encoding,Allocator>* Iterator;
};

#endif 






template<typename CharType>
struct GenericStringRef {
    typedef CharType Ch; 

    
#ifndef __clang__ 
    
#endif
    template<SizeType N>
    GenericStringRef(const CharType (&str)[N]) CEREAL_RAPIDJSON_NOEXCEPT
        : s(str), length(N-1) {}

    
#ifndef __clang__ 
    
#endif
    explicit GenericStringRef(const CharType* str)
        : s(str), length(NotNullStrLen(str)) {}

    
#ifndef __clang__ 
    
#endif
    GenericStringRef(const CharType* str, SizeType len)
        : s(CEREAL_RAPIDJSON_LIKELY(str) ? str : emptyString), length(len) { CEREAL_RAPIDJSON_ASSERT(str != 0 || len == 0u); }

    GenericStringRef(const GenericStringRef& rhs) : s(rhs.s), length(rhs.length) {}

    
    operator const Ch *() const { return s; }

    const Ch* const s; 
    const SizeType length; 

private:
    SizeType NotNullStrLen(const CharType* str) {
        CEREAL_RAPIDJSON_ASSERT(str != 0);
        return internal::StrLen(str);
    }

    
    static const Ch emptyString[];

    
    template<SizeType N>
    GenericStringRef(CharType (&str)[N]) ;
    
    GenericStringRef& operator=(const GenericStringRef& rhs) ;
};

template<typename CharType>
const CharType GenericStringRef<CharType>::emptyString[] = { CharType() };



template<typename CharType>
inline GenericStringRef<CharType> StringRef(const CharType* str) {
    return GenericStringRef<CharType>(str);
}



template<typename CharType>
inline GenericStringRef<CharType> StringRef(const CharType* str, size_t length) {
    return GenericStringRef<CharType>(str, SizeType(length));
}

#if CEREAL_RAPIDJSON_HAS_STDSTRING


template<typename CharType>
inline GenericStringRef<CharType> StringRef(const std::basic_string<CharType>& str) {
    return GenericStringRef<CharType>(str.data(), SizeType(str.size()));
}
#endif



namespace internal {

template <typename T, typename Encoding = void, typename Allocator = void>
struct IsGenericValueImpl : FalseType {};


template <typename T> struct IsGenericValueImpl<T, typename Void<typename T::EncodingType>::Type, typename Void<typename T::AllocatorType>::Type>
    : IsBaseOf<GenericValue<typename T::EncodingType, typename T::AllocatorType>, T>::Type {};


template <typename T> struct IsGenericValue : IsGenericValueImpl<T>::Type {};

} 




namespace internal {

template <typename ValueType, typename T>
struct TypeHelper {};

template<typename ValueType>
struct TypeHelper<ValueType, bool> {
    static bool Is(const ValueType& v) { return v.IsBool(); }
    static bool Get(const ValueType& v) { return v.GetBool(); }
    static ValueType& Set(ValueType& v, bool data) { return v.SetBool(data); }
    static ValueType& Set(ValueType& v, bool data, typename ValueType::AllocatorType&) { return v.SetBool(data); }
};

template<typename ValueType>
struct TypeHelper<ValueType, int> {
    static bool Is(const ValueType& v) { return v.IsInt(); }
    static int Get(const ValueType& v) { return v.GetInt(); }
    static ValueType& Set(ValueType& v, int data) { return v.SetInt(data); }
    static ValueType& Set(ValueType& v, int data, typename ValueType::AllocatorType&) { return v.SetInt(data); }
};

template<typename ValueType>
struct TypeHelper<ValueType, unsigned> {
    static bool Is(const ValueType& v) { return v.IsUint(); }
    static unsigned Get(const ValueType& v) { return v.GetUint(); }
    static ValueType& Set(ValueType& v, unsigned data) { return v.SetUint(data); }
    static ValueType& Set(ValueType& v, unsigned data, typename ValueType::AllocatorType&) { return v.SetUint(data); }
};

#ifdef _MSC_VER
CEREAL_RAPIDJSON_STATIC_ASSERT(sizeof(long) == sizeof(int));
template<typename ValueType>
struct TypeHelper<ValueType, long> {
    static bool Is(const ValueType& v) { return v.IsInt(); }
    static long Get(const ValueType& v) { return v.GetInt(); }
    static ValueType& Set(ValueType& v, long data) { return v.SetInt(data); }
    static ValueType& Set(ValueType& v, long data, typename ValueType::AllocatorType&) { return v.SetInt(data); }
};

CEREAL_RAPIDJSON_STATIC_ASSERT(sizeof(unsigned long) == sizeof(unsigned));
template<typename ValueType>
struct TypeHelper<ValueType, unsigned long> {
    static bool Is(const ValueType& v) { return v.IsUint(); }
    static unsigned long Get(const ValueType& v) { return v.GetUint(); }
    static ValueType& Set(ValueType& v, unsigned long data) { return v.SetUint(data); }
    static ValueType& Set(ValueType& v, unsigned long data, typename ValueType::AllocatorType&) { return v.SetUint(data); }
};
#endif

template<typename ValueType>
struct TypeHelper<ValueType, int64_t> {
    static bool Is(const ValueType& v) { return v.IsInt64(); }
    static int64_t Get(const ValueType& v) { return v.GetInt64(); }
    static ValueType& Set(ValueType& v, int64_t data) { return v.SetInt64(data); }
    static ValueType& Set(ValueType& v, int64_t data, typename ValueType::AllocatorType&) { return v.SetInt64(data); }
};

template<typename ValueType>
struct TypeHelper<ValueType, uint64_t> {
    static bool Is(const ValueType& v) { return v.IsUint64(); }
    static uint64_t Get(const ValueType& v) { return v.GetUint64(); }
    static ValueType& Set(ValueType& v, uint64_t data) { return v.SetUint64(data); }
    static ValueType& Set(ValueType& v, uint64_t data, typename ValueType::AllocatorType&) { return v.SetUint64(data); }
};

template<typename ValueType>
struct TypeHelper<ValueType, double> {
    static bool Is(const ValueType& v) { return v.IsDouble(); }
    static double Get(const ValueType& v) { return v.GetDouble(); }
    static ValueType& Set(ValueType& v, double data) { return v.SetDouble(data); }
    static ValueType& Set(ValueType& v, double data, typename ValueType::AllocatorType&) { return v.SetDouble(data); }
};

template<typename ValueType>
struct TypeHelper<ValueType, float> {
    static bool Is(const ValueType& v) { return v.IsFloat(); }
    static float Get(const ValueType& v) { return v.GetFloat(); }
    static ValueType& Set(ValueType& v, float data) { return v.SetFloat(data); }
    static ValueType& Set(ValueType& v, float data, typename ValueType::AllocatorType&) { return v.SetFloat(data); }
};

template<typename ValueType>
struct TypeHelper<ValueType, const typename ValueType::Ch*> {
    typedef const typename ValueType::Ch* StringType;
    static bool Is(const ValueType& v) { return v.IsString(); }
    static StringType Get(const ValueType& v) { return v.GetString(); }
    static ValueType& Set(ValueType& v, const StringType data) { return v.SetString(typename ValueType::StringRefType(data)); }
    static ValueType& Set(ValueType& v, const StringType data, typename ValueType::AllocatorType& a) { return v.SetString(data, a); }
};

#if CEREAL_RAPIDJSON_HAS_STDSTRING
template<typename ValueType>
struct TypeHelper<ValueType, std::basic_string<typename ValueType::Ch> > {
    typedef std::basic_string<typename ValueType::Ch> StringType;
    static bool Is(const ValueType& v) { return v.IsString(); }
    static StringType Get(const ValueType& v) { return StringType(v.GetString(), v.GetStringLength()); }
    static ValueType& Set(ValueType& v, const StringType& data, typename ValueType::AllocatorType& a) { return v.SetString(data, a); }
};
#endif

template<typename ValueType>
struct TypeHelper<ValueType, typename ValueType::Array> {
    typedef typename ValueType::Array ArrayType;
    static bool Is(const ValueType& v) { return v.IsArray(); }
    static ArrayType Get(ValueType& v) { return v.GetArray(); }
    static ValueType& Set(ValueType& v, ArrayType data) { return v = data; }
    static ValueType& Set(ValueType& v, ArrayType data, typename ValueType::AllocatorType&) { return v = data; }
};

template<typename ValueType>
struct TypeHelper<ValueType, typename ValueType::ConstArray> {
    typedef typename ValueType::ConstArray ArrayType;
    static bool Is(const ValueType& v) { return v.IsArray(); }
    static ArrayType Get(const ValueType& v) { return v.GetArray(); }
};

template<typename ValueType>
struct TypeHelper<ValueType, typename ValueType::Object> {
    typedef typename ValueType::Object ObjectType;
    static bool Is(const ValueType& v) { return v.IsObject(); }
    static ObjectType Get(ValueType& v) { return v.GetObject(); }
    static ValueType& Set(ValueType& v, ObjectType data) { return v = data; }
    static ValueType& Set(ValueType& v, ObjectType data, typename ValueType::AllocatorType&) { return v = data; }
};

template<typename ValueType>
struct TypeHelper<ValueType, typename ValueType::ConstObject> {
    typedef typename ValueType::ConstObject ObjectType;
    static bool Is(const ValueType& v) { return v.IsObject(); }
    static ObjectType Get(const ValueType& v) { return v.GetObject(); }
};

} 


template <bool, typename> class GenericArray;
template <bool, typename> class GenericObject;






template <typename Encoding, typename Allocator = MemoryPoolAllocator<> >
class GenericValue {
public:
    
    typedef GenericMember<Encoding, Allocator> Member;
    typedef Encoding EncodingType;                  
    typedef Allocator AllocatorType;                
    typedef typename Encoding::Ch Ch;               
    typedef GenericStringRef<Ch> StringRefType;     
    typedef typename GenericMemberIterator<false,Encoding,Allocator>::Iterator MemberIterator;  
    typedef typename GenericMemberIterator<true,Encoding,Allocator>::Iterator ConstMemberIterator;  
    typedef GenericValue* ValueIterator;            
    typedef const GenericValue* ConstValueIterator; 
    typedef GenericValue<Encoding, Allocator> ValueType;    
    typedef GenericArray<false, ValueType> Array;
    typedef GenericArray<true, ValueType> ConstArray;
    typedef GenericObject<false, ValueType> Object;
    typedef GenericObject<true, ValueType> ConstObject;

    
    

    
    GenericValue() CEREAL_RAPIDJSON_NOEXCEPT : data_() { data_.f.flags = kNullFlag; }

#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    
    GenericValue(GenericValue&& rhs) CEREAL_RAPIDJSON_NOEXCEPT : data_(rhs.data_) {
        rhs.data_.f.flags = kNullFlag; 
    }
#endif

private:
    
    GenericValue(const GenericValue& rhs);

#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    
    template <typename StackAllocator>
    GenericValue(GenericDocument<Encoding,Allocator,StackAllocator>&& rhs);

    
    template <typename StackAllocator>
    GenericValue& operator=(GenericDocument<Encoding,Allocator,StackAllocator>&& rhs);
#endif

public:

    
    
    explicit GenericValue(Type type) CEREAL_RAPIDJSON_NOEXCEPT : data_() {
        static const uint16_t defaultFlags[] = {
            kNullFlag, kFalseFlag, kTrueFlag, kObjectFlag, kArrayFlag, kShortStringFlag,
            kNumberAnyFlag
        };
        CEREAL_RAPIDJSON_NOEXCEPT_ASSERT(type >= kNullType && type <= kNumberType);
        data_.f.flags = defaultFlags[type];

        
        if (type == kStringType)
            data_.ss.SetLength(0);
    }

    
    
    template <typename SourceAllocator>
    GenericValue(const GenericValue<Encoding,SourceAllocator>& rhs, Allocator& allocator, bool copyConstStrings = false) {
        switch (rhs.GetType()) {
        case kObjectType: {
                SizeType count = rhs.data_.o.size;
                Member* lm = reinterpret_cast<Member*>(allocator.Malloc(count * sizeof(Member)));
                const typename GenericValue<Encoding,SourceAllocator>::Member* rm = rhs.GetMembersPointer();
                for (SizeType i = 0; i < count; i++) {
                    new (&lm[i].name) GenericValue(rm[i].name, allocator, copyConstStrings);
                    new (&lm[i].value) GenericValue(rm[i].value, allocator, copyConstStrings);
                }
                data_.f.flags = kObjectFlag;
                data_.o.size = data_.o.capacity = count;
                SetMembersPointer(lm);
            }
            break;
        case kArrayType: {
                SizeType count = rhs.data_.a.size;
                GenericValue* le = reinterpret_cast<GenericValue*>(allocator.Malloc(count * sizeof(GenericValue)));
                const GenericValue<Encoding,SourceAllocator>* re = rhs.GetElementsPointer();
                for (SizeType i = 0; i < count; i++)
                    new (&le[i]) GenericValue(re[i], allocator, copyConstStrings);
                data_.f.flags = kArrayFlag;
                data_.a.size = data_.a.capacity = count;
                SetElementsPointer(le);
            }
            break;
        case kStringType:
            if (rhs.data_.f.flags == kConstStringFlag && !copyConstStrings) {
                data_.f.flags = rhs.data_.f.flags;
                data_  = *reinterpret_cast<const Data*>(&rhs.data_);
            }
            else
                SetStringRaw(StringRef(rhs.GetString(), rhs.GetStringLength()), allocator);
            break;
        default:
            data_.f.flags = rhs.data_.f.flags;
            data_  = *reinterpret_cast<const Data*>(&rhs.data_);
            break;
        }
    }

    
    
#ifndef CEREAL_RAPIDJSON_DOXYGEN_RUNNING 
    template <typename T>
    explicit GenericValue(T b, CEREAL_RAPIDJSON_ENABLEIF((internal::IsSame<bool, T>))) CEREAL_RAPIDJSON_NOEXCEPT  
#else
    explicit GenericValue(bool b) CEREAL_RAPIDJSON_NOEXCEPT
#endif
        : data_() {
            
            CEREAL_RAPIDJSON_STATIC_ASSERT((internal::IsSame<bool,T>::Value));
            data_.f.flags = b ? kTrueFlag : kFalseFlag;
    }

    
    explicit GenericValue(int i) CEREAL_RAPIDJSON_NOEXCEPT : data_() {
        data_.n.i64 = i;
        data_.f.flags = (i >= 0) ? (kNumberIntFlag | kUintFlag | kUint64Flag) : kNumberIntFlag;
    }

    
    explicit GenericValue(unsigned u) CEREAL_RAPIDJSON_NOEXCEPT : data_() {
        data_.n.u64 = u;
        data_.f.flags = (u & 0x80000000) ? kNumberUintFlag : (kNumberUintFlag | kIntFlag | kInt64Flag);
    }

    
    explicit GenericValue(int64_t i64) CEREAL_RAPIDJSON_NOEXCEPT : data_() {
        data_.n.i64 = i64;
        data_.f.flags = kNumberInt64Flag;
        if (i64 >= 0) {
            data_.f.flags |= kNumberUint64Flag;
            if (!(static_cast<uint64_t>(i64) & CEREAL_RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x00000000)))
                data_.f.flags |= kUintFlag;
            if (!(static_cast<uint64_t>(i64) & CEREAL_RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x80000000)))
                data_.f.flags |= kIntFlag;
        }
        else if (i64 >= static_cast<int64_t>(CEREAL_RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x80000000)))
            data_.f.flags |= kIntFlag;
    }

    
    explicit GenericValue(uint64_t u64) CEREAL_RAPIDJSON_NOEXCEPT : data_() {
        data_.n.u64 = u64;
        data_.f.flags = kNumberUint64Flag;
        if (!(u64 & CEREAL_RAPIDJSON_UINT64_C2(0x80000000, 0x00000000)))
            data_.f.flags |= kInt64Flag;
        if (!(u64 & CEREAL_RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x00000000)))
            data_.f.flags |= kUintFlag;
        if (!(u64 & CEREAL_RAPIDJSON_UINT64_C2(0xFFFFFFFF, 0x80000000)))
            data_.f.flags |= kIntFlag;
    }

    
    explicit GenericValue(double d) CEREAL_RAPIDJSON_NOEXCEPT : data_() { data_.n.d = d; data_.f.flags = kNumberDoubleFlag; }

    
    explicit GenericValue(float f) CEREAL_RAPIDJSON_NOEXCEPT : data_() { data_.n.d = static_cast<double>(f); data_.f.flags = kNumberDoubleFlag; }

    
    GenericValue(const Ch* s, SizeType length) CEREAL_RAPIDJSON_NOEXCEPT : data_() { SetStringRaw(StringRef(s, length)); }

    
    explicit GenericValue(StringRefType s) CEREAL_RAPIDJSON_NOEXCEPT : data_() { SetStringRaw(s); }

    
    GenericValue(const Ch* s, SizeType length, Allocator& allocator) : data_() { SetStringRaw(StringRef(s, length), allocator); }

    
    GenericValue(const Ch*s, Allocator& allocator) : data_() { SetStringRaw(StringRef(s), allocator); }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    
    GenericValue(const std::basic_string<Ch>& s, Allocator& allocator) : data_() { SetStringRaw(StringRef(s), allocator); }
#endif

    
    
    GenericValue(Array a) CEREAL_RAPIDJSON_NOEXCEPT : data_(a.value_.data_) {
        a.value_.data_ = Data();
        a.value_.data_.f.flags = kArrayFlag;
    }

    
    
    GenericValue(Object o) CEREAL_RAPIDJSON_NOEXCEPT : data_(o.value_.data_) {
        o.value_.data_ = Data();
        o.value_.data_.f.flags = kObjectFlag;
    }

    
    
    ~GenericValue() {
        if (Allocator::kNeedFree) { 
            switch(data_.f.flags) {
            case kArrayFlag:
                {
                    GenericValue* e = GetElementsPointer();
                    for (GenericValue* v = e; v != e + data_.a.size; ++v)
                        v->~GenericValue();
                    Allocator::Free(e);
                }
                break;

            case kObjectFlag:
                for (MemberIterator m = MemberBegin(); m != MemberEnd(); ++m)
                    m->~Member();
                Allocator::Free(GetMembersPointer());
                break;

            case kCopyStringFlag:
                Allocator::Free(const_cast<Ch*>(GetStringPointer()));
                break;

            default:
                break;  
            }
        }
    }

    

    
    

    
    
    GenericValue& operator=(GenericValue& rhs) CEREAL_RAPIDJSON_NOEXCEPT {
        if (CEREAL_RAPIDJSON_LIKELY(this != &rhs)) {
            this->~GenericValue();
            RawAssign(rhs);
        }
        return *this;
    }

#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    
    GenericValue& operator=(GenericValue&& rhs) CEREAL_RAPIDJSON_NOEXCEPT {
        return *this = rhs.Move();
    }
#endif

    
    
    GenericValue& operator=(StringRefType str) CEREAL_RAPIDJSON_NOEXCEPT {
        GenericValue s(str);
        return *this = s;
    }

    
    
    template <typename T>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::IsPointer<T>), (GenericValue&))
    operator=(T value) {
        GenericValue v(value);
        return *this = v;
    }

    
    
    template <typename SourceAllocator>
    GenericValue& CopyFrom(const GenericValue<Encoding, SourceAllocator>& rhs, Allocator& allocator, bool copyConstStrings = false) {
        CEREAL_RAPIDJSON_ASSERT(static_cast<void*>(this) != static_cast<void const*>(&rhs));
        this->~GenericValue();
        new (this) GenericValue(rhs, allocator, copyConstStrings);
        return *this;
    }

    
    
    GenericValue& Swap(GenericValue& other) CEREAL_RAPIDJSON_NOEXCEPT {
        GenericValue temp;
        temp.RawAssign(*this);
        RawAssign(other);
        other.RawAssign(temp);
        return *this;
    }

    
    
    friend inline void swap(GenericValue& a, GenericValue& b) CEREAL_RAPIDJSON_NOEXCEPT { a.Swap(b); }

    
    
    GenericValue& Move() CEREAL_RAPIDJSON_NOEXCEPT { return *this; }
    

    
    
    
    
    template <typename SourceAllocator>
    bool operator==(const GenericValue<Encoding, SourceAllocator>& rhs) const {
        typedef GenericValue<Encoding, SourceAllocator> RhsType;
        if (GetType() != rhs.GetType())
            return false;

        switch (GetType()) {
        case kObjectType: 
            if (data_.o.size != rhs.data_.o.size)
                return false;
            for (ConstMemberIterator lhsMemberItr = MemberBegin(); lhsMemberItr != MemberEnd(); ++lhsMemberItr) {
                typename RhsType::ConstMemberIterator rhsMemberItr = rhs.FindMember(lhsMemberItr->name);
                if (rhsMemberItr == rhs.MemberEnd() || lhsMemberItr->value != rhsMemberItr->value)
                    return false;
            }
            return true;

        case kArrayType:
            if (data_.a.size != rhs.data_.a.size)
                return false;
            for (SizeType i = 0; i < data_.a.size; i++)
                if ((*this)[i] != rhs[i])
                    return false;
            return true;

        case kStringType:
            return StringEqual(rhs);

        case kNumberType:
            if (IsDouble() || rhs.IsDouble()) {
                double a = GetDouble();     
                double b = rhs.GetDouble(); 
                return a >= b && a <= b;    
            }
            else
                return data_.n.u64 == rhs.data_.n.u64;

        default:
            return true;
        }
    }

    
    bool operator==(const Ch* rhs) const { return *this == GenericValue(StringRef(rhs)); }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    
    bool operator==(const std::basic_string<Ch>& rhs) const { return *this == GenericValue(StringRef(rhs)); }
#endif

    
    
    template <typename T> CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>,internal::IsGenericValue<T> >), (bool)) operator==(const T& rhs) const { return *this == GenericValue(rhs); }

    
    
    template <typename SourceAllocator>
    bool operator!=(const GenericValue<Encoding, SourceAllocator>& rhs) const { return !(*this == rhs); }

    
    bool operator!=(const Ch* rhs) const { return !(*this == rhs); }

    
    
    template <typename T> CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::IsGenericValue<T>), (bool)) operator!=(const T& rhs) const { return !(*this == rhs); }

    
    
    template <typename T> friend CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::IsGenericValue<T>), (bool)) operator==(const T& lhs, const GenericValue& rhs) { return rhs == lhs; }

    
    
    template <typename T> friend CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::IsGenericValue<T>), (bool)) operator!=(const T& lhs, const GenericValue& rhs) { return !(rhs == lhs); }
    

    
    

    Type GetType()  const { return static_cast<Type>(data_.f.flags & kTypeMask); }
    bool IsNull()   const { return data_.f.flags == kNullFlag; }
    bool IsFalse()  const { return data_.f.flags == kFalseFlag; }
    bool IsTrue()   const { return data_.f.flags == kTrueFlag; }
    bool IsBool()   const { return (data_.f.flags & kBoolFlag) != 0; }
    bool IsObject() const { return data_.f.flags == kObjectFlag; }
    bool IsArray()  const { return data_.f.flags == kArrayFlag; }
    bool IsNumber() const { return (data_.f.flags & kNumberFlag) != 0; }
    bool IsInt()    const { return (data_.f.flags & kIntFlag) != 0; }
    bool IsUint()   const { return (data_.f.flags & kUintFlag) != 0; }
    bool IsInt64()  const { return (data_.f.flags & kInt64Flag) != 0; }
    bool IsUint64() const { return (data_.f.flags & kUint64Flag) != 0; }
    bool IsDouble() const { return (data_.f.flags & kDoubleFlag) != 0; }
    bool IsString() const { return (data_.f.flags & kStringFlag) != 0; }

    
    bool IsLosslessDouble() const {
        if (!IsNumber()) return false;
        if (IsUint64()) {
            uint64_t u = GetUint64();
            volatile double d = static_cast<double>(u);
            return (d >= 0.0)
                && (d < static_cast<double>((std::numeric_limits<uint64_t>::max)()))
                && (u == static_cast<uint64_t>(d));
        }
        if (IsInt64()) {
            int64_t i = GetInt64();
            volatile double d = static_cast<double>(i);
            return (d >= static_cast<double>((std::numeric_limits<int64_t>::min)()))
                && (d < static_cast<double>((std::numeric_limits<int64_t>::max)()))
                && (i == static_cast<int64_t>(d));
        }
        return true; 
    }

    
    bool IsFloat() const  {
        if ((data_.f.flags & kDoubleFlag) == 0)
            return false;
        double d = GetDouble();
        return d >= -3.4028234e38 && d <= 3.4028234e38;
    }
    
    bool IsLosslessFloat() const {
        if (!IsNumber()) return false;
        double a = GetDouble();
        if (a < static_cast<double>(-(std::numeric_limits<float>::max)())
                || a > static_cast<double>((std::numeric_limits<float>::max)()))
            return false;
        double b = static_cast<double>(static_cast<float>(a));
        return a >= b && a <= b;    
    }

    

    
    

    GenericValue& SetNull() { this->~GenericValue(); new (this) GenericValue(); return *this; }

    

    
    

    bool GetBool() const { CEREAL_RAPIDJSON_ASSERT(IsBool()); return data_.f.flags == kTrueFlag; }
    
    
    GenericValue& SetBool(bool b) { this->~GenericValue(); new (this) GenericValue(b); return *this; }

    

    
    

    
    
    GenericValue& SetObject() { this->~GenericValue(); new (this) GenericValue(kObjectType); return *this; }

    
    SizeType MemberCount() const { CEREAL_RAPIDJSON_ASSERT(IsObject()); return data_.o.size; }

    
    SizeType MemberCapacity() const { CEREAL_RAPIDJSON_ASSERT(IsObject()); return data_.o.capacity; }

    
    bool ObjectEmpty() const { CEREAL_RAPIDJSON_ASSERT(IsObject()); return data_.o.size == 0; }

    
    
    template <typename T>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::NotExpr<internal::IsSame<typename internal::RemoveConst<T>::Type, Ch> >),(GenericValue&)) operator[](T* name) {
        GenericValue n(StringRef(name));
        return (*this)[n];
    }
    template <typename T>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::NotExpr<internal::IsSame<typename internal::RemoveConst<T>::Type, Ch> >),(const GenericValue&)) operator[](T* name) const { return const_cast<GenericValue&>(*this)[name]; }

    
    
    template <typename SourceAllocator>
    GenericValue& operator[](const GenericValue<Encoding, SourceAllocator>& name) {
        MemberIterator member = FindMember(name);
        if (member != MemberEnd())
            return member->value;
        else {
            CEREAL_RAPIDJSON_ASSERT(false);    

            
            
            

            
            static char buffer[sizeof(GenericValue)];
            return *new (buffer) GenericValue();
        }
    }
    template <typename SourceAllocator>
    const GenericValue& operator[](const GenericValue<Encoding, SourceAllocator>& name) const { return const_cast<GenericValue&>(*this)[name]; }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    GenericValue& operator[](const std::basic_string<Ch>& name) { return (*this)[GenericValue(StringRef(name))]; }
    const GenericValue& operator[](const std::basic_string<Ch>& name) const { return (*this)[GenericValue(StringRef(name))]; }
#endif

    
    
    ConstMemberIterator MemberBegin() const { CEREAL_RAPIDJSON_ASSERT(IsObject()); return ConstMemberIterator(GetMembersPointer()); }
    
    
    ConstMemberIterator MemberEnd() const   { CEREAL_RAPIDJSON_ASSERT(IsObject()); return ConstMemberIterator(GetMembersPointer() + data_.o.size); }
    
    
    MemberIterator MemberBegin()            { CEREAL_RAPIDJSON_ASSERT(IsObject()); return MemberIterator(GetMembersPointer()); }
    
    
    MemberIterator MemberEnd()              { CEREAL_RAPIDJSON_ASSERT(IsObject()); return MemberIterator(GetMembersPointer() + data_.o.size); }

    
    
    GenericValue& MemberReserve(SizeType newCapacity, Allocator &allocator) {
        CEREAL_RAPIDJSON_ASSERT(IsObject());
        if (newCapacity > data_.o.capacity) {
            SetMembersPointer(reinterpret_cast<Member*>(allocator.Realloc(GetMembersPointer(), data_.o.capacity * sizeof(Member), newCapacity * sizeof(Member))));
            data_.o.capacity = newCapacity;
        }
        return *this;
    }

    
    
    bool HasMember(const Ch* name) const { return FindMember(name) != MemberEnd(); }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    
    bool HasMember(const std::basic_string<Ch>& name) const { return FindMember(name) != MemberEnd(); }
#endif

    
    
    template <typename SourceAllocator>
    bool HasMember(const GenericValue<Encoding, SourceAllocator>& name) const { return FindMember(name) != MemberEnd(); }

    
    
    MemberIterator FindMember(const Ch* name) {
        GenericValue n(StringRef(name));
        return FindMember(n);
    }

    ConstMemberIterator FindMember(const Ch* name) const { return const_cast<GenericValue&>(*this).FindMember(name); }

    
    
    template <typename SourceAllocator>
    MemberIterator FindMember(const GenericValue<Encoding, SourceAllocator>& name) {
        CEREAL_RAPIDJSON_ASSERT(IsObject());
        CEREAL_RAPIDJSON_ASSERT(name.IsString());
        MemberIterator member = MemberBegin();
        for ( ; member != MemberEnd(); ++member)
            if (name.StringEqual(member->name))
                break;
        return member;
    }
    template <typename SourceAllocator> ConstMemberIterator FindMember(const GenericValue<Encoding, SourceAllocator>& name) const { return const_cast<GenericValue&>(*this).FindMember(name); }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    
    MemberIterator FindMember(const std::basic_string<Ch>& name) { return FindMember(GenericValue(StringRef(name))); }
    ConstMemberIterator FindMember(const std::basic_string<Ch>& name) const { return FindMember(GenericValue(StringRef(name))); }
#endif

    
    
    GenericValue& AddMember(GenericValue& name, GenericValue& value, Allocator& allocator) {
        CEREAL_RAPIDJSON_ASSERT(IsObject());
        CEREAL_RAPIDJSON_ASSERT(name.IsString());

        ObjectData& o = data_.o;
        if (o.size >= o.capacity)
            MemberReserve(o.capacity == 0 ? kDefaultObjectCapacity : (o.capacity + (o.capacity + 1) / 2), allocator);
        Member* members = GetMembersPointer();
        members[o.size].name.RawAssign(name);
        members[o.size].value.RawAssign(value);
        o.size++;
        return *this;
    }

    
    
    GenericValue& AddMember(GenericValue& name, StringRefType value, Allocator& allocator) {
        GenericValue v(value);
        return AddMember(name, v, allocator);
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    
    GenericValue& AddMember(GenericValue& name, std::basic_string<Ch>& value, Allocator& allocator) {
        GenericValue v(value, allocator);
        return AddMember(name, v, allocator);
    }
#endif

    
    
    template <typename T>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericValue&))
    AddMember(GenericValue& name, T value, Allocator& allocator) {
        GenericValue v(value);
        return AddMember(name, v, allocator);
    }

#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericValue& AddMember(GenericValue&& name, GenericValue&& value, Allocator& allocator) {
        return AddMember(name, value, allocator);
    }
    GenericValue& AddMember(GenericValue&& name, GenericValue& value, Allocator& allocator) {
        return AddMember(name, value, allocator);
    }
    GenericValue& AddMember(GenericValue& name, GenericValue&& value, Allocator& allocator) {
        return AddMember(name, value, allocator);
    }
    GenericValue& AddMember(StringRefType name, GenericValue&& value, Allocator& allocator) {
        GenericValue n(name);
        return AddMember(n, value, allocator);
    }
#endif 


    
    
    GenericValue& AddMember(StringRefType name, GenericValue& value, Allocator& allocator) {
        GenericValue n(name);
        return AddMember(n, value, allocator);
    }

    
    
    GenericValue& AddMember(StringRefType name, StringRefType value, Allocator& allocator) {
        GenericValue v(value);
        return AddMember(name, v, allocator);
    }

    
    
    template <typename T>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericValue&))
    AddMember(StringRefType name, T value, Allocator& allocator) {
        GenericValue n(name);
        return AddMember(n, value, allocator);
    }

    
    
    void RemoveAllMembers() {
        CEREAL_RAPIDJSON_ASSERT(IsObject());
        for (MemberIterator m = MemberBegin(); m != MemberEnd(); ++m)
            m->~Member();
        data_.o.size = 0;
    }

    
    
    bool RemoveMember(const Ch* name) {
        GenericValue n(StringRef(name));
        return RemoveMember(n);
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    bool RemoveMember(const std::basic_string<Ch>& name) { return RemoveMember(GenericValue(StringRef(name))); }
#endif

    template <typename SourceAllocator>
    bool RemoveMember(const GenericValue<Encoding, SourceAllocator>& name) {
        MemberIterator m = FindMember(name);
        if (m != MemberEnd()) {
            RemoveMember(m);
            return true;
        }
        else
            return false;
    }

    
    
    MemberIterator RemoveMember(MemberIterator m) {
        CEREAL_RAPIDJSON_ASSERT(IsObject());
        CEREAL_RAPIDJSON_ASSERT(data_.o.size > 0);
        CEREAL_RAPIDJSON_ASSERT(GetMembersPointer() != 0);
        CEREAL_RAPIDJSON_ASSERT(m >= MemberBegin() && m < MemberEnd());

        MemberIterator last(GetMembersPointer() + (data_.o.size - 1));
        if (data_.o.size > 1 && m != last)
            *m = *last; 
        else
            m->~Member(); 
        --data_.o.size;
        return m;
    }

    
    
    MemberIterator EraseMember(ConstMemberIterator pos) {
        return EraseMember(pos, pos +1);
    }

    
    
    MemberIterator EraseMember(ConstMemberIterator first, ConstMemberIterator last) {
        CEREAL_RAPIDJSON_ASSERT(IsObject());
        CEREAL_RAPIDJSON_ASSERT(data_.o.size > 0);
        CEREAL_RAPIDJSON_ASSERT(GetMembersPointer() != 0);
        CEREAL_RAPIDJSON_ASSERT(first >= MemberBegin());
        CEREAL_RAPIDJSON_ASSERT(first <= last);
        CEREAL_RAPIDJSON_ASSERT(last <= MemberEnd());

        MemberIterator pos = MemberBegin() + (first - MemberBegin());
        for (MemberIterator itr = pos; itr != last; ++itr)
            itr->~Member();
        std::memmove(static_cast<void*>(&*pos), &*last, static_cast<size_t>(MemberEnd() - last) * sizeof(Member));
        data_.o.size -= static_cast<SizeType>(last - first);
        return pos;
    }

    
    
    bool EraseMember(const Ch* name) {
        GenericValue n(StringRef(name));
        return EraseMember(n);
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    bool EraseMember(const std::basic_string<Ch>& name) { return EraseMember(GenericValue(StringRef(name))); }
#endif

    template <typename SourceAllocator>
    bool EraseMember(const GenericValue<Encoding, SourceAllocator>& name) {
        MemberIterator m = FindMember(name);
        if (m != MemberEnd()) {
            EraseMember(m);
            return true;
        }
        else
            return false;
    }

    Object GetObject() { CEREAL_RAPIDJSON_ASSERT(IsObject()); return Object(*this); }
    ConstObject GetObject() const { CEREAL_RAPIDJSON_ASSERT(IsObject()); return ConstObject(*this); }

    

    
    

    
    
    GenericValue& SetArray() { this->~GenericValue(); new (this) GenericValue(kArrayType); return *this; }

    
    SizeType Size() const { CEREAL_RAPIDJSON_ASSERT(IsArray()); return data_.a.size; }

    
    SizeType Capacity() const { CEREAL_RAPIDJSON_ASSERT(IsArray()); return data_.a.capacity; }

    
    bool Empty() const { CEREAL_RAPIDJSON_ASSERT(IsArray()); return data_.a.size == 0; }

    
    
    void Clear() {
        CEREAL_RAPIDJSON_ASSERT(IsArray());
        GenericValue* e = GetElementsPointer();
        for (GenericValue* v = e; v != e + data_.a.size; ++v)
            v->~GenericValue();
        data_.a.size = 0;
    }

    
    
    GenericValue& operator[](SizeType index) {
        CEREAL_RAPIDJSON_ASSERT(IsArray());
        CEREAL_RAPIDJSON_ASSERT(index < data_.a.size);
        return GetElementsPointer()[index];
    }
    const GenericValue& operator[](SizeType index) const { return const_cast<GenericValue&>(*this)[index]; }

    
    
    ValueIterator Begin() { CEREAL_RAPIDJSON_ASSERT(IsArray()); return GetElementsPointer(); }
    
    
    ValueIterator End() { CEREAL_RAPIDJSON_ASSERT(IsArray()); return GetElementsPointer() + data_.a.size; }
    
    
    ConstValueIterator Begin() const { return const_cast<GenericValue&>(*this).Begin(); }
    
    
    ConstValueIterator End() const { return const_cast<GenericValue&>(*this).End(); }

    
    
    GenericValue& Reserve(SizeType newCapacity, Allocator &allocator) {
        CEREAL_RAPIDJSON_ASSERT(IsArray());
        if (newCapacity > data_.a.capacity) {
            SetElementsPointer(reinterpret_cast<GenericValue*>(allocator.Realloc(GetElementsPointer(), data_.a.capacity * sizeof(GenericValue), newCapacity * sizeof(GenericValue))));
            data_.a.capacity = newCapacity;
        }
        return *this;
    }

    
    
    GenericValue& PushBack(GenericValue& value, Allocator& allocator) {
        CEREAL_RAPIDJSON_ASSERT(IsArray());
        if (data_.a.size >= data_.a.capacity)
            Reserve(data_.a.capacity == 0 ? kDefaultArrayCapacity : (data_.a.capacity + (data_.a.capacity + 1) / 2), allocator);
        GetElementsPointer()[data_.a.size++].RawAssign(value);
        return *this;
    }

#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericValue& PushBack(GenericValue&& value, Allocator& allocator) {
        return PushBack(value, allocator);
    }
#endif 

    
    
    GenericValue& PushBack(StringRefType value, Allocator& allocator) {
        return (*this).template PushBack<StringRefType>(value, allocator);
    }

    
    
    template <typename T>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericValue&))
    PushBack(T value, Allocator& allocator) {
        GenericValue v(value);
        return PushBack(v, allocator);
    }

    
    
    GenericValue& PopBack() {
        CEREAL_RAPIDJSON_ASSERT(IsArray());
        CEREAL_RAPIDJSON_ASSERT(!Empty());
        GetElementsPointer()[--data_.a.size].~GenericValue();
        return *this;
    }

    
    
    ValueIterator Erase(ConstValueIterator pos) {
        return Erase(pos, pos + 1);
    }

    
    
    ValueIterator Erase(ConstValueIterator first, ConstValueIterator last) {
        CEREAL_RAPIDJSON_ASSERT(IsArray());
        CEREAL_RAPIDJSON_ASSERT(data_.a.size > 0);
        CEREAL_RAPIDJSON_ASSERT(GetElementsPointer() != 0);
        CEREAL_RAPIDJSON_ASSERT(first >= Begin());
        CEREAL_RAPIDJSON_ASSERT(first <= last);
        CEREAL_RAPIDJSON_ASSERT(last <= End());
        ValueIterator pos = Begin() + (first - Begin());
        for (ValueIterator itr = pos; itr != last; ++itr)
            itr->~GenericValue();
        std::memmove(static_cast<void*>(pos), last, static_cast<size_t>(End() - last) * sizeof(GenericValue));
        data_.a.size -= static_cast<SizeType>(last - first);
        return pos;
    }

    Array GetArray() { CEREAL_RAPIDJSON_ASSERT(IsArray()); return Array(*this); }
    ConstArray GetArray() const { CEREAL_RAPIDJSON_ASSERT(IsArray()); return ConstArray(*this); }

    

    
    

    int GetInt() const          { CEREAL_RAPIDJSON_ASSERT(data_.f.flags & kIntFlag);   return data_.n.i.i;   }
    unsigned GetUint() const    { CEREAL_RAPIDJSON_ASSERT(data_.f.flags & kUintFlag);  return data_.n.u.u;   }
    int64_t GetInt64() const    { CEREAL_RAPIDJSON_ASSERT(data_.f.flags & kInt64Flag); return data_.n.i64; }
    uint64_t GetUint64() const  { CEREAL_RAPIDJSON_ASSERT(data_.f.flags & kUint64Flag); return data_.n.u64; }

    
    
    double GetDouble() const {
        CEREAL_RAPIDJSON_ASSERT(IsNumber());
        if ((data_.f.flags & kDoubleFlag) != 0)                return data_.n.d;   
        if ((data_.f.flags & kIntFlag) != 0)                   return data_.n.i.i; 
        if ((data_.f.flags & kUintFlag) != 0)                  return data_.n.u.u; 
        if ((data_.f.flags & kInt64Flag) != 0)                 return static_cast<double>(data_.n.i64); 
        CEREAL_RAPIDJSON_ASSERT((data_.f.flags & kUint64Flag) != 0);  return static_cast<double>(data_.n.u64); 
    }

    
    
    float GetFloat() const {
        return static_cast<float>(GetDouble());
    }

    GenericValue& SetInt(int i)             { this->~GenericValue(); new (this) GenericValue(i);    return *this; }
    GenericValue& SetUint(unsigned u)       { this->~GenericValue(); new (this) GenericValue(u);    return *this; }
    GenericValue& SetInt64(int64_t i64)     { this->~GenericValue(); new (this) GenericValue(i64);  return *this; }
    GenericValue& SetUint64(uint64_t u64)   { this->~GenericValue(); new (this) GenericValue(u64);  return *this; }
    GenericValue& SetDouble(double d)       { this->~GenericValue(); new (this) GenericValue(d);    return *this; }
    GenericValue& SetFloat(float f)         { this->~GenericValue(); new (this) GenericValue(static_cast<double>(f)); return *this; }

    

    
    

    const Ch* GetString() const { CEREAL_RAPIDJSON_ASSERT(IsString()); return (data_.f.flags & kInlineStrFlag) ? data_.ss.str : GetStringPointer(); }

    
    
    SizeType GetStringLength() const { CEREAL_RAPIDJSON_ASSERT(IsString()); return ((data_.f.flags & kInlineStrFlag) ? (data_.ss.GetLength()) : data_.s.length); }

    
    
    GenericValue& SetString(const Ch* s, SizeType length) { return SetString(StringRef(s, length)); }

    
    
    GenericValue& SetString(StringRefType s) { this->~GenericValue(); SetStringRaw(s); return *this; }

    
    
    GenericValue& SetString(const Ch* s, SizeType length, Allocator& allocator) { return SetString(StringRef(s, length), allocator); }

    
    
    GenericValue& SetString(const Ch* s, Allocator& allocator) { return SetString(StringRef(s), allocator); }

    
    
    GenericValue& SetString(StringRefType s, Allocator& allocator) { this->~GenericValue(); SetStringRaw(s, allocator); return *this; }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    
    GenericValue& SetString(const std::basic_string<Ch>& s, Allocator& allocator) { return SetString(StringRef(s), allocator); }
#endif

    

    
    

    
    
    template <typename T>
    bool Is() const { return internal::TypeHelper<ValueType, T>::Is(*this); }

    template <typename T>
    T Get() const { return internal::TypeHelper<ValueType, T>::Get(*this); }

    template <typename T>
    T Get() { return internal::TypeHelper<ValueType, T>::Get(*this); }

    template<typename T>
    ValueType& Set(const T& data) { return internal::TypeHelper<ValueType, T>::Set(*this, data); }

    template<typename T>
    ValueType& Set(const T& data, AllocatorType& allocator) { return internal::TypeHelper<ValueType, T>::Set(*this, data, allocator); }

    

    
    
    template <typename Handler>
    bool Accept(Handler& handler) const {
        switch(GetType()) {
        case kNullType:     return handler.Null();
        case kFalseType:    return handler.Bool(false);
        case kTrueType:     return handler.Bool(true);

        case kObjectType:
            if (CEREAL_RAPIDJSON_UNLIKELY(!handler.StartObject()))
                return false;
            for (ConstMemberIterator m = MemberBegin(); m != MemberEnd(); ++m) {
                CEREAL_RAPIDJSON_ASSERT(m->name.IsString()); 
                if (CEREAL_RAPIDJSON_UNLIKELY(!handler.Key(m->name.GetString(), m->name.GetStringLength(), (m->name.data_.f.flags & kCopyFlag) != 0)))
                    return false;
                if (CEREAL_RAPIDJSON_UNLIKELY(!m->value.Accept(handler)))
                    return false;
            }
            return handler.EndObject(data_.o.size);

        case kArrayType:
            if (CEREAL_RAPIDJSON_UNLIKELY(!handler.StartArray()))
                return false;
            for (const GenericValue* v = Begin(); v != End(); ++v)
                if (CEREAL_RAPIDJSON_UNLIKELY(!v->Accept(handler)))
                    return false;
            return handler.EndArray(data_.a.size);

        case kStringType:
            return handler.String(GetString(), GetStringLength(), (data_.f.flags & kCopyFlag) != 0);

        default:
            CEREAL_RAPIDJSON_ASSERT(GetType() == kNumberType);
            if (IsDouble())         return handler.Double(data_.n.d);
            else if (IsInt())       return handler.Int(data_.n.i.i);
            else if (IsUint())      return handler.Uint(data_.n.u.u);
            else if (IsInt64())     return handler.Int64(data_.n.i64);
            else                    return handler.Uint64(data_.n.u64);
        }
    }

private:
    template <typename, typename> friend class GenericValue;
    template <typename, typename, typename> friend class GenericDocument;

    enum {
        kBoolFlag       = 0x0008,
        kNumberFlag     = 0x0010,
        kIntFlag        = 0x0020,
        kUintFlag       = 0x0040,
        kInt64Flag      = 0x0080,
        kUint64Flag     = 0x0100,
        kDoubleFlag     = 0x0200,
        kStringFlag     = 0x0400,
        kCopyFlag       = 0x0800,
        kInlineStrFlag  = 0x1000,

        
        kNullFlag = kNullType,
        kTrueFlag = kTrueType | kBoolFlag,
        kFalseFlag = kFalseType | kBoolFlag,
        kNumberIntFlag = kNumberType | kNumberFlag | kIntFlag | kInt64Flag,
        kNumberUintFlag = kNumberType | kNumberFlag | kUintFlag | kUint64Flag | kInt64Flag,
        kNumberInt64Flag = kNumberType | kNumberFlag | kInt64Flag,
        kNumberUint64Flag = kNumberType | kNumberFlag | kUint64Flag,
        kNumberDoubleFlag = kNumberType | kNumberFlag | kDoubleFlag,
        kNumberAnyFlag = kNumberType | kNumberFlag | kIntFlag | kInt64Flag | kUintFlag | kUint64Flag | kDoubleFlag,
        kConstStringFlag = kStringType | kStringFlag,
        kCopyStringFlag = kStringType | kStringFlag | kCopyFlag,
        kShortStringFlag = kStringType | kStringFlag | kCopyFlag | kInlineStrFlag,
        kObjectFlag = kObjectType,
        kArrayFlag = kArrayType,

        kTypeMask = 0x07
    };

    static const SizeType kDefaultArrayCapacity = 16;
    static const SizeType kDefaultObjectCapacity = 16;

    struct Flag {
#if CEREAL_RAPIDJSON_48BITPOINTER_OPTIMIZATION
        char payload[sizeof(SizeType) * 2 + 6];     
#elif CEREAL_RAPIDJSON_64BIT
        char payload[sizeof(SizeType) * 2 + sizeof(void*) + 6]; 
#else
        char payload[sizeof(SizeType) * 2 + sizeof(void*) + 2]; 
#endif
        uint16_t flags;
    };

    struct String {
        SizeType length;
        SizeType hashcode;  
        const Ch* str;
    };  

    
    
    
    
    
    
    
    
    struct ShortString {
        enum { MaxChars = sizeof(static_cast<Flag*>(0)->payload) / sizeof(Ch), MaxSize = MaxChars - 1, LenPos = MaxSize };
        Ch str[MaxChars];

        inline static bool Usable(SizeType len) { return                       (MaxSize >= len); }
        inline void     SetLength(SizeType len) { str[LenPos] = static_cast<Ch>(MaxSize -  len); }
        inline SizeType GetLength() const       { return  static_cast<SizeType>(MaxSize -  str[LenPos]); }
    };  

    
    union Number {
#if CEREAL_RAPIDJSON_ENDIAN == CEREAL_RAPIDJSON_LITTLEENDIAN
        struct I {
            int i;
            char padding[4];
        }i;
        struct U {
            unsigned u;
            char padding2[4];
        }u;
#else
        struct I {
            char padding[4];
            int i;
        }i;
        struct U {
            char padding2[4];
            unsigned u;
        }u;
#endif
        int64_t i64;
        uint64_t u64;
        double d;
    };  

    struct ObjectData {
        SizeType size;
        SizeType capacity;
        Member* members;
    };  

    struct ArrayData {
        SizeType size;
        SizeType capacity;
        GenericValue* elements;
    };  

    union Data {
        String s;
        ShortString ss;
        Number n;
        ObjectData o;
        ArrayData a;
        Flag f;
    };  

    CEREAL_RAPIDJSON_FORCEINLINE const Ch* GetStringPointer() const { return CEREAL_RAPIDJSON_GETPOINTER(Ch, data_.s.str); }
    CEREAL_RAPIDJSON_FORCEINLINE const Ch* SetStringPointer(const Ch* str) { return CEREAL_RAPIDJSON_SETPOINTER(Ch, data_.s.str, str); }
    CEREAL_RAPIDJSON_FORCEINLINE GenericValue* GetElementsPointer() const { return CEREAL_RAPIDJSON_GETPOINTER(GenericValue, data_.a.elements); }
    CEREAL_RAPIDJSON_FORCEINLINE GenericValue* SetElementsPointer(GenericValue* elements) { return CEREAL_RAPIDJSON_SETPOINTER(GenericValue, data_.a.elements, elements); }
    CEREAL_RAPIDJSON_FORCEINLINE Member* GetMembersPointer() const { return CEREAL_RAPIDJSON_GETPOINTER(Member, data_.o.members); }
    CEREAL_RAPIDJSON_FORCEINLINE Member* SetMembersPointer(Member* members) { return CEREAL_RAPIDJSON_SETPOINTER(Member, data_.o.members, members); }

    
    void SetArrayRaw(GenericValue* values, SizeType count, Allocator& allocator) {
        data_.f.flags = kArrayFlag;
        if (count) {
            GenericValue* e = static_cast<GenericValue*>(allocator.Malloc(count * sizeof(GenericValue)));
            SetElementsPointer(e);
            std::memcpy(static_cast<void*>(e), values, count * sizeof(GenericValue));
        }
        else
            SetElementsPointer(0);
        data_.a.size = data_.a.capacity = count;
    }

    
    void SetObjectRaw(Member* members, SizeType count, Allocator& allocator) {
        data_.f.flags = kObjectFlag;
        if (count) {
            Member* m = static_cast<Member*>(allocator.Malloc(count * sizeof(Member)));
            SetMembersPointer(m);
            std::memcpy(static_cast<void*>(m), members, count * sizeof(Member));
        }
        else
            SetMembersPointer(0);
        data_.o.size = data_.o.capacity = count;
    }

    
    void SetStringRaw(StringRefType s) CEREAL_RAPIDJSON_NOEXCEPT {
        data_.f.flags = kConstStringFlag;
        SetStringPointer(s);
        data_.s.length = s.length;
    }

    
    void SetStringRaw(StringRefType s, Allocator& allocator) {
        Ch* str = 0;
        if (ShortString::Usable(s.length)) {
            data_.f.flags = kShortStringFlag;
            data_.ss.SetLength(s.length);
            str = data_.ss.str;
        } else {
            data_.f.flags = kCopyStringFlag;
            data_.s.length = s.length;
            str = static_cast<Ch *>(allocator.Malloc((s.length + 1) * sizeof(Ch)));
            SetStringPointer(str);
        }
        std::memcpy(str, s, s.length * sizeof(Ch));
        str[s.length] = '\0';
    }

    
    void RawAssign(GenericValue& rhs) CEREAL_RAPIDJSON_NOEXCEPT {
        data_ = rhs.data_;
        
        rhs.data_.f.flags = kNullFlag;
    }

    template <typename SourceAllocator>
    bool StringEqual(const GenericValue<Encoding, SourceAllocator>& rhs) const {
        CEREAL_RAPIDJSON_ASSERT(IsString());
        CEREAL_RAPIDJSON_ASSERT(rhs.IsString());

        const SizeType len1 = GetStringLength();
        const SizeType len2 = rhs.GetStringLength();
        if(len1 != len2) { return false; }

        const Ch* const str1 = GetString();
        const Ch* const str2 = rhs.GetString();
        if(str1 == str2) { return true; } 

        return (std::memcmp(str1, str2, sizeof(Ch) * len1) == 0);
    }

    Data data_;
};


typedef GenericValue<UTF8<> > Value;






template <typename Encoding, typename Allocator = MemoryPoolAllocator<>, typename StackAllocator = CrtAllocator>
class GenericDocument : public GenericValue<Encoding, Allocator> {
public:
    typedef typename Encoding::Ch Ch;                       
    typedef GenericValue<Encoding, Allocator> ValueType;    
    typedef Allocator AllocatorType;                        

    
    
    explicit GenericDocument(Type type, Allocator* allocator = 0, size_t stackCapacity = kDefaultStackCapacity, StackAllocator* stackAllocator = 0) :
        GenericValue<Encoding, Allocator>(type),  allocator_(allocator), ownAllocator_(0), stack_(stackAllocator, stackCapacity), parseResult_()
    {
        if (!allocator_)
            ownAllocator_ = allocator_ = CEREAL_RAPIDJSON_NEW(Allocator)();
    }

    
    
    GenericDocument(Allocator* allocator = 0, size_t stackCapacity = kDefaultStackCapacity, StackAllocator* stackAllocator = 0) :
        allocator_(allocator), ownAllocator_(0), stack_(stackAllocator, stackCapacity), parseResult_()
    {
        if (!allocator_)
            ownAllocator_ = allocator_ = CEREAL_RAPIDJSON_NEW(Allocator)();
    }

#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    
    GenericDocument(GenericDocument&& rhs) CEREAL_RAPIDJSON_NOEXCEPT
        : ValueType(std::forward<ValueType>(rhs)), 
          allocator_(rhs.allocator_),
          ownAllocator_(rhs.ownAllocator_),
          stack_(std::move(rhs.stack_)),
          parseResult_(rhs.parseResult_)
    {
        rhs.allocator_ = 0;
        rhs.ownAllocator_ = 0;
        rhs.parseResult_ = ParseResult();
    }
#endif

    ~GenericDocument() {
        Destroy();
    }

#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    
    GenericDocument& operator=(GenericDocument&& rhs) CEREAL_RAPIDJSON_NOEXCEPT
    {
        
        
        ValueType::operator=(std::forward<ValueType>(rhs));

        
        Destroy();

        allocator_ = rhs.allocator_;
        ownAllocator_ = rhs.ownAllocator_;
        stack_ = std::move(rhs.stack_);
        parseResult_ = rhs.parseResult_;

        rhs.allocator_ = 0;
        rhs.ownAllocator_ = 0;
        rhs.parseResult_ = ParseResult();

        return *this;
    }
#endif

    
    
    GenericDocument& Swap(GenericDocument& rhs) CEREAL_RAPIDJSON_NOEXCEPT {
        ValueType::Swap(rhs);
        stack_.Swap(rhs.stack_);
        internal::Swap(allocator_, rhs.allocator_);
        internal::Swap(ownAllocator_, rhs.ownAllocator_);
        internal::Swap(parseResult_, rhs.parseResult_);
        return *this;
    }

    
    
    using ValueType::Swap;

    
    
    friend inline void swap(GenericDocument& a, GenericDocument& b) CEREAL_RAPIDJSON_NOEXCEPT { a.Swap(b); }

    
    
    template <typename Generator>
    GenericDocument& Populate(Generator& g) {
        ClearStackOnExit scope(*this);
        if (g(*this)) {
            CEREAL_RAPIDJSON_ASSERT(stack_.GetSize() == sizeof(ValueType)); 
            ValueType::operator=(*stack_.template Pop<ValueType>(1));
        }
        return *this;
    }

    
    

    
    
    template <unsigned parseFlags, typename SourceEncoding, typename InputStream>
    GenericDocument& ParseStream(InputStream& is) {
        GenericReader<SourceEncoding, Encoding, StackAllocator> reader(
            stack_.HasAllocator() ? &stack_.GetAllocator() : 0);
        ClearStackOnExit scope(*this);
        parseResult_ = reader.template Parse<parseFlags>(is, *this);
        if (parseResult_) {
            CEREAL_RAPIDJSON_ASSERT(stack_.GetSize() == sizeof(ValueType)); 
            ValueType::operator=(*stack_.template Pop<ValueType>(1));
        }
        return *this;
    }

    
    
    template <unsigned parseFlags, typename InputStream>
    GenericDocument& ParseStream(InputStream& is) {
        return ParseStream<parseFlags, Encoding, InputStream>(is);
    }

    
    
    template <typename InputStream>
    GenericDocument& ParseStream(InputStream& is) {
        return ParseStream<kParseDefaultFlags, Encoding, InputStream>(is);
    }
    

    
    

    
    
    template <unsigned parseFlags>
    GenericDocument& ParseInsitu(Ch* str) {
        GenericInsituStringStream<Encoding> s(str);
        return ParseStream<parseFlags | kParseInsituFlag>(s);
    }

    
    
    GenericDocument& ParseInsitu(Ch* str) {
        return ParseInsitu<kParseDefaultFlags>(str);
    }
    

    
    

    
    
    template <unsigned parseFlags, typename SourceEncoding>
    GenericDocument& Parse(const typename SourceEncoding::Ch* str) {
        CEREAL_RAPIDJSON_ASSERT(!(parseFlags & kParseInsituFlag));
        GenericStringStream<SourceEncoding> s(str);
        return ParseStream<parseFlags, SourceEncoding>(s);
    }

    
    
    template <unsigned parseFlags>
    GenericDocument& Parse(const Ch* str) {
        return Parse<parseFlags, Encoding>(str);
    }

    
    
    GenericDocument& Parse(const Ch* str) {
        return Parse<kParseDefaultFlags>(str);
    }

    template <unsigned parseFlags, typename SourceEncoding>
    GenericDocument& Parse(const typename SourceEncoding::Ch* str, size_t length) {
        CEREAL_RAPIDJSON_ASSERT(!(parseFlags & kParseInsituFlag));
        MemoryStream ms(reinterpret_cast<const char*>(str), length * sizeof(typename SourceEncoding::Ch));
        EncodedInputStream<SourceEncoding, MemoryStream> is(ms);
        ParseStream<parseFlags, SourceEncoding>(is);
        return *this;
    }

    template <unsigned parseFlags>
    GenericDocument& Parse(const Ch* str, size_t length) {
        return Parse<parseFlags, Encoding>(str, length);
    }

    GenericDocument& Parse(const Ch* str, size_t length) {
        return Parse<kParseDefaultFlags>(str, length);
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    template <unsigned parseFlags, typename SourceEncoding>
    GenericDocument& Parse(const std::basic_string<typename SourceEncoding::Ch>& str) {
        
        return Parse<parseFlags, SourceEncoding>(str.c_str());
    }

    template <unsigned parseFlags>
    GenericDocument& Parse(const std::basic_string<Ch>& str) {
        return Parse<parseFlags, Encoding>(str.c_str());
    }

    GenericDocument& Parse(const std::basic_string<Ch>& str) {
        return Parse<kParseDefaultFlags>(str);
    }
#endif 

    

    
    

    
    bool HasParseError() const { return parseResult_.IsError(); }

    
    ParseErrorCode GetParseError() const { return parseResult_.Code(); }

    
    size_t GetErrorOffset() const { return parseResult_.Offset(); }

    
#ifndef __clang 
    
#endif
    operator ParseResult() const { return parseResult_; }
    

    
    Allocator& GetAllocator() {
        CEREAL_RAPIDJSON_ASSERT(allocator_);
        return *allocator_;
    }

    
    size_t GetStackCapacity() const { return stack_.GetCapacity(); }

private:
    
    struct ClearStackOnExit {
        explicit ClearStackOnExit(GenericDocument& d) : d_(d) {}
        ~ClearStackOnExit() { d_.ClearStack(); }
    private:
        ClearStackOnExit(const ClearStackOnExit&);
        ClearStackOnExit& operator=(const ClearStackOnExit&);
        GenericDocument& d_;
    };

    
    
    template <typename, typename> friend class GenericValue; 

public:
    
    bool Null() { new (stack_.template Push<ValueType>()) ValueType(); return true; }
    bool Bool(bool b) { new (stack_.template Push<ValueType>()) ValueType(b); return true; }
    bool Int(int i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Uint(unsigned i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Int64(int64_t i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Uint64(uint64_t i) { new (stack_.template Push<ValueType>()) ValueType(i); return true; }
    bool Double(double d) { new (stack_.template Push<ValueType>()) ValueType(d); return true; }

    bool RawNumber(const Ch* str, SizeType length, bool copy) {
        if (copy)
            new (stack_.template Push<ValueType>()) ValueType(str, length, GetAllocator());
        else
            new (stack_.template Push<ValueType>()) ValueType(str, length);
        return true;
    }

    bool String(const Ch* str, SizeType length, bool copy) {
        if (copy)
            new (stack_.template Push<ValueType>()) ValueType(str, length, GetAllocator());
        else
            new (stack_.template Push<ValueType>()) ValueType(str, length);
        return true;
    }

    bool StartObject() { new (stack_.template Push<ValueType>()) ValueType(kObjectType); return true; }

    bool Key(const Ch* str, SizeType length, bool copy) { return String(str, length, copy); }

    bool EndObject(SizeType memberCount) {
        typename ValueType::Member* members = stack_.template Pop<typename ValueType::Member>(memberCount);
        stack_.template Top<ValueType>()->SetObjectRaw(members, memberCount, GetAllocator());
        return true;
    }

    bool StartArray() { new (stack_.template Push<ValueType>()) ValueType(kArrayType); return true; }

    bool EndArray(SizeType elementCount) {
        ValueType* elements = stack_.template Pop<ValueType>(elementCount);
        stack_.template Top<ValueType>()->SetArrayRaw(elements, elementCount, GetAllocator());
        return true;
    }

private:
    
    GenericDocument(const GenericDocument&);
    
    GenericDocument& operator=(const GenericDocument&);

    void ClearStack() {
        if (Allocator::kNeedFree)
            while (stack_.GetSize() > 0)    
                (stack_.template Pop<ValueType>(1))->~ValueType();
        else
            stack_.Clear();
        stack_.ShrinkToFit();
    }

    void Destroy() {
        CEREAL_RAPIDJSON_DELETE(ownAllocator_);
    }

    static const size_t kDefaultStackCapacity = 1024;
    Allocator* allocator_;
    Allocator* ownAllocator_;
    internal::Stack<StackAllocator> stack_;
    ParseResult parseResult_;
};


typedef GenericDocument<UTF8<> > Document;



template <bool Const, typename ValueT>
class GenericArray {
public:
    typedef GenericArray<true, ValueT> ConstArray;
    typedef GenericArray<false, ValueT> Array;
    typedef ValueT PlainType;
    typedef typename internal::MaybeAddConst<Const,PlainType>::Type ValueType;
    typedef ValueType* ValueIterator;  
    typedef const ValueT* ConstValueIterator;
    typedef typename ValueType::AllocatorType AllocatorType;
    typedef typename ValueType::StringRefType StringRefType;

    template <typename, typename>
    friend class GenericValue;

    GenericArray(const GenericArray& rhs) : value_(rhs.value_) {}
    GenericArray& operator=(const GenericArray& rhs) { value_ = rhs.value_; return *this; }
    ~GenericArray() {}

    SizeType Size() const { return value_.Size(); }
    SizeType Capacity() const { return value_.Capacity(); }
    bool Empty() const { return value_.Empty(); }
    void Clear() const { value_.Clear(); }
    ValueType& operator[](SizeType index) const {  return value_[index]; }
    ValueIterator Begin() const { return value_.Begin(); }
    ValueIterator End() const { return value_.End(); }
    GenericArray Reserve(SizeType newCapacity, AllocatorType &allocator) const { value_.Reserve(newCapacity, allocator); return *this; }
    GenericArray PushBack(ValueType& value, AllocatorType& allocator) const { value_.PushBack(value, allocator); return *this; }
#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericArray PushBack(ValueType&& value, AllocatorType& allocator) const { value_.PushBack(value, allocator); return *this; }
#endif 
    GenericArray PushBack(StringRefType value, AllocatorType& allocator) const { value_.PushBack(value, allocator); return *this; }
    template <typename T> CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (const GenericArray&)) PushBack(T value, AllocatorType& allocator) const { value_.PushBack(value, allocator); return *this; }
    GenericArray PopBack() const { value_.PopBack(); return *this; }
    ValueIterator Erase(ConstValueIterator pos) const { return value_.Erase(pos); }
    ValueIterator Erase(ConstValueIterator first, ConstValueIterator last) const { return value_.Erase(first, last); }

#if CEREAL_RAPIDJSON_HAS_CXX11_RANGE_FOR
    ValueIterator begin() const { return value_.Begin(); }
    ValueIterator end() const { return value_.End(); }
#endif

private:
    GenericArray();
    GenericArray(ValueType& value) : value_(value) {}
    ValueType& value_;
};



template <bool Const, typename ValueT>
class GenericObject {
public:
    typedef GenericObject<true, ValueT> ConstObject;
    typedef GenericObject<false, ValueT> Object;
    typedef ValueT PlainType;
    typedef typename internal::MaybeAddConst<Const,PlainType>::Type ValueType;
    typedef GenericMemberIterator<Const, typename ValueT::EncodingType, typename ValueT::AllocatorType> MemberIterator;  
    typedef GenericMemberIterator<true, typename ValueT::EncodingType, typename ValueT::AllocatorType> ConstMemberIterator;
    typedef typename ValueType::AllocatorType AllocatorType;
    typedef typename ValueType::StringRefType StringRefType;
    typedef typename ValueType::EncodingType EncodingType;
    typedef typename ValueType::Ch Ch;

    template <typename, typename>
    friend class GenericValue;

    GenericObject(const GenericObject& rhs) : value_(rhs.value_) {}
    GenericObject& operator=(const GenericObject& rhs) { value_ = rhs.value_; return *this; }
    ~GenericObject() {}

    SizeType MemberCount() const { return value_.MemberCount(); }
    SizeType MemberCapacity() const { return value_.MemberCapacity(); }
    bool ObjectEmpty() const { return value_.ObjectEmpty(); }
    template <typename T> ValueType& operator[](T* name) const { return value_[name]; }
    template <typename SourceAllocator> ValueType& operator[](const GenericValue<EncodingType, SourceAllocator>& name) const { return value_[name]; }
#if CEREAL_RAPIDJSON_HAS_STDSTRING
    ValueType& operator[](const std::basic_string<Ch>& name) const { return value_[name]; }
#endif
    MemberIterator MemberBegin() const { return value_.MemberBegin(); }
    MemberIterator MemberEnd() const { return value_.MemberEnd(); }
    GenericObject MemberReserve(SizeType newCapacity, AllocatorType &allocator) const { value_.MemberReserve(newCapacity, allocator); return *this; }
    bool HasMember(const Ch* name) const { return value_.HasMember(name); }
#if CEREAL_RAPIDJSON_HAS_STDSTRING
    bool HasMember(const std::basic_string<Ch>& name) const { return value_.HasMember(name); }
#endif
    template <typename SourceAllocator> bool HasMember(const GenericValue<EncodingType, SourceAllocator>& name) const { return value_.HasMember(name); }
    MemberIterator FindMember(const Ch* name) const { return value_.FindMember(name); }
    template <typename SourceAllocator> MemberIterator FindMember(const GenericValue<EncodingType, SourceAllocator>& name) const { return value_.FindMember(name); }
#if CEREAL_RAPIDJSON_HAS_STDSTRING
    MemberIterator FindMember(const std::basic_string<Ch>& name) const { return value_.FindMember(name); }
#endif
    GenericObject AddMember(ValueType& name, ValueType& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(ValueType& name, StringRefType value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
#if CEREAL_RAPIDJSON_HAS_STDSTRING
    GenericObject AddMember(ValueType& name, std::basic_string<Ch>& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
#endif
    template <typename T> CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&)) AddMember(ValueType& name, T value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    GenericObject AddMember(ValueType&& name, ValueType&& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(ValueType&& name, ValueType& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(ValueType& name, ValueType&& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(StringRefType name, ValueType&& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
#endif 
    GenericObject AddMember(StringRefType name, ValueType& value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    GenericObject AddMember(StringRefType name, StringRefType value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    template <typename T> CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (GenericObject)) AddMember(StringRefType name, T value, AllocatorType& allocator) const { value_.AddMember(name, value, allocator); return *this; }
    void RemoveAllMembers() { value_.RemoveAllMembers(); }
    bool RemoveMember(const Ch* name) const { return value_.RemoveMember(name); }
#if CEREAL_RAPIDJSON_HAS_STDSTRING
    bool RemoveMember(const std::basic_string<Ch>& name) const { return value_.RemoveMember(name); }
#endif
    template <typename SourceAllocator> bool RemoveMember(const GenericValue<EncodingType, SourceAllocator>& name) const { return value_.RemoveMember(name); }
    MemberIterator RemoveMember(MemberIterator m) const { return value_.RemoveMember(m); }
    MemberIterator EraseMember(ConstMemberIterator pos) const { return value_.EraseMember(pos); }
    MemberIterator EraseMember(ConstMemberIterator first, ConstMemberIterator last) const { return value_.EraseMember(first, last); }
    bool EraseMember(const Ch* name) const { return value_.EraseMember(name); }
#if CEREAL_RAPIDJSON_HAS_STDSTRING
    bool EraseMember(const std::basic_string<Ch>& name) const { return EraseMember(ValueType(StringRef(name))); }
#endif
    template <typename SourceAllocator> bool EraseMember(const GenericValue<EncodingType, SourceAllocator>& name) const { return value_.EraseMember(name); }

#if CEREAL_RAPIDJSON_HAS_CXX11_RANGE_FOR
    MemberIterator begin() const { return value_.MemberBegin(); }
    MemberIterator end() const { return value_.MemberEnd(); }
#endif

private:
    GenericObject();
    GenericObject(ValueType& value) : value_(value) {}
    ValueType& value_;
};

CEREAL_RAPIDJSON_NAMESPACE_END
CEREAL_RAPIDJSON_DIAG_POP

#endif 
