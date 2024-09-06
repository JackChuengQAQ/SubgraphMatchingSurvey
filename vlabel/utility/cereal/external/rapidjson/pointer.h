













#ifndef CEREAL_RAPIDJSON_POINTER_H_
#define CEREAL_RAPIDJSON_POINTER_H_

#include "document.h"
#include "internal/itoa.h"

#ifdef __clang__
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(switch-enum)
#elif defined(_MSC_VER)
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(4512) 
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN

static const SizeType kPointerInvalidIndex = ~SizeType(0);  



enum PointerParseErrorCode {
    kPointerParseErrorNone = 0,                     

    kPointerParseErrorTokenMustBeginWithSolidus,    
    kPointerParseErrorInvalidEscape,                
    kPointerParseErrorInvalidPercentEncoding,       
    kPointerParseErrorCharacterMustPercentEncode    
};






template <typename ValueType, typename Allocator = CrtAllocator>
class GenericPointer {
public:
    typedef typename ValueType::EncodingType EncodingType;  
    typedef typename ValueType::Ch Ch;                      

    
    
    struct Token {
        const Ch* name;             
        SizeType length;            
        SizeType index;             
    };

    
    

    
    GenericPointer(Allocator* allocator = 0) : allocator_(allocator), ownAllocator_(), nameBuffer_(), tokens_(), tokenCount_(), parseErrorOffset_(), parseErrorCode_(kPointerParseErrorNone) {}

    
    
