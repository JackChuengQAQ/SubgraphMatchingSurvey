













#ifndef CEREAL_RAPIDJSON_FWD_H_
#define CEREAL_RAPIDJSON_FWD_H_

#include "rapidjson.h"

CEREAL_RAPIDJSON_NAMESPACE_BEGIN



template<typename CharType> struct UTF8;
template<typename CharType> struct UTF16;
template<typename CharType> struct UTF16BE;
template<typename CharType> struct UTF16LE;
template<typename CharType> struct UTF32;
template<typename CharType> struct UTF32BE;
template<typename CharType> struct UTF32LE;
template<typename CharType> struct ASCII;
template<typename CharType> struct AutoUTF;

template<typename SourceEncoding, typename TargetEncoding>
struct Transcoder;



class CrtAllocator;

template <typename BaseAllocator>
class MemoryPoolAllocator;



template <typename Encoding>
struct GenericStringStream;

typedef GenericStringStream<UTF8<char> > StringStream;

template <typename Encoding>
struct GenericInsituStringStream;

typedef GenericInsituStringStream<UTF8<char> > InsituStringStream;



template <typename Encoding, typename Allocator>
class GenericStringBuffer;

typedef GenericStringBuffer<UTF8<char>, CrtAllocator> StringBuffer;



class FileReadStream;



class FileWriteStream;



template <typename Allocator>
struct GenericMemoryBuffer;

typedef GenericMemoryBuffer<CrtAllocator> MemoryBuffer;



struct MemoryStream;



template<typename Encoding, typename Derived>
struct BaseReaderHandler;

template <typename SourceEncoding, typename TargetEncoding, typename StackAllocator>
class GenericReader;

typedef GenericReader<UTF8<char>, UTF8<char>, CrtAllocator> Reader;



template<typename OutputStream, typename SourceEncoding, typename TargetEncoding, typename StackAllocator, unsigned writeFlags>
class Writer;



template<typename OutputStream, typename SourceEncoding, typename TargetEncoding, typename StackAllocator, unsigned writeFlags>
class PrettyWriter;



template <typename Encoding, typename Allocator> 
struct GenericMember;

template <bool Const, typename Encoding, typename Allocator>
class GenericMemberIterator;

template<typename CharType>
struct GenericStringRef;

template <typename Encoding, typename Allocator> 
class GenericValue;

typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator> > Value;

template <typename Encoding, typename Allocator, typename StackAllocator>
class GenericDocument;

typedef GenericDocument<UTF8<char>, MemoryPoolAllocator<CrtAllocator>, CrtAllocator> Document;



template <typename ValueType, typename Allocator>
class GenericPointer;

typedef GenericPointer<Value, CrtAllocator> Pointer;



template <typename SchemaDocumentType>
class IGenericRemoteSchemaDocumentProvider;

template <typename ValueT, typename Allocator>
class GenericSchemaDocument;

typedef GenericSchemaDocument<Value, CrtAllocator> SchemaDocument;
typedef IGenericRemoteSchemaDocumentProvider<SchemaDocument> IRemoteSchemaDocumentProvider;

template <
    typename SchemaDocumentType,
    typename OutputHandler,
    typename StateAllocator>
class GenericSchemaValidator;

typedef GenericSchemaValidator<SchemaDocument, BaseReaderHandler<UTF8<char>, void>, CrtAllocator> SchemaValidator;

CEREAL_RAPIDJSON_NAMESPACE_END

#endif 
