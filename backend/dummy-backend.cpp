#include <cerrno>
#include <cstdio>
#include "dummy-backend.h"

dummy_backend::dummy_backend():current(0){

}

void dummy_backend::init(){
}

void dummy_backend::final(){
}

dummy_backend::~dummy_backend(){}

int dummy_backend::open(const char *path, int flags){
	printf("DUMMY open file: %s, flags: %d\n", path, flags);
	// Boundary cases?
	int fd = current++;	
	fd_map[path] = fd;
	return fd;
}

void dummy_backend::close(const char *path){
	printf("DUMMY close file: %s\n", path);
	auto iter = fd_map.find(path);
	if(iter == fd_map.end()){
		return;
	}
	fd_map.erase(iter);
}

int dummy_backend::create(const char *path, int flags){
	printf("DUMMY create file: %s, flags: %d\n", path, flags);
	return 0;
}

int dummy_backend::remove(const char *path){
	printf("DUMMY remove file: %s\n", path);
	return 0;
}

int dummy_backend::mkdir(const char *path, int flags, mode_t mode){
	printf("DUMMY make directory: %s, mode: %d\n", path, mode);
	return 0;
}

int dummy_backend::rmdir(const char *path){
	printf("DUMMY remove directory: %s\n", path);
	return 0;
}

int dummy_backend::read_stat(const char *path, struct stat* buf){
	printf("DUMMY make directory: %s\n", path);
	return 0;
}

int dummy_backend::write(const char *path, char* buf, size_t size, off_t offset){
	printf("DUMMY write, file: %s, size:%lu, offset:%lu\n", path, size, offset);
	return size;
}

int dummy_backend::read_page(const char *path, Pgoff_t page_off, char* buf){
	printf("DUMMY read page, file: %s, page offset:%lu\n", path, page_off);
	return PAGE_SIZE;
}
