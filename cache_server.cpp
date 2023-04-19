#include <string.h>
#include "cache_client.h"
#include "cache_server.h"
#include "cache_scheme/LRU.h"
#include "backend/dummy-backend.h"
#include "backend/ceph-backend.h"
#include <cstdio>

data_cache_t::data_cache_t(int size, backend* be):
	cache_size(size),
	store(be){
	// Allocate the monitor of metadata cache
	// Can be replaced!!!
	cache_monitor = new lru_cache<Cache_key_t>;
}


size_t data_cache_t::read_page_from_storage(Cache_key_t pageID, void* buf){
	return store->read_page(pageID.inode.c_str(), pageID.page_id, (char*)buf);
}


size_t data_cache_t::query(Cache_key_t pageID, void* buf, bool* is_hit){

	mtx.lock();
	// Check whether the desired page is in cache.
	auto iter = cache_space.find(pageID);
	mtx.unlock();

	size_t result = 0;
	// If the page is in the cache
	if (iter != cache_space.end()){
		// Read data from the cache
		*is_hit = true;
		memcpy(buf, cache_space[pageID], PAGE_SIZE);
		result = PAGE_SIZE;
	}
	// If the page is not in the cache
	else{
		*is_hit = false;
		// Read the data from the cache
		result = read_page_from_storage(pageID, buf);
		// Check whether the queried page should enter the cache or not. (Admission)
		// bool is_admission = cache_monitor->enter(pageID);
		// if (is_admission){
			insert(pageID, buf);
		// }
	}
	return result;
}

bool data_cache_t::insert(Cache_key_t pageID, void* buf){

	bool is_admission = cache_monitor->enter(pageID);

	if(is_admission){
		// Check the cache is full?
		// If the cache is full, evict the last page first
		if (!(cache_space.size() < cache_size)){
			evict(1);
		}
		mtx.lock();
		// Insert the page into cache.
		char* cache_buf = new char[PAGE_SIZE];
		memcpy(cache_buf, buf, PAGE_SIZE);
		cache_space[pageID] = cache_buf;
		mtx.unlock();
	}
	return is_admission;
}

bool data_cache_t::pre_insert(Cache_key_t pageID, void* buf){

	bool is_admission = cache_monitor->pre_enter(pageID);

	if(is_admission){
		// Check the cache is full?
		// If the cache is full, evict the last page first
		if (!(cache_space.size() < cache_size)){
			evict(1);
		}
		mtx.lock();
		// Insert the page into cache.
		char* cache_buf = new char[PAGE_SIZE];
		memcpy(cache_buf, buf, PAGE_SIZE);
		cache_space[pageID] = cache_buf;
		mtx.unlock();
	}
	return is_admission;
}

void data_cache_t::evict(uint64_t evicted_number){
	Cache_key_t* page_IDs = new Cache_key_t[evicted_number];
	// Use the evicting module to choose which pages to evict
	cache_monitor->evict(evicted_number, page_IDs);
	mtx.lock();
	// delete the evicted pages
	for (int i = 0; i < evicted_number; i++){
		Cache_key_t& key = page_IDs[i];
		// Search the cache_space for the pageID
		auto iter = cache_space.find(key);
		// The evicted item must be in cache.
		// assert(iter != cache_space.end());
		// Free the page buffer;
		delete[] (char*)iter->second;
		// Erase the cache entry.
		cache_space.erase(iter);
	}
	mtx.unlock();
	delete[] page_IDs;
}

bool data_cache_t::prefetch(Cache_key_t pageID){
	mtx.lock();
	// Check whether the desired page is in cache.
	auto iter = cache_space.find(pageID);
	mtx.unlock();
	char buf[PAGE_SIZE] = {0};
	// If the page is not in the cache
	if (iter == cache_space.end()){
		// Read the page from storage
		read_page_from_storage(pageID, buf);
		// The page enter the cache via the prefetch-method
		// cache_monitor->pre_enter(pageID);
		pre_insert(pageID, buf);
		return true;
	}
	return false;
}

