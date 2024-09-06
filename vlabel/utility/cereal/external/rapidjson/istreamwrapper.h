













#ifndef CEREAL_RAPIDJSON_ISTREAMWRAPPER_H_
#define CEREAL_RAPIDJSON_ISTREAMWRAPPER_H_

#include "stream.h"
#include <iosfwd>
#include <ios>

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(padded)
#elif defined(_MSC_VER)
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(4351) 
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN



   
template <typename StreamType>
class BasicIStreamWrapper {
public:
    typedef typename StreamType::char_type Ch;

    
    
    BasicIStreamWrapper(StreamType &stream) : stream_(stream), buffer_(peekBuffer_), bufferSize_(4), bufferLast_(0), current_(buffer_), readCount_(0), count_(0), eof_(false) { 
        Read();
    }

    
    
    BasicIStreamWrapper(StreamType &stream, char* buffer, size_t bufferSize) : stream_(stream), buffer_(buffer), bufferSize_(bufferSize), bufferLast_(0), current_(buffer_), readCount_(0), count_(0), eof_(false) { 
        CEREAL_RAPIDJSON_ASSERT(bufferSize >= 4);
        Read();
    }

    Ch Peek() const { return *current_; }
    Ch Take() { Ch c = *current_; Read(); return c; }
    size_t Tell() const { return count_ + static_cast<size_t>(current_ - buffer_); }

    
    void Put(Ch) { CEREAL_RAPIDJSON_ASSERT(false); }
    void Flush() { CEREAL_RAPIDJSON_ASSERT(false); } 
    Ch* PutBegin() { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    size_t PutEnd(Ch*) { CEREAL_RAPIDJSON_ASSERT(false); return 0; }

    
    const Ch* Peek4() const {
        return (current_ + 4 - !eof_ <= bufferLast_) ? current_ : 0;
    }

private:
    BasicIStreamWrapper();
    BasicIStreamWrapper(const BasicIStreamWrapper&);
    BasicIStreamWrapper& operator=(const BasicIStreamWrapper&);

    void Read() {
        if (current_ < bufferLast_)
            ++current_;
        else if (!eof_) {
            count_ += readCount_;
            readCount_ = bufferSize_;
            bufferLast_ = buffer_ + readCount_ - 1;
            current_ = buffer_;

            if (!stream_.read(buffer_, static_cast<std::streamsize>(bufferSize_))) {
                readCount_ = static_cast<size_t>(stream_.gcount());
                *(bufferLast_ = buffer_ + readCount_) = '\0';
                eof_ = true;
            }
        }
    }

    StreamType &stream_;
    Ch peekBuffer_[4], *buffer_;
    size_t bufferSize_;
    Ch *bufferLast_;
    Ch *current_;
    size_t readCount_;
    size_t count_;  
    bool eof_;
};

typedef BasicIStreamWrapper<std::istream> IStreamWrapper;
typedef BasicIStreamWrapper<std::wistream> WIStreamWrapper;

#if defined(__clang__) || defined(_MSC_VER)
CEREAL_RAPIDJSON_DIAG_POP
#endif

CEREAL_RAPIDJSON_NAMESPACE_END

#endif 