    explicit GenericPointer(const Ch* source, Allocator* allocator = 0) : allocator_(allocator), ownAllocator_(), nameBuffer_(), tokens_(), tokenCount_(), parseErrorOffset_(), parseErrorCode_(kPointerParseErrorNone) {
        Parse(source, internal::StrLen(source));
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    
    explicit GenericPointer(const std::basic_string<Ch>& source, Allocator* allocator = 0) : allocator_(allocator), ownAllocator_(), nameBuffer_(), tokens_(), tokenCount_(), parseErrorOffset_(), parseErrorCode_(kPointerParseErrorNone) {
        Parse(source.c_str(), source.size());
    }
#endif

    
    
    GenericPointer(const Ch* source, size_t length, Allocator* allocator = 0) : allocator_(allocator), ownAllocator_(), nameBuffer_(), tokens_(), tokenCount_(), parseErrorOffset_(), parseErrorCode_(kPointerParseErrorNone) {
        Parse(source, length);
    }

    
    
    GenericPointer(const Token* tokens, size_t tokenCount) : allocator_(), ownAllocator_(), nameBuffer_(), tokens_(const_cast<Token*>(tokens)), tokenCount_(tokenCount), parseErrorOffset_(), parseErrorCode_(kPointerParseErrorNone) {}

    
    GenericPointer(const GenericPointer& rhs) : allocator_(rhs.allocator_), ownAllocator_(), nameBuffer_(), tokens_(), tokenCount_(), parseErrorOffset_(), parseErrorCode_(kPointerParseErrorNone) {
        *this = rhs;
    }

    
    GenericPointer(const GenericPointer& rhs, Allocator* allocator) : allocator_(allocator), ownAllocator_(), nameBuffer_(), tokens_(), tokenCount_(), parseErrorOffset_(), parseErrorCode_(kPointerParseErrorNone) {
        *this = rhs;
    }

    
    ~GenericPointer() {
        if (nameBuffer_)    
            Allocator::Free(tokens_);
        CEREAL_RAPIDJSON_DELETE(ownAllocator_);
    }

    
    GenericPointer& operator=(const GenericPointer& rhs) {
        if (this != &rhs) {
            
            if (nameBuffer_)
                Allocator::Free(tokens_);

            tokenCount_ = rhs.tokenCount_;
            parseErrorOffset_ = rhs.parseErrorOffset_;
            parseErrorCode_ = rhs.parseErrorCode_;

            if (rhs.nameBuffer_)
                CopyFromRaw(rhs); 
            else {
                tokens_ = rhs.tokens_; 
                nameBuffer_ = 0;
            }
        }
        return *this;
    }

    
    
    GenericPointer& Swap(GenericPointer& other) CEREAL_RAPIDJSON_NOEXCEPT {
        internal::Swap(allocator_, other.allocator_);
        internal::Swap(ownAllocator_, other.ownAllocator_);
        internal::Swap(nameBuffer_, other.nameBuffer_);
        internal::Swap(tokens_, other.tokens_);
        internal::Swap(tokenCount_, other.tokenCount_);
        internal::Swap(parseErrorOffset_, other.parseErrorOffset_);
        internal::Swap(parseErrorCode_, other.parseErrorCode_);
        return *this;
    }

    
    
    friend inline void swap(GenericPointer& a, GenericPointer& b) CEREAL_RAPIDJSON_NOEXCEPT { a.Swap(b); }

    

    
    

    
    
    GenericPointer Append(const Token& token, Allocator* allocator = 0) const {
        GenericPointer r;
        r.allocator_ = allocator;
        Ch *p = r.CopyFromRaw(*this, 1, token.length + 1);
        std::memcpy(p, token.name, (token.length + 1) * sizeof(Ch));
        r.tokens_[tokenCount_].name = p;
        r.tokens_[tokenCount_].length = token.length;
        r.tokens_[tokenCount_].index = token.index;
        return r;
    }

    
    
    GenericPointer Append(const Ch* name, SizeType length, Allocator* allocator = 0) const {
        Token token = { name, length, kPointerInvalidIndex };
        return Append(token, allocator);
    }

    
    
    template <typename T>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::NotExpr<internal::IsSame<typename internal::RemoveConst<T>::Type, Ch> >), (GenericPointer))
    Append(T* name, Allocator* allocator = 0) const {
        return Append(name, internal::StrLen(name), allocator);
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    
    GenericPointer Append(const std::basic_string<Ch>& name, Allocator* allocator = 0) const {
        return Append(name.c_str(), static_cast<SizeType>(name.size()), allocator);
    }
#endif

    
    
    GenericPointer Append(SizeType index, Allocator* allocator = 0) const {
        char buffer[21];
        char* end = sizeof(SizeType) == 4 ? internal::u32toa(index, buffer) : internal::u64toa(index, buffer);
        SizeType length = static_cast<SizeType>(end - buffer);
        buffer[length] = '\0';

        if (sizeof(Ch) == 1) {
            Token token = { reinterpret_cast<Ch*>(buffer), length, index };
            return Append(token, allocator);
        }
        else {
            Ch name[21];
            for (size_t i = 0; i <= length; i++)
                name[i] = static_cast<Ch>(buffer[i]);
            Token token = { name, length, index };
            return Append(token, allocator);
        }
    }

    
    
    GenericPointer Append(const ValueType& token, Allocator* allocator = 0) const {
        if (token.IsString())
            return Append(token.GetString(), token.GetStringLength(), allocator);
        else {
            CEREAL_RAPIDJSON_ASSERT(token.IsUint64());
            CEREAL_RAPIDJSON_ASSERT(token.GetUint64() <= SizeType(~0));
            return Append(static_cast<SizeType>(token.GetUint64()), allocator);
        }
    }

    
    

    
    bool IsValid() const { return parseErrorCode_ == kPointerParseErrorNone; }

    
    size_t GetParseErrorOffset() const { return parseErrorOffset_; }

    
    PointerParseErrorCode GetParseErrorCode() const { return parseErrorCode_; }

    

    
    Allocator& GetAllocator() { return *allocator_; }

    
    

    
    const Token* GetTokens() const { return tokens_; }

    
    size_t GetTokenCount() const { return tokenCount_; }

    

    
    

    
    
    bool operator==(const GenericPointer& rhs) const {
        if (!IsValid() || !rhs.IsValid() || tokenCount_ != rhs.tokenCount_)
            return false;

        for (size_t i = 0; i < tokenCount_; i++) {
            if (tokens_[i].index != rhs.tokens_[i].index ||
                tokens_[i].length != rhs.tokens_[i].length || 
                (tokens_[i].length != 0 && std::memcmp(tokens_[i].name, rhs.tokens_[i].name, sizeof(Ch)* tokens_[i].length) != 0))
            {
                return false;
            }
        }

        return true;
    }

    
    
    bool operator!=(const GenericPointer& rhs) const { return !(*this == rhs); }

    
    
    bool operator<(const GenericPointer& rhs) const {
        if (!IsValid())
            return false;
        if (!rhs.IsValid())
            return true;

        if (tokenCount_ != rhs.tokenCount_)
            return tokenCount_ < rhs.tokenCount_;

        for (size_t i = 0; i < tokenCount_; i++) {
            if (tokens_[i].index != rhs.tokens_[i].index)
                return tokens_[i].index < rhs.tokens_[i].index;

            if (tokens_[i].length != rhs.tokens_[i].length)
                return tokens_[i].length < rhs.tokens_[i].length;

            if (int cmp = std::memcmp(tokens_[i].name, rhs.tokens_[i].name, sizeof(Ch) * tokens_[i].length))
                return cmp < 0;
        }

        return false;
    }

    

    
    

    
    
    template<typename OutputStream>
    bool Stringify(OutputStream& os) const {
        return Stringify<false, OutputStream>(os);
    }

    
    
    template<typename OutputStream>
    bool StringifyUriFragment(OutputStream& os) const {
        return Stringify<true, OutputStream>(os);
    }

    

    
    

    
    
    ValueType& Create(ValueType& root, typename ValueType::AllocatorType& allocator, bool* alreadyExist = 0) const {
        CEREAL_RAPIDJSON_ASSERT(IsValid());
        ValueType* v = &root;
        bool exist = true;
        for (const Token *t = tokens_; t != tokens_ + tokenCount_; ++t) {
            if (v->IsArray() && t->name[0] == '-' && t->length == 1) {
                v->PushBack(ValueType().Move(), allocator);
                v = &((*v)[v->Size() - 1]);
                exist = false;
            }
            else {
                if (t->index == kPointerInvalidIndex) { 
                    if (!v->IsObject())
                        v->SetObject(); 
                }
                else { 
                    if (!v->IsArray() && !v->IsObject())
                        v->SetArray(); 
                }

                if (v->IsArray()) {
                    if (t->index >= v->Size()) {
                        v->Reserve(t->index + 1, allocator);
                        while (t->index >= v->Size())
                            v->PushBack(ValueType().Move(), allocator);
                        exist = false;
                    }
                    v = &((*v)[t->index]);
                }
                else {
                    typename ValueType::MemberIterator m = v->FindMember(GenericStringRef<Ch>(t->name, t->length));
                    if (m == v->MemberEnd()) {
                        v->AddMember(ValueType(t->name, t->length, allocator).Move(), ValueType().Move(), allocator);
                        v = &(--v->MemberEnd())->value; 
                        exist = false;
                    }
                    else
                        v = &m->value;
                }
            }
        }

        if (alreadyExist)
            *alreadyExist = exist;

        return *v;
    }

    
    
    template <typename stackAllocator>
    ValueType& Create(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, bool* alreadyExist = 0) const {
        return Create(document, document.GetAllocator(), alreadyExist);
    }

    

    
    

    
    
    ValueType* Get(ValueType& root, size_t* unresolvedTokenIndex = 0) const {
        CEREAL_RAPIDJSON_ASSERT(IsValid());
        ValueType* v = &root;
        for (const Token *t = tokens_; t != tokens_ + tokenCount_; ++t) {
            switch (v->GetType()) {
            case kObjectType:
                {
                    typename ValueType::MemberIterator m = v->FindMember(GenericStringRef<Ch>(t->name, t->length));
                    if (m == v->MemberEnd())
                        break;
                    v = &m->value;
                }
                continue;
            case kArrayType:
                if (t->index == kPointerInvalidIndex || t->index >= v->Size())
                    break;
                v = &((*v)[t->index]);
                continue;
            default:
                break;
            }

            
            if (unresolvedTokenIndex)
                *unresolvedTokenIndex = static_cast<size_t>(t - tokens_);
            return 0;
        }
        return v;
    }

    
    
    const ValueType* Get(const ValueType& root, size_t* unresolvedTokenIndex = 0) const { 
        return Get(const_cast<ValueType&>(root), unresolvedTokenIndex);
    }

    

    
    

    
    
    ValueType& GetWithDefault(ValueType& root, const ValueType& defaultValue, typename ValueType::AllocatorType& allocator) const {
        bool alreadyExist;
        ValueType& v = Create(root, allocator, &alreadyExist);
        return alreadyExist ? v : v.CopyFrom(defaultValue, allocator);
    }

    
    ValueType& GetWithDefault(ValueType& root, const Ch* defaultValue, typename ValueType::AllocatorType& allocator) const {
        bool alreadyExist;
        ValueType& v = Create(root, allocator, &alreadyExist);
        return alreadyExist ? v : v.SetString(defaultValue, allocator);
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    ValueType& GetWithDefault(ValueType& root, const std::basic_string<Ch>& defaultValue, typename ValueType::AllocatorType& allocator) const {
        bool alreadyExist;
        ValueType& v = Create(root, allocator, &alreadyExist);
        return alreadyExist ? v : v.SetString(defaultValue, allocator);
    }
#endif

    
    
    template <typename T>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&))
    GetWithDefault(ValueType& root, T defaultValue, typename ValueType::AllocatorType& allocator) const {
        return GetWithDefault(root, ValueType(defaultValue).Move(), allocator);
    }

    
    template <typename stackAllocator>
    ValueType& GetWithDefault(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, const ValueType& defaultValue) const {
        return GetWithDefault(document, defaultValue, document.GetAllocator());
    }

    
    template <typename stackAllocator>
    ValueType& GetWithDefault(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, const Ch* defaultValue) const {
        return GetWithDefault(document, defaultValue, document.GetAllocator());
    }
    
