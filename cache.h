#ifndef H_GREY_CACHE
#define H_GREY_CACHE

#include <unistd.h>
#include <stdint.h>
#include <string>

#define PAGE_SIZE 4096
#define MAX_CACHE_SIZE 10000

typedef int64_t Pgoff_t;
typedef std::string Inode_t;

typedef struct
{
	bool type; // 0 refers to data reads, and 1 refers to meta reads.
	Inode_t inode;
	off_t offset;
	size_t size;
} File_read_request_t;

struct Cache_key_t
{
	Inode_t inode;
	Pgoff_t page_id;
	bool operator < (const Cache_key_t& key) const{
		if (inode != key.inode){
			return inode < key.inode;
		}
		else{
			return page_id < key.page_id;
		}
	}

	bool operator == (const Cache_key_t& key) const{
		return page_id == key.page_id && inode == key.inode;
	}
};

namespace std
{
    template<>
    struct hash<Cache_key_t>
    {
        uint64_t operator()(Cache_key_t const& s) const
        {
            uint64_t const h1 (std::hash<Inode_t>()(s.inode) * 0x932983238);
            uint64_t const h2 (std::hash<Pgoff_t>()(s.page_id) * 0x28128912);
            return h1 ^ h2;
        }
    };
}

class data_prefetcher_abs;
class meta_prefetcher_abs;

class cache_client_t;
class data_cache_t;
class meta_cache_t;
class cache_server_t;

#endif