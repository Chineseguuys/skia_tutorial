#ifndef _HASH_COMBINE_H_
#define _HASH_COMBINE_H_

#include <functional>

static inline void hashCombineSingleHashed(size_t& combinedHash, size_t hash) {
    combinedHash = 31 * combinedHash + hash;
}

template<typename T>
static inline void hashCombineSingle(size_t& combinedHash, const T& val) {
    hashCombineSingleHashed(combinedHash, std::hash<T>{}(val));
}

template<typename... Types>
static inline size_t hashCombine(const Types& ... args) {
    size_t hash = 0;
    ( hashCombineSingle(hash, args), ... );
    return hash;
}

#endif // end _HASH_COMBINE_H_