/*=================================================================================
	Meta Cache
===================================================================================*/


meta_cache_t::meta_cache_t(int size, backend* be):
	cache_size(size),
	store(be){
	// Allocate the monitor of metadata cache
	// Can be replaced!!!
	cache_monitor = new lru_cache<Inode_t>;
}

size_t meta_cache_t::read_meta_from_storage(Inode_t fileID, struct stat* buf){
	return store->read_stat(fileID.c_str(), buf);
}


size_t meta_cache_t::query(Inode_t fileID, struct stat* buf, bool* is_hit){

	mtx.lock();
	// Check whether the desired page is in cache.
	auto iter = cache_space.find(fileID);
	mtx.unlock();

	size_t result = 1;
	// If the page is in the cache
	if (iter != cache_space.end()){
		// Read data from the cache

		*is_hit = true;
		*buf = cache_space[fileID];
	}
	// If the page is not in the cache
	else{
		*is_hit = false;
		// Read the data from the cache
		result = read_meta_from_storage(fileID, buf);

		// Check whether the queried page should enter the cache or not. (Admission)
		// bool is_admission = cache_monitor->enter(fileID);

		// if (is_admission){
		// 	insert(fileID, buf);
		// }
		insert(fileID, buf);

	}
	return result;
}


bool meta_cache_t::insert(Inode_t fileID, struct stat* buf){
	// Check the cache is full?
	// If the cache is full, evict the last item first

	bool is_admission = cache_monitor->enter(fileID);

	if(is_admission){
		if (!(cache_space.size() < cache_size)){
			evict(1);
		}
		mtx.lock();
		// Insert the file metadata into cache.
		cache_space[fileID] = *buf;
		mtx.unlock();
	}
	return is_admission;
}

bool meta_cache_t::pre_insert(Inode_t fileID, struct stat* buf){
	// Check the cache is full?
	// If the cache is full, evict the last item first

	bool is_admission = cache_monitor->pre_enter(fileID);

	if(is_admission){
		if (!(cache_space.size() < cache_size)){
			evict(1);
		}
		mtx.lock();
		// Insert the file metadata into cache.
		cache_space[fileID] = *buf;
		mtx.unlock();
	}
	return is_admission;
}

void meta_cache_t::evict(uint64_t evicted_number){
	Inode_t* inodes = new Inode_t[evicted_number];
	// Use the evicting module to choose which pages to evict
	cache_monitor->evict(evicted_number, inodes);
	mtx.lock();
	// delete the evicted pages
	for (int i = 0; i < evicted_number; i++){
		Inode_t fileID = inodes[i];
		// Search the cache_space for the fileID
		auto iter = cache_space.find(fileID);
		// The evicted item must be in cache.
		// assert(iter != cache_space.end());
		// Erase the cache entry.
		cache_space.erase(iter);
	}
	mtx.unlock();
	delete[] inodes;
}

bool meta_cache_t::prefetch(Inode_t fileID){
	mtx.lock();
	// Check whether the desired page is in cache.
	auto iter = cache_space.find(fileID);
	mtx.unlock();

	struct stat file_meta;
	// If the file metadata is not in the cache
	if (iter == cache_space.end()){
		// Read the metadata from storage
		read_meta_from_storage(fileID, &file_meta);
		// The page enter the cache via the prefetch-method
		// cache_monitor->pre_enter(fileID);
		pre_insert(fileID, &file_meta);
	}
}

/*=================================================================================
	Cache Server
===================================================================================*/

cache_server_t::cache_server_t():
	store(new ceph_backend())
{
	printf("Start Initial");
	store->init();
	meta_cache_ptr = new meta_cache_t(MAX_CACHE_SIZE, store);
	data_cache_ptr = new data_cache_t(MAX_CACHE_SIZE, store);
}

cache_server_t::~cache_server_t(){
	delete meta_cache_ptr;
	delete data_cache_ptr;
	store->final();
	delete store;
}