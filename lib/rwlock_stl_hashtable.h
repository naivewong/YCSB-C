//
//  rwlock_stl_hashtable.h
//
//  Created by Alec on 04/21/20.
//

#ifndef YCSB_C_LIB_LOCK_STL_HASHTABLE_H_
#define YCSB_C_LIB_LOCK_STL_HASHTABLE_H_

#include "lib/mutexlock.h"
#include "lib/stl_hashtable.h"

#include <vector>

namespace vmp {

template<class V>
class RWLockStlHashtable : public StlHashtable<V> {
 public:
  typedef typename StringHashtable<V>::KVPair KVPair;

  V Get(const char *key) const; ///< Returns NULL if the key is not found
  bool Insert(const char *key, V value);
  V Update(const char *key, V value);
  V Remove(const char *key);
  std::vector<KVPair> Entries(const char *key = NULL, size_t n = -1) const;
  std::size_t Size() const;

 private:
  mutable RWMutex rwlock_;
};

template<class V>
inline V RWLockStlHashtable<V>::Get(const char *key) const {
  ReadLock lock(&rwlock_);
  return StlHashtable<V>::Get(key);
}

template<class V>
inline bool RWLockStlHashtable<V>::Insert(const char *key, V value) {
  WriteLock lock(&rwlock_);
  return StlHashtable<V>::Insert(key, value);
}

template<class V>
inline V RWLockStlHashtable<V>::Update(const char *key, V value) {
  WriteLock lock(&rwlock_);
  return StlHashtable<V>::Update(key, value);
}

template<class V>
inline V RWLockStlHashtable<V>::Remove(const char *key) {
  WriteLock lock(&rwlock_);
  return StlHashtable<V>::Remove(key);
}

template<class V>
inline std::size_t RWLockStlHashtable<V>::Size() const {
  ReadLock lock(&rwlock_);
  return StlHashtable<V>::Size();
}

template<class V>
inline std::vector<typename RWLockStlHashtable<V>::KVPair>
RWLockStlHashtable<V>::Entries(const char *key, size_t n) const {
  ReadLock lock(&rwlock_);
  return StlHashtable<V>::Entries(key, n);
}

} // vmp

#endif // YCSB_C_LIB_LOCK_STL_HASHTABLE_H_