#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    template <typename stackAllocator>
    ValueType& GetWithDefault(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, const std::basic_string<Ch>& defaultValue) const {
        return GetWithDefault(document, defaultValue, document.GetAllocator());
    }
#endif

    
    
    template <typename T, typename stackAllocator>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&))
    GetWithDefault(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, T defaultValue) const {
        return GetWithDefault(document, defaultValue, document.GetAllocator());
    }

    

    
    

    
    
    ValueType& Set(ValueType& root, ValueType& value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator) = value;
    }

    
    ValueType& Set(ValueType& root, const ValueType& value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator).CopyFrom(value, allocator);
    }

    
    ValueType& Set(ValueType& root, const Ch* value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator) = ValueType(value, allocator).Move();
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    ValueType& Set(ValueType& root, const std::basic_string<Ch>& value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator) = ValueType(value, allocator).Move();
    }
#endif

    
    
    template <typename T>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&))
    Set(ValueType& root, T value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator) = ValueType(value).Move();
    }

    
    template <typename stackAllocator>
    ValueType& Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, ValueType& value) const {
        return Create(document) = value;
    }

    
    template <typename stackAllocator>
    ValueType& Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, const ValueType& value) const {
        return Create(document).CopyFrom(value, document.GetAllocator());
    }

    
    template <typename stackAllocator>
    ValueType& Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, const Ch* value) const {
        return Create(document) = ValueType(value, document.GetAllocator()).Move();
    }

