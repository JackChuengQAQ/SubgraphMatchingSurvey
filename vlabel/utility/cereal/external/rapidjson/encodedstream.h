













#ifndef CEREAL_RAPIDJSON_ENCODEDSTREAM_H_
#define CEREAL_RAPIDJSON_ENCODEDSTREAM_H_

#include "stream.h"
#include "memorystream.h"

#ifdef __GNUC__
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(effc++)
#endif

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(padded)
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN



template <typename Encoding, typename InputByteStream>
class EncodedInputStream {
    CEREAL_RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
public:
    typedef typename Encoding::Ch Ch;

    EncodedInputStream(InputByteStream& is) : is_(is) { 
        current_ = Encoding::TakeBOM(is_);
    }

    Ch Peek() const { return current_; }
    Ch Take() { Ch c = current_; current_ = Encoding::Take(is_); return c; }
    size_t Tell() const { return is_.Tell(); }

    
    void Put(Ch) { CEREAL_RAPIDJSON_ASSERT(false); }
    void Flush() { CEREAL_RAPIDJSON_ASSERT(false); } 
    Ch* PutBegin() { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    size_t PutEnd(Ch*) { CEREAL_RAPIDJSON_ASSERT(false); return 0; }

private:
    EncodedInputStream(const EncodedInputStream&);
    EncodedInputStream& operator=(const EncodedInputStream&);

    InputByteStream& is_;
    Ch current_;
};


template <>
class EncodedInputStream<UTF8<>, MemoryStream> {
public:
    typedef UTF8<>::Ch Ch;

    EncodedInputStream(MemoryStream& is) : is_(is) {
        if (static_cast<unsigned char>(is_.Peek()) == 0xEFu) is_.Take();
        if (static_cast<unsigned char>(is_.Peek()) == 0xBBu) is_.Take();
        if (static_cast<unsigned char>(is_.Peek()) == 0xBFu) is_.Take();
    }
    Ch Peek() const { return is_.Peek(); }
    Ch Take() { return is_.Take(); }
    size_t Tell() const { return is_.Tell(); }

    
    void Put(Ch) {}
    void Flush() {} 
    Ch* PutBegin() { return 0; }
    size_t PutEnd(Ch*) { return 0; }

    MemoryStream& is_;

private:
    EncodedInputStream(const EncodedInputStream&);
    EncodedInputStream& operator=(const EncodedInputStream&);
};



template <typename Encoding, typename OutputByteStream>
class EncodedOutputStream {
    CEREAL_RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
public:
    typedef typename Encoding::Ch Ch;

    EncodedOutputStream(OutputByteStream& os, bool putBOM = true) : os_(os) { 
        if (putBOM)
            Encoding::PutBOM(os_);
    }

    void Put(Ch c) { Encoding::Put(os_, c);  }
    void Flush() { os_.Flush(); }

    
    Ch Peek() const { CEREAL_RAPIDJSON_ASSERT(false); return 0;}
    Ch Take() { CEREAL_RAPIDJSON_ASSERT(false); return 0;}
    size_t Tell() const { CEREAL_RAPIDJSON_ASSERT(false);  return 0; }
    Ch* PutBegin() { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    size_t PutEnd(Ch*) { CEREAL_RAPIDJSON_ASSERT(false); return 0; }

private:
    EncodedOutputStream(const EncodedOutputStream&);
    EncodedOutputStream& operator=(const EncodedOutputStream&);

    OutputByteStream& os_;
};

#define CEREAL_RAPIDJSON_ENCODINGS_FUNC(x) UTF8<Ch>::x, UTF16LE<Ch>::x, UTF16BE<Ch>::x, UTF32LE<Ch>::x, UTF32BE<Ch>::x



template <typename CharType, typename InputByteStream>
class AutoUTFInputStream {
    CEREAL_RAPIDJSON_STATIC_ASSERT(sizeof(typename InputByteStream::Ch) == 1);
public:
    typedef CharType Ch;

    
    
    AutoUTFInputStream(InputByteStream& is, UTFType type = kUTF8) : is_(&is), type_(type), hasBOM_(false) {
        CEREAL_RAPIDJSON_ASSERT(type >= kUTF8 && type <= kUTF32BE);        
        DetectType();
        static const TakeFunc f[] = { CEREAL_RAPIDJSON_ENCODINGS_FUNC(Take) };
        takeFunc_ = f[type_];
        current_ = takeFunc_(*is_);
    }

    UTFType GetType() const { return type_; }
    bool HasBOM() const { return hasBOM_; }

