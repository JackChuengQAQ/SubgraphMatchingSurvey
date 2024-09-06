













#include "rapidjson.h"

#ifndef CEREAL_RAPIDJSON_STREAM_H_
#define CEREAL_RAPIDJSON_STREAM_H_

#include "encodings.h"

CEREAL_RAPIDJSON_NAMESPACE_BEGIN








template<typename Stream>
struct StreamTraits {
    
    
    enum { copyOptimization = 0 };
};


template<typename Stream>
inline void PutReserve(Stream& stream, size_t count) {
    (void)stream;
    (void)count;
}


template<typename Stream>
inline void PutUnsafe(Stream& stream, typename Stream::Ch c) {
    stream.Put(c);
}


template<typename Stream, typename Ch>
inline void PutN(Stream& stream, Ch c, size_t n) {
    PutReserve(stream, n);
    for (size_t i = 0; i < n; i++)
        PutUnsafe(stream, c);
}







#if defined(_MSC_VER) && _MSC_VER <= 1800
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(4702)  
CEREAL_RAPIDJSON_DIAG_OFF(4512)  
#endif

template <typename InputStream, typename Encoding = UTF8<> >
class GenericStreamWrapper {
public:
    typedef typename Encoding::Ch Ch;
    GenericStreamWrapper(InputStream& is): is_(is) {}

    Ch Peek() const { return is_.Peek(); }
    Ch Take() { return is_.Take(); }
    size_t Tell() { return is_.Tell(); }
    Ch* PutBegin() { return is_.PutBegin(); }
    void Put(Ch ch) { is_.Put(ch); }
    void Flush() { is_.Flush(); }
    size_t PutEnd(Ch* ch) { return is_.PutEnd(ch); }

    
    const Ch* Peek4() const { return is_.Peek4(); }

    
    UTFType GetType() const { return is_.GetType(); }
    bool HasBOM() const { return is_.HasBOM(); }

protected:
    InputStream& is_;
};

#if defined(_MSC_VER) && _MSC_VER <= 1800
CEREAL_RAPIDJSON_DIAG_POP
#endif






template <typename Encoding>
struct GenericStringStream {
    typedef typename Encoding::Ch Ch;

    GenericStringStream(const Ch *src) : src_(src), head_(src) {}

    Ch Peek() const { return *src_; }
    Ch Take() { return *src_++; }
    size_t Tell() const { return static_cast<size_t>(src_ - head_); }

    Ch* PutBegin() { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    void Put(Ch) { CEREAL_RAPIDJSON_ASSERT(false); }
    void Flush() { CEREAL_RAPIDJSON_ASSERT(false); }
    size_t PutEnd(Ch*) { CEREAL_RAPIDJSON_ASSERT(false); return 0; }

    const Ch* src_;     
    const Ch* head_;    
};

template <typename Encoding>
struct StreamTraits<GenericStringStream<Encoding> > {
    enum { copyOptimization = 1 };
};


typedef GenericStringStream<UTF8<> > StringStream;






template <typename Encoding>
struct GenericInsituStringStream {
    typedef typename Encoding::Ch Ch;

    GenericInsituStringStream(Ch *src) : src_(src), dst_(0), head_(src) {}

    
    Ch Peek() { return *src_; }
    Ch Take() { return *src_++; }
    size_t Tell() { return static_cast<size_t>(src_ - head_); }

    
    void Put(Ch c) { CEREAL_RAPIDJSON_ASSERT(dst_ != 0); *dst_++ = c; }

    Ch* PutBegin() { return dst_ = src_; }
    size_t PutEnd(Ch* begin) { return static_cast<size_t>(dst_ - begin); }
    void Flush() {}

    Ch* Push(size_t count) { Ch* begin = dst_; dst_ += count; return begin; }
    void Pop(size_t count) { dst_ -= count; }

    Ch* src_;
    Ch* dst_;
    Ch* head_;
};

template <typename Encoding>
struct StreamTraits<GenericInsituStringStream<Encoding> > {
    enum { copyOptimization = 1 };
};


typedef GenericInsituStringStream<UTF8<> > InsituStringStream;

CEREAL_RAPIDJSON_NAMESPACE_END

#endif 
