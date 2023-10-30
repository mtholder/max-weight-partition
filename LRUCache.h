#pragma once
#include <map>
#include <memory>
#include <algorithm>
#include "Cache.h"
#include "DoublyLinkedList.h"

class LRUCache :
	public Cache
{
private:
	size_t cacheSize;
	size_t curCnt = 0;
	shared_ptr<DoublyLinkedList<CacheDataPtr>> DLL;
	map<set<size_t>, ListNode<CacheDataPtr>*> M;

	void deleteCacheEntry(ListNode<CacheDataPtr> * node);

public:
	LRUCache(size_t size=MAX_CACHE_SIZE);
	shared_ptr<ConnectedComponent> get(const set<size_t> & key);
	using cache_val_pair = std::pair<bool, shared_ptr<ConnectedComponent> >;
	cache_val_pair get_pair(const set<size_t> & key) {
		auto itr = M.find(key);
		if (itr == M.end()) {
			return {false, shared_ptr<ConnectedComponent>{nullptr} };
		}
		return {true, get(key)};
	}
	void put(const set<size_t> &  key, 
			 shared_ptr<ConnectedComponent> value);
	size_t size() const {
		return curCnt;
	}
	void deleteKey(const set<size_t> &  key);
	void clear();
	~LRUCache();
};

