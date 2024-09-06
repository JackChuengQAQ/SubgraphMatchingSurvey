













#ifndef CEREAL_RAPIDJSON_ALLOCATORS_H_
#define CEREAL_RAPIDJSON_ALLOCATORS_H_

#include "rapidjson.h"

CEREAL_RAPIDJSON_NAMESPACE_BEGIN









#ifndef CEREAL_RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY
#define CEREAL_RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY (64 * 1024)
#endif







class CrtAllocator {
public:
    static const bool kNeedFree = true;
    void* Malloc(size_t size) { 
        if (size) 
            return std::malloc(size);
        else
            return NULL; 
    }
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
        (void)originalSize;
        if (newSize == 0) {
            std::free(originalPtr);
            return NULL;
        }
        return std::realloc(originalPtr, newSize);
    }
    static void Free(void *ptr) { std::free(ptr); }
};






template <typename BaseAllocator = CrtAllocator>
class MemoryPoolAllocator {
public:
    static const bool kNeedFree = false;    

    
    
    MemoryPoolAllocator(size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0) : 
        chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(0), baseAllocator_(baseAllocator), ownBaseAllocator_(0)
    {
    }

    
    
    MemoryPoolAllocator(void *buffer, size_t size, size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0) :
        chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(buffer), baseAllocator_(baseAllocator), ownBaseAllocator_(0)
    {
        CEREAL_RAPIDJSON_ASSERT(buffer != 0);
        CEREAL_RAPIDJSON_ASSERT(size > sizeof(ChunkHeader));
        chunkHead_ = reinterpret_cast<ChunkHeader*>(buffer);
        chunkHead_->capacity = size - sizeof(ChunkHeader);
        chunkHead_->size = 0;
        chunkHead_->next = 0;
    }

    
    
    ~MemoryPoolAllocator() {
        Clear();
        CEREAL_RAPIDJSON_DELETE(ownBaseAllocator_);
    }

    
    void Clear() {
        while (chunkHead_ && chunkHead_ != userBuffer_) {
            ChunkHeader* next = chunkHead_->next;
            baseAllocator_->Free(chunkHead_);
            chunkHead_ = next;
        }
        if (chunkHead_ && chunkHead_ == userBuffer_)
            chunkHead_->size = 0; 
    }

    
    
    size_t Capacity() const {
        size_t capacity = 0;
        for (ChunkHeader* c = chunkHead_; c != 0; c = c->next)
            capacity += c->capacity;
        return capacity;
    }

    
    
    size_t Size() const {
        size_t size = 0;
        for (ChunkHeader* c = chunkHead_; c != 0; c = c->next)
            size += c->size;
        return size;
    }

    
    void* Malloc(size_t size) {
        if (!size)
            return NULL;

        size = CEREAL_RAPIDJSON_ALIGN(size);
        if (chunkHead_ == 0 || chunkHead_->size + size > chunkHead_->capacity)
            if (!AddChunk(chunk_capacity_ > size ? chunk_capacity_ : size))
                return NULL;

        void *buffer = reinterpret_cast<char *>(chunkHead_) + CEREAL_RAPIDJSON_ALIGN(sizeof(ChunkHeader)) + chunkHead_->size;
        chunkHead_->size += size;
        return buffer;
    }

    
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) {
        if (originalPtr == 0)
            return Malloc(newSize);

        if (newSize == 0)
            return NULL;

        originalSize = CEREAL_RAPIDJSON_ALIGN(originalSize);
        newSize = CEREAL_RAPIDJSON_ALIGN(newSize);

        
        if (originalSize >= newSize)
            return originalPtr;

        
        if (originalPtr == reinterpret_cast<char *>(chunkHead_) + CEREAL_RAPIDJSON_ALIGN(sizeof(ChunkHeader)) + chunkHead_->size - originalSize) {
            size_t increment = static_cast<size_t>(newSize - originalSize);
            if (chunkHead_->size + increment <= chunkHead_->capacity) {
                chunkHead_->size += increment;
                return originalPtr;
            }
        }

        
        if (void* newBuffer = Malloc(newSize)) {
            if (originalSize)
                std::memcpy(newBuffer, originalPtr, originalSize);
            return newBuffer;
        }
        else
            return NULL;
    }

    
    static void Free(void *ptr) { (void)ptr; } 

private:
    
    MemoryPoolAllocator(const MemoryPoolAllocator& rhs) ;
    
    MemoryPoolAllocator& operator=(const MemoryPoolAllocator& rhs) ;

    
    
    bool AddChunk(size_t capacity) {
        if (!baseAllocator_)
            ownBaseAllocator_ = baseAllocator_ = CEREAL_RAPIDJSON_NEW(BaseAllocator)();
        if (ChunkHeader* chunk = reinterpret_cast<ChunkHeader*>(baseAllocator_->Malloc(CEREAL_RAPIDJSON_ALIGN(sizeof(ChunkHeader)) + capacity))) {
            chunk->capacity = capacity;
            chunk->size = 0;
            chunk->next = chunkHead_;
            chunkHead_ =  chunk;
            return true;
        }
        else
            return false;
    }

    static const int kDefaultChunkCapacity = CEREAL_RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY; 

    
    
    struct ChunkHeader {
        size_t capacity;    
        size_t size;        
        ChunkHeader *next;  
    };

    ChunkHeader *chunkHead_;    
    size_t chunk_capacity_;     
    void *userBuffer_;          
    BaseAllocator* baseAllocator_;  
    BaseAllocator* ownBaseAllocator_;   
};

CEREAL_RAPIDJSON_NAMESPACE_END

#endif 