    Ch Peek() const { return current_; }
    Ch Take() { Ch c = current_; current_ = takeFunc_(*is_); return c; }
    size_t Tell() const { return is_->Tell(); }

    
    void Put(Ch) { CEREAL_RAPIDJSON_ASSERT(false); }
    void Flush() { CEREAL_RAPIDJSON_ASSERT(false); } 
    Ch* PutBegin() { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    size_t PutEnd(Ch*) { CEREAL_RAPIDJSON_ASSERT(false); return 0; }

private:
    AutoUTFInputStream(const AutoUTFInputStream&);
    AutoUTFInputStream& operator=(const AutoUTFInputStream&);

    
    void DetectType() {
        
        
        
        
        
        

        const unsigned char* c = reinterpret_cast<const unsigned char *>(is_->Peek4());
        if (!c)
            return;

        unsigned bom = static_cast<unsigned>(c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));
        hasBOM_ = false;
        if (bom == 0xFFFE0000)                  { type_ = kUTF32BE; hasBOM_ = true; is_->Take(); is_->Take(); is_->Take(); is_->Take(); }
        else if (bom == 0x0000FEFF)             { type_ = kUTF32LE; hasBOM_ = true; is_->Take(); is_->Take(); is_->Take(); is_->Take(); }
        else if ((bom & 0xFFFF) == 0xFFFE)      { type_ = kUTF16BE; hasBOM_ = true; is_->Take(); is_->Take();                           }
        else if ((bom & 0xFFFF) == 0xFEFF)      { type_ = kUTF16LE; hasBOM_ = true; is_->Take(); is_->Take();                           }
        else if ((bom & 0xFFFFFF) == 0xBFBBEF)  { type_ = kUTF8;    hasBOM_ = true; is_->Take(); is_->Take(); is_->Take();              }

        
        
        
        
        
        
        
        
        
        

        if (!hasBOM_) {
            int pattern = (c[0] ? 1 : 0) | (c[1] ? 2 : 0) | (c[2] ? 4 : 0) | (c[3] ? 8 : 0);
            switch (pattern) {
            case 0x08: type_ = kUTF32BE; break;
            case 0x0A: type_ = kUTF16BE; break;
            case 0x01: type_ = kUTF32LE; break;
            case 0x05: type_ = kUTF16LE; break;
            case 0x0F: type_ = kUTF8;    break;
            default: break; 
            }
        }

        
        if (type_ == kUTF16LE || type_ == kUTF16BE) CEREAL_RAPIDJSON_ASSERT(sizeof(Ch) >= 2);
        if (type_ == kUTF32LE || type_ == kUTF32BE) CEREAL_RAPIDJSON_ASSERT(sizeof(Ch) >= 4);
    }

    typedef Ch (*TakeFunc)(InputByteStream& is);
    InputByteStream* is_;
    UTFType type_;
    Ch current_;
    TakeFunc takeFunc_;
    bool hasBOM_;
};



template <typename CharType, typename OutputByteStream>
class AutoUTFOutputStream {
    CEREAL_RAPIDJSON_STATIC_ASSERT(sizeof(typename OutputByteStream::Ch) == 1);
public:
    typedef CharType Ch;

    
    
    AutoUTFOutputStream(OutputByteStream& os, UTFType type, bool putBOM) : os_(&os), type_(type) {
        CEREAL_RAPIDJSON_ASSERT(type >= kUTF8 && type <= kUTF32BE);

        
        if (type_ == kUTF16LE || type_ == kUTF16BE) CEREAL_RAPIDJSON_ASSERT(sizeof(Ch) >= 2);
        if (type_ == kUTF32LE || type_ == kUTF32BE) CEREAL_RAPIDJSON_ASSERT(sizeof(Ch) >= 4);

        static const PutFunc f[] = { CEREAL_RAPIDJSON_ENCODINGS_FUNC(Put) };
        putFunc_ = f[type_];

        if (putBOM)
            PutBOM();
    }

    UTFType GetType() const { return type_; }

    void Put(Ch c) { putFunc_(*os_, c); }
    void Flush() { os_->Flush(); } 

    
    Ch Peek() const { CEREAL_RAPIDJSON_ASSERT(false); return 0;}
    Ch Take() { CEREAL_RAPIDJSON_ASSERT(false); return 0;}
    size_t Tell() const { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    Ch* PutBegin() { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    size_t PutEnd(Ch*) { CEREAL_RAPIDJSON_ASSERT(false); return 0; }

private:
    AutoUTFOutputStream(const AutoUTFOutputStream&);
    AutoUTFOutputStream& operator=(const AutoUTFOutputStream&);

    void PutBOM() { 
        typedef void (*PutBOMFunc)(OutputByteStream&);
        static const PutBOMFunc f[] = { CEREAL_RAPIDJSON_ENCODINGS_FUNC(PutBOM) };
        f[type_](*os_);
    }

    typedef void (*PutFunc)(OutputByteStream&, Ch);

    OutputByteStream* os_;
    UTFType type_;
    PutFunc putFunc_;
};

#undef CEREAL_RAPIDJSON_ENCODINGS_FUNC

CEREAL_RAPIDJSON_NAMESPACE_END

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_POP
#endif

#ifdef __GNUC__
CEREAL_RAPIDJSON_DIAG_POP
#endif

#endif 
