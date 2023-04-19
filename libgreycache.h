#ifndef _DUPLEXMT_LIBDUPLEXFS_H_
#define _DUPLEXMT_LIBDUPLEXFS_H_

#include <sys/stat.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stat;

void grey_Init(void** handle);

void grey_Final(void* handle);

int32_t grey_Stat(void* handle, const char *path, int32_t flags, struct stat *buf);

int32_t grey_Mknod(void* handle, const char *path, int32_t flags, uint32_t mode);

int32_t grey_Open(void* handle, const char *path, int32_t flags);

void grey_close(void* handle, const char *path);

int32_t grey_Mkdir(void* handle, const char *path, int32_t flags, uint32_t mode);

int32_t grey_Rmdir(void* handle, const char *path);

int32_t grey_Delete(void* handle, const char *path);

int32_t grey_Access(void* handle, const char *path);

int64_t grey_Read(void* handle, const char *path, char *buffer, uint64_t count, uint64_t offset);

int64_t grey_Write(void* handle, const char *path, char *buffer, uint64_t count, uint64_t offset);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /* _DUPLEXMT_LIBDUPLEXFS_H_ */