













#ifndef CEREAL_RAPIDJSON_CURSORSTREAMWRAPPER_H_
#define CEREAL_RAPIDJSON_CURSORSTREAMWRAPPER_H_

#include "stream.h"

#if defined(__GNUC__)
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(effc++)
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1800
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(4702)  
CEREAL_RAPIDJSON_DIAG_OFF(4512)  
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN




template <typename InputStream, typename Encoding = UTF8<> >
class CursorStreamWrapper : public GenericStreamWrapper<InputStream, Encoding> {
public:
    typedef typename Encoding::Ch Ch;

    CursorStreamWrapper(InputStream& is):
        GenericStreamWrapper<InputStream, Encoding>(is), line_(1), col_(0) {}

    
    Ch Take() {
        Ch ch = this->is_.Take();
        if(ch == '\n') {
            line_ ++;
            col_ = 0;
        } else {
            col_ ++;
        }
        return ch;
    }

    
    size_t GetLine() const { return line_; }
    
    size_t GetColumn() const { return col_; }

private:
    size_t line_;   
    size_t col_;    
};

#if defined(_MSC_VER) && _MSC_VER <= 1800
CEREAL_RAPIDJSON_DIAG_POP
#endif

#if defined(__GNUC__)
CEREAL_RAPIDJSON_DIAG_POP
#endif

CEREAL_RAPIDJSON_NAMESPACE_END

#endif 
