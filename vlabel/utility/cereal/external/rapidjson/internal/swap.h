













#ifndef CEREAL_RAPIDJSON_INTERNAL_SWAP_H_
#define CEREAL_RAPIDJSON_INTERNAL_SWAP_H_

#include "../rapidjson.h"

#if defined(__clang__)
CEREAL_RAPIDJSON_DIAG_PUSH
CEREAL_RAPIDJSON_DIAG_OFF(c++98-compat)
#endif

CEREAL_RAPIDJSON_NAMESPACE_BEGIN
namespace internal {



template <typename T>
inline void Swap(T& a, T& b) CEREAL_RAPIDJSON_NOEXCEPT {
    T tmp = a;
        a = b;
        b = tmp;
}

} 
CEREAL_RAPIDJSON_NAMESPACE_END

#if defined(__clang__)
CEREAL_RAPIDJSON_DIAG_POP
#endif

#endif 
