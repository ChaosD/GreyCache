#include <iostream>
#include <algorithm>
#include "cache_client.h"
#include "cache_server.h"
#include "prefetcher/metadata_prefetcher.h"
#include "prefetcher/sequential_data_prefetcher.h"
#include "prefetcher/empty_data_prefetcher.h"

cache_client_t::cache_client_t():
	data_request_count(0), 
	data_cache_hit_count(0), 
	meta_request_count(0), 
	meta_cache_hit_count(0){
	// Initial instances of prefetcher
	cache_server = new cache_server_t;
	meta_prefetcher_ptr = new metadata_prefetcher;
	// data_prefetcher_ptr = new sequential_data_prefetcher;
	data_prefetcher_ptr = new empty_data_prefetcher;

}

cache_client_t::~cache_client_t(){
	// Destroy instances of prefetcher
	delete cache_server;
	delete meta_prefetcher_ptr;
	delete data_prefetcher_ptr;
}


void cache_client_t::data_prefetch(Cache_key_t* p_pages, uint64_t num_page){
	if(!num_page){
		return;
	}
	for(int i = 0; i < num_page; ++i){
		// data_cache_ptr->prefetch should be thread safe
		cache_server->data_cache_ptr->prefetch(p_pages[i]);
	}
	delete[] p_pages;
}

int cache_client_t::parse_read(Inode_t fileID, void* read_buf, size_t size, off_t offset){
	// Split a request into pages
	if(size == 0){
		return 0;
	}
	// Desired pages: [page_start, page_end) 
	Pgoff_t page_start = offset / PAGE_SIZE;
	Pgoff_t page_end = (offset + size - 1) / PAGE_SIZE + 1;
	off_t buf_offset = 0;
	auto data_cache_ptr = cache_server->data_cache_ptr;

	for (uint64_t page_i = page_start; page_i < page_end; ++page_i){
		data_request_count++;
		// query page
		Cache_key_t page_request = {fileID, page_i};
		bool is_hit = 0;
		char page_buf[PAGE_SIZE] = {0};		
		data_cache_ptr->query(page_request, page_buf, &is_hit);
		data_prefetcher_ptr->enter(page_request, is_hit);

		data_cache_hit_count += is_hit;
		// copy data from page_buf to read_buf
		// off_t off_from_start = std::min((uint64_t)offset, page_i * PAGE_SIZE);
		// size_t len_in_page = (page_i + 1) * PAGE_SIZE - off_from_start;
		// memcpy(read_buf + off_from_start, page_buf + off_from_start % PAGE_SIZE, len_in_page);
	}

	// prefetch pages
	Cache_key_t* p_pages = NULL;
	uint64_t num_page = 0;
	data_prefetcher_ptr->prefetch(p_pages, num_page);

	// Parse pages to the prefetch thread.
	std::thread data_prefetch_thread(&cache_client_t::data_prefetch, this, p_pages, num_page);
	data_prefetch_thread.detach();
	return size;
}

void cache_client_t::meta_prefetch(Inode_t* p_inodes, uint64_t num_file){
	if(!num_file){
		return;
	}
	for(int i = 0; i < num_file; ++i){
		// meta_cache_ptr->prefetch should be thread safe
		cache_server->meta_cache_ptr->prefetch(p_inodes[i]);
	}
	delete[] p_inodes;
}

int cache_client_t::parse_stat(Inode_t fileID, struct stat* read_buf){
// void cache_client_t::parse_meta_request(File_read_request_t request, struct stat* read_buf){
	// Split a request into pages

	bool is_hit = 0;
	meta_request_count++;

	// Access cache server to request file information.
	auto meta_cache_ptr = cache_server->meta_cache_ptr;
	meta_cache_ptr->query(fileID, read_buf, &is_hit);

	meta_prefetcher_ptr->enter(fileID, is_hit);

	meta_cache_hit_count += is_hit;

	// prefetch inodes (the metadata of file)
	Inode_t* p_inodes = NULL;
	uint64_t num_file = 0;
	meta_prefetcher_ptr->prefetch(p_inodes, num_file);

	// Parse pages to the prefetch thread.
	std::thread meta_prefetch_thread(&cache_client_t::meta_prefetch, this, p_inodes, num_file);
	meta_prefetch_thread.detach();
	return sizeof(struct stat);
}

void cache_client_t::print_hit_ratio(){
	std::cout << "Data requests: " << data_request_count << std::endl;
	std::cout << "Hit: " << data_cache_hit_count << std::endl;
	std::cout << "Hit ratio: " << (double)data_cache_hit_count / data_request_count << std::endl;

	std::cout << "Meta requests: " << meta_request_count << std::endl;
	std::cout << "Hit: " << meta_cache_hit_count << std::endl;
	std::cout << "Hit ratio: " << (double)meta_cache_hit_count / meta_request_count << std::endl;
}

int cache_client_t::open(Inode_t fileID, int flags){
	return cache_server->store->open(fileID.c_str(), flags);
}
void cache_client_t::close(Inode_t fileID){
	cache_server->store->close(fileID.c_str());
}

int cache_client_t::create(Inode_t fileID, int flags){
	struct stat file_meta;
	cache_server->meta_cache_ptr->insert(fileID, &file_meta);
	return cache_server->store->create(fileID.c_str(), flags);
}

int cache_client_t::remove(Inode_t fileID){
	return cache_server->store->remove(fileID.c_str());
}

int cache_client_t::mkdir(Inode_t fileID, int flags, mode_t mode){
	struct stat file_meta;
	cache_server->meta_cache_ptr->insert(fileID, &file_meta);
	return cache_server->store->mkdir(fileID.c_str(), flags, mode);
}

int cache_client_t::rmdir(Inode_t fileID){
	return cache_server->store->rmdir(fileID.c_str());
}

int cache_client_t::write(Inode_t fileID, char* buf, size_t size, off_t offset){

	// // Split a request into pages
	// if(size == 0){
	// 	return 0;
	// }
	// // Desired pages: [page_start, page_end) 
	// Pgoff_t page_start = offset / PAGE_SIZE;
	// Pgoff_t page_end = (offset + size - 1) / PAGE_SIZE + 1;

	// for (uint64_t page_i = page_start; page_i < page_end; ++page_i){
	// 	// query page
	// 	Cache_key_t page_request = {fileID, page_i};
	// 	bool is_hit = 0;
	// 	char page_buf[PAGE_SIZE] = {0};		
	// 	cache_server->data_cache_ptr->insert(page_request, page_buf);
	// }

	return cache_server->store->write(fileID.c_str(), buf, size, offset);
}