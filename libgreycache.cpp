#include "cache_client.h"
#include "libgreycache.h"

void grey_Init(void** handle){
	auto *client = new cache_client_t();
	*handle = client;
}

void grey_Final(void* handle){
	cache_client_t* client = (cache_client_t*)handle;
	delete client;
}

int32_t grey_Stat(void* handle, const char *path, int32_t flags, struct stat *buf){
	cache_client_t* client = (cache_client_t*)handle;
	client->parse_stat(path, buf);
	return 1;
}

int32_t grey_Mknod(void* handle, const char *path, int32_t flags, uint32_t mode){
	cache_client_t* client = (cache_client_t*)handle;
	return client->create(path, flags);
}


int32_t grey_Open(void* handle, const char *path, int32_t flags){
	cache_client_t* client = (cache_client_t*)handle;
	return client->open(path, flags);

}

void grey_close(void* handle, const char *path){
	cache_client_t* client = (cache_client_t*)handle;
	client->close(path);
}

int32_t grey_Mkdir(void* handle, const char *path, int32_t flags, uint32_t mode){
	cache_client_t* client = (cache_client_t*)handle;
	return client->mkdir(path, flags, mode);
}

int32_t grey_Rmdir(void* handle, const char *path){
	cache_client_t* client = (cache_client_t*)handle;
	return client->rmdir(path);	
}

int32_t grey_Delete(void* handle, const char *path){
	cache_client_t* client = (cache_client_t*)handle;
	return client->remove(path);
}

int32_t grey_Access(void* handle, const char *path){
	struct stat buf;
	return grey_Stat(handle, path, 0, &buf);
}

int64_t grey_Read(void* handle, const char *path, char *buffer, uint64_t count, uint64_t offset){
	cache_client_t* client = (cache_client_t*)handle;
	return client->parse_read(path, buffer, count, offset);
}

int64_t grey_Write(void* handle, const char *path, char *buffer, uint64_t count, uint64_t offset){
	cache_client_t* client = (cache_client_t*)handle;
	return client->write(path, buffer, count, offset);
}