













#ifndef CEREAL_RAPIDJSON_ERROR_ERROR_H_
#define CEREAL_RAPIDJSON_ERROR_ERROR_H_

#include "../rapidjson.h"

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(padded)
#endif










#ifndef CEREAL_RAPIDJSON_ERROR_CHARTYPE
#define CEREAL_RAPIDJSON_ERROR_CHARTYPE char
#endif






#ifndef CEREAL_RAPIDJSON_ERROR_STRING
#define CEREAL_RAPIDJSON_ERROR_STRING(x) x
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN






enum ParseErrorCode {
    kParseErrorNone = 0,                        

    kParseErrorDocumentEmpty,                   
    kParseErrorDocumentRootNotSingular,         

    kParseErrorValueInvalid,                    

    kParseErrorObjectMissName,                  
    kParseErrorObjectMissColon,                 
    kParseErrorObjectMissCommaOrCurlyBracket,   

    kParseErrorArrayMissCommaOrSquareBracket,   

    kParseErrorStringUnicodeEscapeInvalidHex,   
    kParseErrorStringUnicodeSurrogateInvalid,   
    kParseErrorStringEscapeInvalid,             
    kParseErrorStringMissQuotationMark,         
    kParseErrorStringInvalidEncoding,           

    kParseErrorNumberTooBig,                    
    kParseErrorNumberMissFraction,              
    kParseErrorNumberMissExponent,              

    kParseErrorTermination,                     
    kParseErrorUnspecificSyntaxError            
};



struct ParseResult {
    
    typedef bool (ParseResult::*BooleanType)() const;
public:
    
    ParseResult() : code_(kParseErrorNone), offset_(0) {}
    
    ParseResult(ParseErrorCode code, size_t offset) : code_(code), offset_(offset) {}

    
    ParseErrorCode Code() const { return code_; }
    
    size_t Offset() const { return offset_; }

    
    operator BooleanType() const { return !IsError() ? &ParseResult::IsError : NULL; }
    
    bool IsError() const { return code_ != kParseErrorNone; }

    bool operator==(const ParseResult& that) const { return code_ == that.code_; }
    bool operator==(ParseErrorCode code) const { return code_ == code; }
    friend bool operator==(ParseErrorCode code, const ParseResult & err) { return code == err.code_; }

    bool operator!=(const ParseResult& that) const { return !(*this == that); }
    bool operator!=(ParseErrorCode code) const { return !(*this == code); }
    friend bool operator!=(ParseErrorCode code, const ParseResult & err) { return err != code; }

    
    void Clear() { Set(kParseErrorNone); }
    
    void Set(ParseErrorCode code, size_t offset = 0) { code_ = code; offset_ = offset; }

private:
    ParseErrorCode code_;
    size_t offset_;
};



typedef const CEREAL_RAPIDJSON_ERROR_CHARTYPE* (*GetParseErrorFunc)(ParseErrorCode);

CEREAL_RAPIDJSON_NAMESPACE_END

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_POP
#endif

#endif 
