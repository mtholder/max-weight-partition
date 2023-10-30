#pragma once
#include <string>
#include <memory>
#include <set>
constexpr auto MAX_CACHE_SIZE = 4096;
using namespace std;

class ConnectedComponent;

// MTH simplified to only LRUCache
enum CacheType {
	LRU_CACHE
};

struct CacheData {
	set<size_t> key;
	shared_ptr<ConnectedComponent> value; // mth modified
	int ttl;
	CacheData(set<size_t> k, shared_ptr<ConnectedComponent> v, int t)
		:key(k),
		value(v),
		ttl(t) {
	}
};
typedef shared_ptr<CacheData> CacheDataPtr;

class comparator {
public:
	bool operator()(const CacheDataPtr l, const CacheDataPtr r) {
		return l->ttl > r->ttl;
	}
};

class Cache;
typedef shared_ptr<Cache> CachePtr;
class Cache {
public:
	Cache(int size = MAX_CACHE_SIZE);
	virtual shared_ptr<ConnectedComponent> get(const set<size_t> & key) = 0;
	virtual void put(const set<size_t> &  key, shared_ptr<ConnectedComponent> value, int ttl = -1) = 0;
	virtual void deleteKey(const set<size_t> &  key) = 0;
	virtual void clear() = 0;
	static CachePtr getCache(CacheType c, int size=MAX_CACHE_SIZE);
	~Cache();
};


