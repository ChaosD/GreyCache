#ifndef H_GREY_EMPTY_DATA_PREFETCHER
#define H_GREY_EMPTY_DATA_PREFETCHER

#include "../cache_client.h"
#include <unordered_map>
#include <vector>
#include<cstring>


class empty_data_prefetcher: public data_prefetcher_abs{

public:
	void enter(Cache_key_t pageID, bool is_hit){}
	void prefetch(Cache_key_t* &pageIDs, uint64_t &requests_number){}
	~empty_data_prefetcher(){}

};


#endif