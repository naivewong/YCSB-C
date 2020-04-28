#ifndef YCSB_C_LIB_CMAP_H_
#define YCSB_C_LIB_CMAP_H_

#include <unordered_map>

#include "lib/mutexlock.h"

template<typename K, typename V>
class ConcurrentUnorderedMap {
private:
	typedef std::unordered_map<K, V> Hashtable;
	Hashtable table_;
	mutable RWMutex rwlock_;
public:
	V get(const K& key) const {
		ReadLock lock(&rwlock_);
		typename Hashtable::const_iterator pos = table_.find(key);
		if (pos == table_.end()) return NULL;
		else return pos->second;
	}
	
	bool insert(const K& key, const V& value) {
		WriteLock lock(&rwlock_);
		if (!key) return false;
	  return table_.insert(std::make_pair(key, value)).second;
	}
	
	V update(const K& key, const V& value) {
		WriteLock lock(&rwlock_);
		typename Hashtable::iterator pos = table_.find(key);
		if (pos == table_.end()) return NULL;
		V old = pos->second;
		pos->second = value;
		return old;
	}
	
	V remove(const K& key) {
		WriteLock lock(&rwlock_);
		typename Hashtable::const_iterator pos = table_.find(key);
		if (pos == table_.end()) return NULL;
		V old = pos->second;
		table_.erase(pos);
		return old;
	}
	
	void clear() {
		WriteLock lock(&rwlock_);
		table_.clear();
	}
	
	size_t size() {
		ReadLock lock(&rwlock_);
		return table_.size();
	}

	bool empty() {
		ReadLock lock(&rwlock_);
		return table_.empty();
	}
};

#endif