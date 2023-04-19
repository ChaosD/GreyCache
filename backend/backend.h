#ifndef H_GREY_BACKEND
#define H_GREY_BACKEND

#include <sys/stat.h>
#include "../cache.h"
class backend{
public:
	virtual void init(){};
	virtual void final(){};

	virtual int open(const char *path, int flags) = 0;
	virtual void close(const char *path){};

	virtual int create(const char *path, int flags) = 0;
	virtual int remove(const char *path) = 0;

	virtual int mkdir(const char *path, int flags, mode_t mode) = 0;
	virtual int rmdir(const char *path) = 0;

	virtual int read_stat(const char *path, struct stat* buf) = 0;

	virtual int read_page(const char *path, Pgoff_t page_off, char* buf) = 0;
	virtual int write(const char *path, char* buf, size_t size, off_t offset) = 0;
	virtual ~backend(){}
};

#endif