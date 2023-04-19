#ifndef H_GREY_SEQUENTIAL_DATA_PREFETCHER
#define H_GREY_SEQUENTIAL_DATA_PREFETCHER

#include "../cache_client.h"
#include <vector>
#include <cstring>
#include <unordered_map>

struct file_ra_stat{
	file_ra_stat(): start(0),size(8), prev_pos(-1){}
	Pgoff_t start; 		// Where readahead start
	int size; 			// Number of readahead pages
	Pgoff_t prev_pos; 	// Cache last read() position
};

class sequential_data_prefetcher: public data_prefetcher_abs{
public:
	std::unordered_map<Inode_t, file_ra_stat> prefetch_map;
	~sequential_data_prefetcher(){}

private:
	std::vector<Cache_key_t> prefetch_page_pool;

public:
	void enter(Cache_key_t pageID, bool is_hit){
		Inode_t inode = pageID.inode;
		Pgoff_t current_pos = pageID.page_id;

		auto iter = prefetch_map.find(inode);
		// Not find the file, insert the item into the map.
		if (iter == prefetch_map.end()){
			prefetch_map[inode] = file_ra_stat();
		}
		file_ra_stat& f_ra = prefetch_map[inode];
		// Adjust the size of prefetch window
		
		// if(!is_hit){
		// 	f_ra.size /= 2;
		// }
		// else{
		// 	f_ra.size += 1;
		// }

		// If hitting file and cache, read ahead the following pages
		if(current_pos == f_ra.start || current_pos == (f_ra.start - 1) || (current_pos == 0 && f_ra.prev_pos == -1)){
			for(int i = 0; i < f_ra.size; i++){
				Cache_key_t pageID = {inode, current_pos + i};
				prefetch_page_pool.push_back(pageID);
			}
			f_ra.start = current_pos + f_ra.size;
			f_ra.prev_pos = current_pos;
		}
	}


	void prefetch(Cache_key_t* &pageIDs, uint64_t &requests_number){
		requests_number = prefetch_page_pool.size();
		if(requests_number == 0){
			return;			
		}
		pageIDs = new Cache_key_t[requests_number];
		std::memcpy(pageIDs, &prefetch_page_pool[0], requests_number * sizeof(Cache_key_t));
		prefetch_page_pool.clear();

	}
};


#endif