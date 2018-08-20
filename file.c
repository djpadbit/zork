#include <ctype.h>
#include "file.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bfile.h>

// Based on EDIT's source code (see FilePath function)
void file_make_path(uint16_t* dst,char* root,char *fold,char *fn)
{
	char *tp;
	tp = (char*)malloc(2+strlen(root)+1+strlen(fold)+1+strlen(fn)+1); // probably off by 1 or 2 bytes
	if(strlen(fold)==0) sprintf(tp,"\\\\%s\\%s",root,fn); //File path without folder
	else if(strlen(fn)==0) sprintf(tp,"\\\\%s\\%s",root,fn); //File path without file
	else sprintf(tp,"\\\\%s\\%s\\%s",root,fold,fn); //File path with file & folder
	for (int i=0;i<strlen(tp);i++) dst[i] = tp[i];
	dst[strlen(tp)] = 0;
	free(tp);
}

int file_getfile(int fileidx)
{
	for (int i=0;i<FILE_MAX;i++) {
		if (files[i]->bfile_idx == fileidx) return i;
	}
	return -1;
}

int file_getfree()
{
	for (int i=0;i<FILE_MAX;i++) {
		if (files[i] == NULL) return i;
	}
	return -1;
}

int file_create(char* filename,int size)
{
	uint16_t path[64];
	file_make_path(path,"fls0","",filename);
	int r = BFile_Open(path,BFile_ReadOnly);
	if (r > 0) {
		BFile_Close(r);
		BFile_Remove(path);
	}
	int s = size;
	r = BFile_Create(path,BFile_File,&s);
	if (r < 0) {}//BFile_optimise? but we don't have that in gint sadly
	return r;
}

int file_open(char* filename,enum BFile_OpenMode mode)
{
	int filep = file_getfree();
	if (filep < 0) return -1;
	uint16_t path[64];
	file_make_path(path,"fls0","",filename);
	int bfid = BFile_Open(path,mode);
	if (bfid < 0) return bfid;
	files[filep] = (file_t*) malloc(sizeof(file_t));
	if (files[filep] == NULL) {BFile_Close(bfid);return -1;}
	files[filep]->getc_buff = (unsigned char*) malloc(sizeof(unsigned char)*GETC_BUFF);
	if (files[filep]->getc_buff == NULL) {BFile_Close(bfid);return -1;}
	files[filep]->bfile_idx = bfid;
	files[filep]->file_size = BFile_GetFileSize(bfid);
	files[filep]->getc_bidx = GETC_BUFF;
	files[filep]->getc_fidxr = 0;
	files[filep]->getc_fidxc = 0;
	files[filep]->getc_fidx = 0;
	return bfid;
}

int file_close(int fileidx)
{
	int fi = file_getfile(fileidx);
	if (fi < 0) return -1;
	free(files[fi]->getc_buff);
	free(files[fi]);
	files[fi] = NULL;
	return BFile_Close(fileidx);
}

int file_fwrite(const void *ptr, int size, int nmemb, int fileidx)
{
	return BFile_Write(fileidx,ptr,size*nmemb);
}

int file_fread(void *ptr, int size, int nmemb, int fileidx)
{
	return BFile_Read(fileidx,ptr,size*nmemb,-1);
}

int file_getc(int fileidx)
{
	int fi = file_getfile(fileidx);
	if (fi < 0) return -1;
	if (files[fi]->getc_bidx >= GETC_BUFF || files[fi]->getc_fidxc) {
		if (!files[fi]->getc_fidxc) files[fi]->getc_fidx += files[fi]->getc_fidxr;
		files[fi]->getc_fidxc = 0;
		files[fi]->getc_fidxr = BFile_Read(fileidx,files[fi]->getc_buff,GETC_BUFF,files[fi]->getc_fidx);
		files[fi]->getc_bidx = 0;
	}
	if (files[fi]->getc_bidx+files[fi]->getc_fidx >= files[fi]->file_size) return -1;
	return files[fi]->getc_buff[files[fi]->getc_bidx++];
}

int file_ftell(int fileidx)
{
	int fi = file_getfile(fileidx);
	if (fi < 0) return -1;
	return files[fi]->getc_bidx+files[fi]->getc_fidx;
}

int file_fseek(int fileidx,int offset,int or)
{
	int fi = file_getfile(fileidx);
	if (fi < 0) return -1;
	if (or == 1) {
		files[fi]->getc_fidx = offset;
	} else if (or == 2) {
		files[fi]->getc_fidx += offset;
	}
	files[fi]->getc_fidxc = 1;
	if (files[fi]->getc_fidx >= files[fi]->file_size) {
		files[fi]->getc_fidx = files[fi]->file_size;
		return -1;
	}
	return 0;
}