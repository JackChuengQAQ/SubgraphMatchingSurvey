













#ifndef CEREAL_RAPIDJSON_OSTREAMWRAPPER_H_
#define CEREAL_RAPIDJSON_OSTREAMWRAPPER_H_

#include "stream.h"
#include <iosfwd>

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(padded)
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN



   
template <typename StreamType>
class BasicOStreamWrapper {
public:
    typedef typename StreamType::char_type Ch;
    BasicOStreamWrapper(StreamType& stream) : stream_(stream) {}

    void Put(Ch c) {
        stream_.put(c);
    }

    void Flush() {
        stream_.flush();
    }

    
    char Peek() const { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    char Take() { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    size_t Tell() const { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    char* PutBegin() { CEREAL_RAPIDJSON_ASSERT(false); return 0; }
    size_t PutEnd(char*) { CEREAL_RAPIDJSON_ASSERT(false); return 0; }

private:
    BasicOStreamWrapper(const BasicOStreamWrapper&);
    BasicOStreamWrapper& operator=(const BasicOStreamWrapper&);

    StreamType& stream_;
};

typedef BasicOStreamWrapper<std::ostream> OStreamWrapper;
typedef BasicOStreamWrapper<std::wostream> WOStreamWrapper;

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_POP
#endif

CEREAL_RAPIDJSON_NAMESPACE_END

#endif 