#if CEREAL_RAPIDJSON_HAS_STDSTRING
    
    template <typename stackAllocator>
    ValueType& Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, const std::basic_string<Ch>& value) const {
        return Create(document) = ValueType(value, document.GetAllocator()).Move();
    }
#endif

    
    
    template <typename T, typename stackAllocator>
    CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T>, internal::IsGenericValue<T> >), (ValueType&))
        Set(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, T value) const {
            return Create(document) = value;
    }

    

    
    

    
    
    ValueType& Swap(ValueType& root, ValueType& value, typename ValueType::AllocatorType& allocator) const {
        return Create(root, allocator).Swap(value);
    }

    
    template <typename stackAllocator>
    ValueType& Swap(GenericDocument<EncodingType, typename ValueType::AllocatorType, stackAllocator>& document, ValueType& value) const {
        return Create(document).Swap(value);
    }

    

    
    
    bool Erase(ValueType& root) const {
        CEREAL_RAPIDJSON_ASSERT(IsValid());
        if (tokenCount_ == 0) 
            return false;

        ValueType* v = &root;
        const Token* last = tokens_ + (tokenCount_ - 1);
        for (const Token *t = tokens_; t != last; ++t) {
            switch (v->GetType()) {
            case kObjectType:
                {
                    typename ValueType::MemberIterator m = v->FindMember(GenericStringRef<Ch>(t->name, t->length));
                    if (m == v->MemberEnd())
                        return false;
                    v = &m->value;
                }
                break;
            case kArrayType:
                if (t->index == kPointerInvalidIndex || t->index >= v->Size())
                    return false;
                v = &((*v)[t->index]);
                break;
            default:
                return false;
            }
        }

        switch (v->GetType()) {
        case kObjectType:
            return v->EraseMember(GenericStringRef<Ch>(last->name, last->length));
        case kArrayType:
            if (last->index == kPointerInvalidIndex || last->index >= v->Size())
                return false;
            v->Erase(v->Begin() + last->index);
            return true;
        default:
            return false;
        }
    }

