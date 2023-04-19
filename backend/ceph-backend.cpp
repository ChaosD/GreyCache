#include <cerrno>
#include <cstdio>
#include "ceph-backend.h"

ceph_backend::ceph_backend():
	user_name("admin"),
	remote_prefix("/"),
	conf_path("/etc/ceph/ceph.conf"){}

void ceph_backend::init(){
	int ret = ceph_create(&cmount, "admin");
	ret = ceph_conf_read_file(cmount, "/etc/ceph/ceph.conf");
	ret = ceph_mount(cmount, "/");
	if(ret){
		printf("Mount error:%d\n", ret);
		ceph_shutdown(cmount);
	}
	else{
		printf("Mounted.\n");
	}
}

void ceph_backend::final(){
	int ret = ceph_unmount(cmount);
    ret = ceph_release(cmount);
}

ceph_backend::~ceph_backend(){}


int ceph_backend::open(const char *path, int flags){
	// Boundary cases?
	int fd = ceph_open(cmount, path, flags, 0644);
	fd_map[path] = fd;
	return fd;

}

void ceph_backend::close(const char *path){
	auto iter = fd_map.find(path);
	if(iter == fd_map.end()){
		return;
	}
	const int fd = iter->second;
	fd_map.erase(iter);
	ceph_close(cmount, fd);
}

int ceph_backend::create(const char *path, int flags){
	int ret = open(path, flags | 00000100);
	// printf("Create %s, return: %d\n", path, ret);
	return ret; // Mask for create
}

int ceph_backend::remove(const char *path){
	int ret =  ceph_unlink(cmount, path);
	// printf("Unlink %s, return: %d\n", path, ret);
	return ret;
}

int ceph_backend::mkdir(const char *path, int flags, mode_t mode){
	int ret = ceph_mkdir(cmount, path, mode);
	// printf("Mkdir %s, return: %d\n", path, ret);
	return ret;
}

int ceph_backend::rmdir(const char *path){
	int ret = ceph_rmdir(cmount, path);
	// printf("Rmdir %s, return: %d\n", path, ret);
	return ret;


}

int ceph_backend::read_stat(const char *path, struct stat* buf){
	return ceph_listxattr(cmount, path, (char* )buf, sizeof(struct stat));
}

int ceph_backend::write(const char *path, char* buf, size_t size, off_t offset){
	auto iter = fd_map.find(path);
	if(iter == fd_map.end()){
		return -1;
	}
	const int fd = iter->second;
	int ret = ceph_write(cmount, fd, buf, size, offset);
	
	if (ret < 0) {
		printf("unable to write file to CephFS");
	}
	else if (ret < size) {
		printf("short write to CephFS");
	}
	return size;
}

int ceph_backend::read_page(const char *path, Pgoff_t page_off, char* buf){
	auto iter = fd_map.find(path);
	if(iter == fd_map.end()){
		return -1;
	}
	const int fd = iter->second;
	ceph_read(cmount, fd, buf, PAGE_SIZE, PAGE_SIZE * page_off);
	return PAGE_SIZE;
}
