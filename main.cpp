#include "cache_client.h"
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>

std::vector<File_read_request_t> file_requests;
void trans_trace(const char* file_path){
	uint64_t order;
	double timestamp;
	char syscall[32] = {0};
	Inode_t inode_ID; 
	uint64_t inode_size;
	off_t offset = 0;
	size_t size = 0;
	char is_hit[32] = {0};

	std::FILE* fp = std::fopen(file_path, "r");
	File_read_request_t f_request;

	// for(int i = 0; i < 1; i++){
	while(!feof(fp)){
		fscanf(fp, "%lu %lf %s %lu %lu", &order, &timestamp, &syscall, &inode_ID, &inode_size);
		if(std::string(syscall) == "READ" || std::string(syscall) == "WRITE"){
			fscanf(fp, "%lu %lu %s", &offset, &size, is_hit);
			f_request = {0, inode_ID, offset, size};
		}
		else{
			f_request = {1, inode_ID, offset, size};
		}
		file_requests.push_back(f_request);
	}
}

void print_file_requests(){
	for(auto i:file_requests){
		std::printf("syscall:%d, inode_ID:%lu, size:%lu, offset:%lu\n", i.type, i.inode, i.size, i.offset);
	}
} 

int main(int argc, char const *argv[])
{
	cache_client_t grey_cache;
	trans_trace("./traces/moodle.1-2014-09-16");
	// print_file_requests();
	char* data_buf = new char[1024*1024*4];
	struct stat file_stat;
	uint64_t request_count = 0;
	for(auto request:file_requests){
		grey_cache.parse_meta_request(request, &file_stat);
		if(request.type == 0){
			grey_cache.parse_data_request(request, data_buf);
		}
		if (++request_count % 100000 == 0){
			printf("request number: %lu\n", request_count);
		}
	}
	grey_cache.print_hit_ratio();
	return 0;
}