private:
    
    
    Ch* CopyFromRaw(const GenericPointer& rhs, size_t extraToken = 0, size_t extraNameBufferSize = 0) {
        if (!allocator_) 
            ownAllocator_ = allocator_ = CEREAL_RAPIDJSON_NEW(Allocator)();

        size_t nameBufferSize = rhs.tokenCount_; 
        for (Token *t = rhs.tokens_; t != rhs.tokens_ + rhs.tokenCount_; ++t)
            nameBufferSize += t->length;

        tokenCount_ = rhs.tokenCount_ + extraToken;
        tokens_ = static_cast<Token *>(allocator_->Malloc(tokenCount_ * sizeof(Token) + (nameBufferSize + extraNameBufferSize) * sizeof(Ch)));
        nameBuffer_ = reinterpret_cast<Ch *>(tokens_ + tokenCount_);
        if (rhs.tokenCount_ > 0) {
            std::memcpy(tokens_, rhs.tokens_, rhs.tokenCount_ * sizeof(Token));
        }
        if (nameBufferSize > 0) {
            std::memcpy(nameBuffer_, rhs.nameBuffer_, nameBufferSize * sizeof(Ch));
        }

        
        std::ptrdiff_t diff = nameBuffer_ - rhs.nameBuffer_;
        for (Token *t = tokens_; t != tokens_ + rhs.tokenCount_; ++t)
            t->name += diff;

        return nameBuffer_ + nameBufferSize;
    }

    
    
    bool NeedPercentEncode(Ch c) const {
        return !((c >= '0' && c <= '9') || (c >= 'A' && c <='Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '.' || c == '_' || c =='~');
    }

    
#ifndef __clang__ 
    
#endif
    void Parse(const Ch* source, size_t length) {
        CEREAL_RAPIDJSON_ASSERT(source != NULL);
        CEREAL_RAPIDJSON_ASSERT(nameBuffer_ == 0);
        CEREAL_RAPIDJSON_ASSERT(tokens_ == 0);

        
        if (!allocator_)
            ownAllocator_ = allocator_ = CEREAL_RAPIDJSON_NEW(Allocator)();

        
        tokenCount_ = 0;
        for (const Ch* s = source; s != source + length; s++) 
            if (*s == '/')
                tokenCount_++;

        Token* token = tokens_ = static_cast<Token *>(allocator_->Malloc(tokenCount_ * sizeof(Token) + length * sizeof(Ch)));
        Ch* name = nameBuffer_ = reinterpret_cast<Ch *>(tokens_ + tokenCount_);
        size_t i = 0;

        
        bool uriFragment = false;
        if (source[i] == '#') {
            uriFragment = true;
            i++;
        }

        if (i != length && source[i] != '/') {
            parseErrorCode_ = kPointerParseErrorTokenMustBeginWithSolidus;
            goto error;
        }

        while (i < length) {
            CEREAL_RAPIDJSON_ASSERT(source[i] == '/');
            i++; 

            token->name = name;
            bool isNumber = true;

            while (i < length && source[i] != '/') {
                Ch c = source[i];
                if (uriFragment) {
                    
                    if (c == '%') {
                        PercentDecodeStream is(&source[i], source + length);
                        GenericInsituStringStream<EncodingType> os(name);
                        Ch* begin = os.PutBegin();
                        if (!Transcoder<UTF8<>, EncodingType>().Validate(is, os) || !is.IsValid()) {
                            parseErrorCode_ = kPointerParseErrorInvalidPercentEncoding;
                            goto error;
                        }
                        size_t len = os.PutEnd(begin);
                        i += is.Tell() - 1;
                        if (len == 1)
                            c = *name;
                        else {
                            name += len;
                            isNumber = false;
                            i++;
                            continue;
                        }
                    }
                    else if (NeedPercentEncode(c)) {
                        parseErrorCode_ = kPointerParseErrorCharacterMustPercentEncode;
                        goto error;
                    }
                }

                i++;
                
                
                if (c == '~') {
                    if (i < length) {
                        c = source[i];
                        if (c == '0')       c = '~';
                        else if (c == '1')  c = '/';
                        else {
                            parseErrorCode_ = kPointerParseErrorInvalidEscape;
                            goto error;
                        }
                        i++;
                    }
                    else {
                        parseErrorCode_ = kPointerParseErrorInvalidEscape;
                        goto error;
                    }
                }

                
                if (c < '0' || c > '9')
                    isNumber = false;

                *name++ = c;
            }
            token->length = static_cast<SizeType>(name - token->name);
            if (token->length == 0)
                isNumber = false;
            *name++ = '\0'; 

            
            if (isNumber && token->length > 1 && token->name[0] == '0')
                isNumber = false;

            
            SizeType n = 0;
            if (isNumber) {
                for (size_t j = 0; j < token->length; j++) {
                    SizeType m = n * 10 + static_cast<SizeType>(token->name[j] - '0');
                    if (m < n) {   
                        isNumber = false;
                        break;
                    }
                    n = m;
                }
            }

            token->index = isNumber ? n : kPointerInvalidIndex;
            token++;
        }

        CEREAL_RAPIDJSON_ASSERT(name <= nameBuffer_ + length); 
        parseErrorCode_ = kPointerParseErrorNone;
        return;

    error:
        Allocator::Free(tokens_);
        nameBuffer_ = 0;
        tokens_ = 0;
        tokenCount_ = 0;
        parseErrorOffset_ = i;
        return;
    }

    
    
    template<bool uriFragment, typename OutputStream>
    bool Stringify(OutputStream& os) const {
        CEREAL_RAPIDJSON_ASSERT(IsValid());

        if (uriFragment)
            os.Put('#');

        for (Token *t = tokens_; t != tokens_ + tokenCount_; ++t) {
            os.Put('/');
            for (size_t j = 0; j < t->length; j++) {
                Ch c = t->name[j];
                if (c == '~') {
                    os.Put('~');
                    os.Put('0');
                }
                else if (c == '/') {
                    os.Put('~');
                    os.Put('1');
                }
                else if (uriFragment && NeedPercentEncode(c)) { 
                    
                    GenericStringStream<typename ValueType::EncodingType> source(&t->name[j]);
                    PercentEncodeStream<OutputStream> target(os);
                    if (!Transcoder<EncodingType, UTF8<> >().Validate(source, target))
                        return false;
                    j += source.Tell() - 1;
                }
                else
                    os.Put(c);
            }
        }
        return true;
    }

    
    
    class PercentDecodeStream {
    public:
        typedef typename ValueType::Ch Ch;

        
        
        PercentDecodeStream(const Ch* source, const Ch* end) : src_(source), head_(source), end_(end), valid_(true) {}

        Ch Take() {
            if (*src_ != '%' || src_ + 3 > end_) { 
                valid_ = false;
                return 0;
            }
            src_++;
            Ch c = 0;
            for (int j = 0; j < 2; j++) {
                c = static_cast<Ch>(c << 4);
                Ch h = *src_;
                if      (h >= '0' && h <= '9') c = static_cast<Ch>(c + h - '0');
                else if (h >= 'A' && h <= 'F') c = static_cast<Ch>(c + h - 'A' + 10);
                else if (h >= 'a' && h <= 'f') c = static_cast<Ch>(c + h - 'a' + 10);
                else {
                    valid_ = false;
                    return 0;
                }
                src_++;
            }
            return c;
        }

        size_t Tell() const { return static_cast<size_t>(src_ - head_); }
        bool IsValid() const { return valid_; }

    private:
        const Ch* src_;     
        const Ch* head_;    
        const Ch* end_;     
        bool valid_;        
    };

    
    template <typename OutputStream>
    class PercentEncodeStream {
    public:
        PercentEncodeStream(OutputStream& os) : os_(os) {}
        void Put(char c) { 
            unsigned char u = static_cast<unsigned char>(c);
            static const char hexDigits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            os_.Put('%');
            os_.Put(static_cast<typename OutputStream::Ch>(hexDigits[u >> 4]));
            os_.Put(static_cast<typename OutputStream::Ch>(hexDigits[u & 15]));
        }
    private:
        OutputStream& os_;
    };

    Allocator* allocator_;                  
    Allocator* ownAllocator_;               
    Ch* nameBuffer_;                        
    Token* tokens_;                         
    size_t tokenCount_;                     
    size_t parseErrorOffset_;               
    PointerParseErrorCode parseErrorCode_;  
};


typedef GenericPointer<Value> Pointer;






template <typename T>
typename T::ValueType& CreateValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::AllocatorType& a) {
    return pointer.Create(root, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& CreateValueByPointer(T& root, const CharType(&source)[N], typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Create(root, a);
}



template <typename DocumentType>
typename DocumentType::ValueType& CreateValueByPointer(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer) {
    return pointer.Create(document);
}

template <typename DocumentType, typename CharType, size_t N>
typename DocumentType::ValueType& CreateValueByPointer(DocumentType& document, const CharType(&source)[N]) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).Create(document);
}



