













#ifndef CEREAL_RAPIDJSON_PRETTYWRITER_H_
#define CEREAL_RAPIDJSON_PRETTYWRITER_H_

#include "writer.h"

#ifdef __GNUC__
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(effc++)
#endif

#if defined(__clang__)
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(c++98-compat)
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN



enum PrettyFormatOptions {
    kFormatDefault = 0,         
    kFormatSingleLineArray = 1  
};



template<typename OutputStream, typename SourceEncoding = UTF8<>, typename TargetEncoding = UTF8<>, typename StackAllocator = CrtAllocator, unsigned writeFlags = kWriteDefaultFlags>
class PrettyWriter : public Writer<OutputStream, SourceEncoding, TargetEncoding, StackAllocator, writeFlags> {
public:
    typedef Writer<OutputStream, SourceEncoding, TargetEncoding, StackAllocator, writeFlags> Base;
    typedef typename Base::Ch Ch;

    
    
    explicit PrettyWriter(OutputStream& os, StackAllocator* allocator = 0, size_t levelDepth = Base::kDefaultLevelDepth) : 
        Base(os, allocator, levelDepth), indentChar_(' '), indentCharCount_(4), formatOptions_(kFormatDefault) {}


    explicit PrettyWriter(StackAllocator* allocator = 0, size_t levelDepth = Base::kDefaultLevelDepth) : 
        Base(allocator, levelDepth), indentChar_(' '), indentCharCount_(4) {}

#if CEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS
    PrettyWriter(PrettyWriter&& rhs) :
        Base(std::forward<PrettyWriter>(rhs)), indentChar_(rhs.indentChar_), indentCharCount_(rhs.indentCharCount_), formatOptions_(rhs.formatOptions_) {}
#endif

    
    
    PrettyWriter& SetIndent(Ch indentChar, unsigned indentCharCount) {
        CEREAL_RAPIDJSON_ASSERT(indentChar == ' ' || indentChar == '\t' || indentChar == '\n' || indentChar == '\r');
        indentChar_ = indentChar;
        indentCharCount_ = indentCharCount;
        return *this;
    }

    
    
    PrettyWriter& SetFormatOptions(PrettyFormatOptions options) {
        formatOptions_ = options;
        return *this;
    }

    
    

    bool Null()                 { PrettyPrefix(kNullType);   return Base::EndValue(Base::WriteNull()); }
    bool Bool(bool b)           { PrettyPrefix(b ? kTrueType : kFalseType); return Base::EndValue(Base::WriteBool(b)); }
    bool Int(int i)             { PrettyPrefix(kNumberType); return Base::EndValue(Base::WriteInt(i)); }
    bool Uint(unsigned u)       { PrettyPrefix(kNumberType); return Base::EndValue(Base::WriteUint(u)); }
    bool Int64(int64_t i64)     { PrettyPrefix(kNumberType); return Base::EndValue(Base::WriteInt64(i64)); }
    bool Uint64(uint64_t u64)   { PrettyPrefix(kNumberType); return Base::EndValue(Base::WriteUint64(u64));  }
    bool Double(double d)       { PrettyPrefix(kNumberType); return Base::EndValue(Base::WriteDouble(d)); }

    bool RawNumber(const Ch* str, SizeType length, bool copy = false) {
        CEREAL_RAPIDJSON_ASSERT(str != 0);
        (void)copy;
        PrettyPrefix(kNumberType);
        return Base::EndValue(Base::WriteString(str, length));
    }

