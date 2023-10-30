#include "LRUCache.h"
#include <iostream>
#include <chrono>
using namespace std;

LRUCache::LRUCache(size_t size)
  :cacheSize(size) {
  	// MTH cout to cerr
	cerr << "LRU Cache of size " << size << " created." << endl;
	DLL = make_shared<DoublyLinkedList<CacheDataPtr>>();
}

void LRUCache::deleteCacheEntry(ListNode<CacheDataPtr> * node) {
	M.erase(node->val->key);
	DLL->deleteNode(node);
}


shared_ptr<ConnectedComponent> LRUCache::get(const set<size_t> & key) {
	shared_ptr<ConnectedComponent> value;
	auto itr = M.find(key);
	if (itr != M.end()) {
		value = itr->second->val->value;
		DLL->push_back(itr->second);
	}
	return value;
}

void LRUCache::put(const set<size_t> & key,
				   shared_ptr<ConnectedComponent> value) {
	auto itr = M.find(key);
	if (itr == M.end()) {
		auto node = DLL->addNode(make_shared<CacheData>(key, value));
		M.insert(make_pair(key, node));
		curCnt++;
		if(curCnt > cacheSize){
			deleteCacheEntry(DLL->get_front());
			curCnt--;
		}
	}
	else {
		auto node = itr->second;
		node->val->value = value;
		DLL->push_back(node);
	}
	// print();
}

void LRUCache::deleteKey(const set<size_t> & key) {
	auto itr = M.find(key);
	if (itr != M.end()) {
		deleteCacheEntry(itr->second);
		curCnt--;
	}
	// print();
}

void LRUCache::clear() {
	M.clear();
	DLL->clear();
	curCnt = 0;
}

LRUCache::~LRUCache() {
	clear();
}
