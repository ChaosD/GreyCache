#ifndef H_GREY_CACHE_SERVER
#define H_GREY_CACHE_SERVER

#include "cache.h"
#include <unordered_map>
#include <mutex>
#include "backend/backend.h"

template <class K>
class cache_scheme_abs{
public:
	// Admission
	virtual bool enter(K &request) = 0;
	virtual bool pre_enter(K &request) = 0;
	// Eviction
	virtual void evict(uint64_t evict_number, K* p_request) = 0;
	virtual ~cache_scheme_abs(){}
};

/*=================================================================================
	Data Cache
===================================================================================*/
class data_cache_t{
private:
	cache_scheme_abs<Cache_key_t>* cache_monitor;
	// cache_space: we use a map to manage the cached pages.
	// Keys of the map: pageID
	// Values of the map: pointer to the page buffer.
	std::unordered_map<Cache_key_t, void*> cache_space;
	uint64_t cache_size;
	std::mutex mtx;
	backend* store;
public:
	void evict(uint64_t evicting_number);
	//-----------------
	size_t read_page_from_storage(Cache_key_t pageID, void* buf);
	// read_page_to_storage(Cache_key_t pageID, void* buf);
	//-----------------

	data_cache_t(int size, backend* be);
	bool insert(Cache_key_t pageID, void* buf);
	bool pre_insert(Cache_key_t pageID, void* buf);
	size_t query(Cache_key_t pageID, void* buf, bool* is_hit);
	bool prefetch(Cache_key_t pageID);
};

/*=================================================================================
	Meta Cache
===================================================================================*/

class meta_cache_t{
private:
	cache_scheme_abs<Inode_t>* cache_monitor;
	// cache_space: a map, key-
	std::unordered_map<Inode_t, struct stat> cache_space;
	uint64_t cache_size;
	std::mutex mtx;
	backend* store;

public:
	// size_t promote(Inode_t* p_requests);
	void evict(uint64_t evicting_number);
	//-----------------
	size_t read_meta_from_storage(Inode_t fileID, struct stat* buf);
	// read_meta_to_storage(Inode_t fileID, struct stat* buf);
	//-----------------
	meta_cache_t(int size, backend* be);
	bool insert(Inode_t fileID, struct stat* buf);
	bool pre_insert(Inode_t fileID, struct stat* buf);
	size_t query(Inode_t fileID, struct stat* buf, bool* is_hit);
	bool prefetch(Inode_t fileID);
};

/*=================================================================================
	Cache Server
===================================================================================*/
class cache_server_t{
public:
	backend* store;
public:
	cache_server_t();
	class meta_cache_t* meta_cache_ptr;
	class data_cache_t* data_cache_ptr;
	~cache_server_t();
};

# endif