template <typename T>
typename T::ValueType* GetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, size_t* unresolvedTokenIndex = 0) {
    return pointer.Get(root, unresolvedTokenIndex);
}

template <typename T>
const typename T::ValueType* GetValueByPointer(const T& root, const GenericPointer<typename T::ValueType>& pointer, size_t* unresolvedTokenIndex = 0) {
    return pointer.Get(root, unresolvedTokenIndex);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType* GetValueByPointer(T& root, const CharType (&source)[N], size_t* unresolvedTokenIndex = 0) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Get(root, unresolvedTokenIndex);
}

template <typename T, typename CharType, size_t N>
const typename T::ValueType* GetValueByPointer(const T& root, const CharType(&source)[N], size_t* unresolvedTokenIndex = 0) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Get(root, unresolvedTokenIndex);
}



template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::ValueType& defaultValue, typename T::AllocatorType& a) {
    return pointer.GetWithDefault(root, defaultValue, a);
}

template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::Ch* defaultValue, typename T::AllocatorType& a) {
    return pointer.GetWithDefault(root, defaultValue, a);
}

#if CEREAL_RAPIDJSON_HAS_STDSTRING
template <typename T>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, const std::basic_string<typename T::Ch>& defaultValue, typename T::AllocatorType& a) {
    return pointer.GetWithDefault(root, defaultValue, a);
}
#endif

