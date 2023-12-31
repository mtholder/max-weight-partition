#include "Cache.h"
#include "LRUCache.h"
// #include "LFUCache.h"

Cache::Cache(int size) {
}

// MTH simplified to only LRUCache
CachePtr Cache::getCache(CacheType c, size_t size) {
	return make_shared<LRUCache>(size);
}

Cache::~Cache() {
}
