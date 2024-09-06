













#ifndef CEREAL_RAPIDJSON_FILEREADSTREAM_H_
#define CEREAL_RAPIDJSON_FILEREADSTREAM_H_

#include "stream.h"
#include <cstdio>

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(padded)
CEREAL_RAPIDJSON_DIAG_OFF(unreachable-code)
CEREAL_RAPIDJSON_DIAG_OFF(missing-noreturn)
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN



class FileReadStream {
public:
    typedef char Ch;    

    
    
    FileReadStream(std::FILE* fp, char* buffer, size_t bufferSize) : fp_(fp), buffer_(buffer), bufferSize_(bufferSize), bufferLast_(0), current_(buffer_), readCount_(0), count_(0), eof_(false) { 
        CEREAL_RAPIDJSON_ASSERT(fp_ != 0);
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
    void Read() {
        if (current_ < bufferLast_)
            ++current_;
        else if (!eof_) {
            count_ += readCount_;
            readCount_ = std::fread(buffer_, 1, bufferSize_, fp_);
            bufferLast_ = buffer_ + readCount_ - 1;
            current_ = buffer_;

            if (readCount_ < bufferSize_) {
                buffer_[readCount_] = '\0';
                ++bufferLast_;
                eof_ = true;
            }
        }
    }

    std::FILE* fp_;
    Ch *buffer_;
    size_t bufferSize_;
    Ch *bufferLast_;
    Ch *current_;
    size_t readCount_;
    size_t count_;  
    bool eof_;
};

CEREAL_RAPIDJSON_NAMESPACE_END

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_POP
#endif

#endif 