template <typename T, typename T2>
CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
GetValueByPointerWithDefault(T& root, const GenericPointer<typename T::ValueType>& pointer, T2 defaultValue, typename T::AllocatorType& a) {
    return pointer.GetWithDefault(root, defaultValue, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], const typename T::ValueType& defaultValue, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).GetWithDefault(root, defaultValue, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], const typename T::Ch* defaultValue, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).GetWithDefault(root, defaultValue, a);
}

#if CEREAL_RAPIDJSON_HAS_STDSTRING
template <typename T, typename CharType, size_t N>
typename T::ValueType& GetValueByPointerWithDefault(T& root, const CharType(&source)[N], const std::basic_string<typename T::Ch>& defaultValue, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).GetWithDefault(root, defaultValue, a);
}
#endif

template <typename T, typename CharType, size_t N, typename T2>
CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
GetValueByPointerWithDefault(T& root, const CharType(&source)[N], T2 defaultValue, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).GetWithDefault(root, defaultValue, a);
}



template <typename DocumentType>
typename DocumentType::ValueType& GetValueByPointerWithDefault(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, const typename DocumentType::ValueType& defaultValue) {
    return pointer.GetWithDefault(document, defaultValue);
}

template <typename DocumentType>
typename DocumentType::ValueType& GetValueByPointerWithDefault(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, const typename DocumentType::Ch* defaultValue) {
    return pointer.GetWithDefault(document, defaultValue);
}

#if CEREAL_RAPIDJSON_HAS_STDSTRING
template <typename DocumentType>
typename DocumentType::ValueType& GetValueByPointerWithDefault(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, const std::basic_string<typename DocumentType::Ch>& defaultValue) {
    return pointer.GetWithDefault(document, defaultValue);
}
#endif

template <typename DocumentType, typename T2>
CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename DocumentType::ValueType&))
GetValueByPointerWithDefault(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, T2 defaultValue) {
    return pointer.GetWithDefault(document, defaultValue);
}

template <typename DocumentType, typename CharType, size_t N>
typename DocumentType::ValueType& GetValueByPointerWithDefault(DocumentType& document, const CharType(&source)[N], const typename DocumentType::ValueType& defaultValue) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).GetWithDefault(document, defaultValue);
}

template <typename DocumentType, typename CharType, size_t N>
typename DocumentType::ValueType& GetValueByPointerWithDefault(DocumentType& document, const CharType(&source)[N], const typename DocumentType::Ch* defaultValue) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).GetWithDefault(document, defaultValue);
}

#if CEREAL_RAPIDJSON_HAS_STDSTRING
template <typename DocumentType, typename CharType, size_t N>
typename DocumentType::ValueType& GetValueByPointerWithDefault(DocumentType& document, const CharType(&source)[N], const std::basic_string<typename DocumentType::Ch>& defaultValue) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).GetWithDefault(document, defaultValue);
}
#endif

template <typename DocumentType, typename CharType, size_t N, typename T2>
CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename DocumentType::ValueType&))
GetValueByPointerWithDefault(DocumentType& document, const CharType(&source)[N], T2 defaultValue) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).GetWithDefault(document, defaultValue);
}



template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::ValueType& value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}

template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::ValueType& value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}

template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, const typename T::Ch* value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}

#if CEREAL_RAPIDJSON_HAS_STDSTRING
template <typename T>
typename T::ValueType& SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, const std::basic_string<typename T::Ch>& value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}
#endif

template <typename T, typename T2>
CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
SetValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, T2 value, typename T::AllocatorType& a) {
    return pointer.Set(root, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], typename T::ValueType& value, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Set(root, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], const typename T::ValueType& value, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Set(root, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], const typename T::Ch* value, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Set(root, value, a);
}

