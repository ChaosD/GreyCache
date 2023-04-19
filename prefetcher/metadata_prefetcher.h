#ifndef H_GREY_METADATA_PREFETCHER
#define H_GREY_METADATA_PREFETCHER

#include "../cache_client.h"
#include <map>
#include <vector>

class metadata_prefetcher: public meta_prefetcher_abs{
public:
	void enter(Inode_t fileID, bool is_hit){
	}

	void prefetch(Inode_t* &fileIDs, uint64_t &requests_number){
	}
	
	~metadata_prefetcher(){}
};


#endif