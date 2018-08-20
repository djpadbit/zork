#ifndef FILE_H
#define FILE_H
#include <ctype.h>
#include <bfile.h>
#include <stddef.h>

#ifndef GETC_BUFF
#define GETC_BUFF 64
#endif
#ifndef FILE_MAX
#define FILE_MAX 4
#endif

typedef struct file_t {
	int bfile_idx;
	int file_size;
	unsigned char* getc_buff;
	int getc_bidx;
	int getc_fidxr;
	int getc_fidxc;
	int getc_fidx;
} file_t;

static file_t *files[FILE_MAX] = {NULL};

void file_make_path(uint16_t* dst,char* root,char *fold,char *fn);
int file_create(char* filename,int size);
int file_open(char* filename,enum BFile_OpenMode mode);
int file_fwrite(const void *ptr, int size, int nmemb, int fileidx);
int file_fread(void *ptr, int size, int nmemb, int fileidx);
int file_close(int fileidx);
int file_getc(int fileidx);
int file_ftell(int fileidx);
int file_fseek(int fileidx,int offset,int or);

#endif