    bool String(const Ch* str, SizeType length, bool copy = false) {
        CEREAL_RAPIDJSON_ASSERT(str != 0);
        (void)copy;
        PrettyPrefix(kStringType);
        return Base::EndValue(Base::WriteString(str, length));
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    bool String(const std::basic_string<Ch>& str) {
        return String(str.data(), SizeType(str.size()));
    }
#endif

    bool StartObject() {
        PrettyPrefix(kObjectType);
        new (Base::level_stack_.template Push<typename Base::Level>()) typename Base::Level(false);
        return Base::WriteStartObject();
    }

    bool Key(const Ch* str, SizeType length, bool copy = false) { return String(str, length, copy); }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    bool Key(const std::basic_string<Ch>& str) {
        return Key(str.data(), SizeType(str.size()));
    }
#endif
	
    bool EndObject(SizeType memberCount = 0) {
        (void)memberCount;
        CEREAL_RAPIDJSON_ASSERT(Base::level_stack_.GetSize() >= sizeof(typename Base::Level)); 
        CEREAL_RAPIDJSON_ASSERT(!Base::level_stack_.template Top<typename Base::Level>()->inArray); 
        CEREAL_RAPIDJSON_ASSERT(0 == Base::level_stack_.template Top<typename Base::Level>()->valueCount % 2); 
       
        bool empty = Base::level_stack_.template Pop<typename Base::Level>(1)->valueCount == 0;

        if (!empty) {
            Base::os_->Put('\n');
            WriteIndent();
        }
        bool ret = Base::EndValue(Base::WriteEndObject());
        (void)ret;
        CEREAL_RAPIDJSON_ASSERT(ret == true);
        if (Base::level_stack_.Empty()) 
            Base::Flush();
        return true;
    }

    bool StartArray() {
        PrettyPrefix(kArrayType);
        new (Base::level_stack_.template Push<typename Base::Level>()) typename Base::Level(true);
        return Base::WriteStartArray();
    }

    bool EndArray(SizeType memberCount = 0) {
        (void)memberCount;
        CEREAL_RAPIDJSON_ASSERT(Base::level_stack_.GetSize() >= sizeof(typename Base::Level));
        CEREAL_RAPIDJSON_ASSERT(Base::level_stack_.template Top<typename Base::Level>()->inArray);
        bool empty = Base::level_stack_.template Pop<typename Base::Level>(1)->valueCount == 0;

        if (!empty && !(formatOptions_ & kFormatSingleLineArray)) {
            Base::os_->Put('\n');
            WriteIndent();
        }
        bool ret = Base::EndValue(Base::WriteEndArray());
        (void)ret;
        CEREAL_RAPIDJSON_ASSERT(ret == true);
        if (Base::level_stack_.Empty()) 
            Base::Flush();
        return true;
    }

    

    
    

    
    bool String(const Ch* str) { return String(str, internal::StrLen(str)); }
    bool Key(const Ch* str) { return Key(str, internal::StrLen(str)); }

    

    
    
    bool RawValue(const Ch* json, size_t length, Type type) {
        CEREAL_RAPIDJSON_ASSERT(json != 0);
        PrettyPrefix(type);
        return Base::EndValue(Base::WriteRawValue(json, length));
    }

protected:
    void PrettyPrefix(Type type) {
        (void)type;
        if (Base::level_stack_.GetSize() != 0) { 
            typename Base::Level* level = Base::level_stack_.template Top<typename Base::Level>();

            if (level->inArray) {
                if (level->valueCount > 0) {
                    Base::os_->Put(','); 
                    if (formatOptions_ & kFormatSingleLineArray)
                        Base::os_->Put(' ');
                }

                if (!(formatOptions_ & kFormatSingleLineArray)) {
                    Base::os_->Put('\n');
                    WriteIndent();
                }
            }
            else {  
                if (level->valueCount > 0) {
                    if (level->valueCount % 2 == 0) {
                        Base::os_->Put(',');
                        Base::os_->Put('\n');
                    }
                    else {
                        Base::os_->Put(':');
                        Base::os_->Put(' ');
                    }
                }
                else
                    Base::os_->Put('\n');

                if (level->valueCount % 2 == 0)
                    WriteIndent();
            }
            if (!level->inArray && level->valueCount % 2 == 0)
                CEREAL_RAPIDJSON_ASSERT(type == kStringType);  
            level->valueCount++;
        }
        else {
            CEREAL_RAPIDJSON_ASSERT(!Base::hasRoot_);  
            Base::hasRoot_ = true;
        }
    }

    void WriteIndent()  {
        size_t count = (Base::level_stack_.GetSize() / sizeof(typename Base::Level)) * indentCharCount_;
        PutN(*Base::os_, static_cast<typename OutputStream::Ch>(indentChar_), count);
    }

    Ch indentChar_;
    unsigned indentCharCount_;
    PrettyFormatOptions formatOptions_;

private:
    
    PrettyWriter(const PrettyWriter&);
    PrettyWriter& operator=(const PrettyWriter&);
};

CEREAL_RAPIDJSON_NAMESPACE_END

#if defined(__clang__)
CEREAL_RAPIDJSON_DIAG_POP
#endif

#ifdef __GNUC__
CEREAL_RAPIDJSON_DIAG_POP
#endif

#endif 