#if CEREAL_RAPIDJSON_HAS_STDSTRING
template <typename T, typename CharType, size_t N>
typename T::ValueType& SetValueByPointer(T& root, const CharType(&source)[N], const std::basic_string<typename T::Ch>& value, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Set(root, value, a);
}
#endif

template <typename T, typename CharType, size_t N, typename T2>
CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename T::ValueType&))
SetValueByPointer(T& root, const CharType(&source)[N], T2 value, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Set(root, value, a);
}



template <typename DocumentType>
typename DocumentType::ValueType& SetValueByPointer(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, typename DocumentType::ValueType& value) {
    return pointer.Set(document, value);
}

template <typename DocumentType>
typename DocumentType::ValueType& SetValueByPointer(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, const typename DocumentType::ValueType& value) {
    return pointer.Set(document, value);
}

template <typename DocumentType>
typename DocumentType::ValueType& SetValueByPointer(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, const typename DocumentType::Ch* value) {
    return pointer.Set(document, value);
}

#if CEREAL_RAPIDJSON_HAS_STDSTRING
template <typename DocumentType>
typename DocumentType::ValueType& SetValueByPointer(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, const std::basic_string<typename DocumentType::Ch>& value) {
    return pointer.Set(document, value);
}
#endif

template <typename DocumentType, typename T2>
CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename DocumentType::ValueType&))
SetValueByPointer(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, T2 value) {
    return pointer.Set(document, value);
}

template <typename DocumentType, typename CharType, size_t N>
typename DocumentType::ValueType& SetValueByPointer(DocumentType& document, const CharType(&source)[N], typename DocumentType::ValueType& value) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).Set(document, value);
}

template <typename DocumentType, typename CharType, size_t N>
typename DocumentType::ValueType& SetValueByPointer(DocumentType& document, const CharType(&source)[N], const typename DocumentType::ValueType& value) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).Set(document, value);
}

template <typename DocumentType, typename CharType, size_t N>
typename DocumentType::ValueType& SetValueByPointer(DocumentType& document, const CharType(&source)[N], const typename DocumentType::Ch* value) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).Set(document, value);
}

#if CEREAL_RAPIDJSON_HAS_STDSTRING
template <typename DocumentType, typename CharType, size_t N>
typename DocumentType::ValueType& SetValueByPointer(DocumentType& document, const CharType(&source)[N], const std::basic_string<typename DocumentType::Ch>& value) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).Set(document, value);
}
#endif

template <typename DocumentType, typename CharType, size_t N, typename T2>
CEREAL_RAPIDJSON_DISABLEIF_RETURN((internal::OrExpr<internal::IsPointer<T2>, internal::IsGenericValue<T2> >), (typename DocumentType::ValueType&))
SetValueByPointer(DocumentType& document, const CharType(&source)[N], T2 value) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).Set(document, value);
}



template <typename T>
typename T::ValueType& SwapValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer, typename T::ValueType& value, typename T::AllocatorType& a) {
    return pointer.Swap(root, value, a);
}

template <typename T, typename CharType, size_t N>
typename T::ValueType& SwapValueByPointer(T& root, const CharType(&source)[N], typename T::ValueType& value, typename T::AllocatorType& a) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Swap(root, value, a);
}

template <typename DocumentType>
typename DocumentType::ValueType& SwapValueByPointer(DocumentType& document, const GenericPointer<typename DocumentType::ValueType>& pointer, typename DocumentType::ValueType& value) {
    return pointer.Swap(document, value);
}

template <typename DocumentType, typename CharType, size_t N>
typename DocumentType::ValueType& SwapValueByPointer(DocumentType& document, const CharType(&source)[N], typename DocumentType::ValueType& value) {
    return GenericPointer<typename DocumentType::ValueType>(source, N - 1).Swap(document, value);
}



template <typename T>
bool EraseValueByPointer(T& root, const GenericPointer<typename T::ValueType>& pointer) {
    return pointer.Erase(root);
}

template <typename T, typename CharType, size_t N>
bool EraseValueByPointer(T& root, const CharType(&source)[N]) {
    return GenericPointer<typename T::ValueType>(source, N - 1).Erase(root);
}



CEREAL_RAPIDJSON_NAMESPACE_END

#if defined(__clang__) || defined(_MSC_VER)
CEREAL_RAPIDJSON_DIAG_POP
#endif

#endif 
