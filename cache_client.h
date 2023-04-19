#ifndef H_GREY_CACHE_CLIENT
#define H_GREY_CACHE_CLIENT
#include "cache.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>

class data_prefetcher_abs{
public:
	virtual void enter(Cache_key_t pageID, bool is_hit) = 0;
	virtual void prefetch(Cache_key_t* &pageIDs, uint64_t &requests_number) = 0;
	virtual ~data_prefetcher_abs(){};
};

class meta_prefetcher_abs{
public:
	virtual void enter(Inode_t fileID, bool is_hit) = 0;
	virtual void prefetch(Inode_t* &fileIDs, uint64_t &requests_number) = 0;
	virtual ~meta_prefetcher_abs(){};
};

class cache_client_t{
private:
	class meta_prefetcher_abs* meta_prefetcher_ptr;
	class data_prefetcher_abs* data_prefetcher_ptr;
	class cache_server_t* cache_server;
	uint64_t data_request_count;
	uint64_t data_cache_hit_count;
	uint64_t meta_request_count;
	uint64_t meta_cache_hit_count;
	void data_prefetch(Cache_key_t* pages, uint64_t num_page);
	void meta_prefetch(Inode_t* inode, uint64_t num_file);

public:
	cache_client_t();
	void print_hit_ratio();

	int parse_read(Inode_t fileID, void* read_buf, size_t size, off_t offset);
	int parse_stat(Inode_t fileID, struct stat* buf);

	int open(Inode_t fileID, int flags);
	void close(Inode_t fileID);

	int create(Inode_t fileID, int flags);
	int remove(Inode_t fileID);

	int mkdir(Inode_t fileID, int flags, mode_t mode);
	int rmdir(Inode_t fileID);

	int write(Inode_t fileID, char* buf, size_t size, off_t offset);

	~cache_client_t();
};

#endif