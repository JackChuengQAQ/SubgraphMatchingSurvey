



#ifndef HASHJOINS_HASH_HELPERS_H
#define HASHJOINS_HASH_HELPERS_H

#include <memory>
#include <tuple>
#include <mutex>
#include <vector>

#define TUPLE_COUNT_PER_OVERFLOW_BUCKET 4

inline uint64_t murmur3(uint64_t val) {
        
        
        val ^= val >> 33;
        val *= 0xff51afd7ed558ccd;
        val ^= val >> 33;
        val *= 0xc4ceb9fe1a85ec53;
        val ^= val >> 33;
        return val;
}

inline uint32_t murmur3(uint32_t val) {
    val ^= val >> 16;
    val *= 0x85ebca6b;
    val ^= val >> 13;
    val *= 0xc2b2ae35;
    val ^= val >> 16;
    return val;
}



typedef std::tuple<uint64_t, uint64_t> my_tuple;


struct overflow {
    my_tuple tuples[TUPLE_COUNT_PER_OVERFLOW_BUCKET];
    std::unique_ptr<overflow> next;
    explicit overflow(my_tuple t) { tuples[0] = t; }
};


struct hash_table{
    
    struct bucket{
        uint32_t count;
        my_tuple t1;
        my_tuple t2;
        std::unique_ptr<overflow> next;

        
        bucket(): count(0), next(nullptr) {}
    };

    std::unique_ptr<bucket[]> arr;
    uint64_t size;

    explicit hash_table(uint64_t size): size(size){
        arr = std::make_unique<bucket[]>(size);
    }
};

#endif  
