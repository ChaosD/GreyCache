#ifndef H_GREY_CEPH_BACKEND
#define H_GREY_CEPH_BACKEND


#include <cephfs/libcephfs.h>
#include <map>
#include "backend.h"
class ceph_backend: public backend{
private: 		
	const char* user_name;
	const char* remote_prefix;
	const char* conf_path;
	std::map<std::string, int> fd_map;
	struct ceph_mount_info *cmount;
	
public:
	ceph_backend();
	
	void init();
	void final();

	int open(const char *path, int flags);
	void close(const char *path);

	int create(const char *path, int flags);
	int remove(const char *path);

	int mkdir(const char *path, int flags, mode_t mode);
	int rmdir(const char *path);

	int read_stat(const char *path, struct stat* buf);
	int read_page(const char *path, Pgoff_t page_off, char* buf);
	int write(const char *path, char* buf, size_t size, off_t offset);
	~ceph_backend();
};

#endif