#include "util.h"

bool
get_file_ioblksize(const char *path, blksize_t *blksize)
{
	if (path == NULL)
		return false;
	char *resolvedpath;
	resolvedpath = realpath(path, NULL);
	if (resolvedpath == NULL)
		return false;
	struct stat st;
	if (stat(resolvedpath, &st) != 0) {
		free(resolvedpath);
		return false;
	}
	*blksize = st.st_blksize;
	free(resolvedpath);
	return true;
}

bool
get_file_mode(const char *path, mode_t *mode)
{
	if (path == NULL || mode == NULL)
		return false;
	char *resolvedpath;
	resolvedpath = realpath(path, NULL);
	if (resolvedpath == NULL)
		return false;
	struct stat st;
	if (stat(resolvedpath, &st) != 0) {
		free(resolvedpath);
		return false;
	}
	*mode = st.st_mode;
	free(resolvedpath);
	return true;
}

bool
is_valid_dir(const char *path)
{	
	if (path == NULL)
		return false;
	char *resolvedpath;
	resolvedpath = realpath(path, NULL);
	if (resolvedpath == NULL)
		return false;
	struct stat st;
	if (stat(resolvedpath, &st) != 0 || !S_ISDIR(st.st_mode)) {
		free(resolvedpath);
		return false;
	}
	free(resolvedpath);
	return true;
}

bool
is_valid_reg(const char *path)
{	
	if (path == NULL)
		return false;
	char *resolvedpath;
	resolvedpath = realpath(path, NULL);
	if (resolvedpath == NULL)
		return false;
	struct stat st;
	if (stat(resolvedpath, &st) != 0 || !S_ISREG(st.st_mode)) {
		free(resolvedpath);
		return false;
	}
	free(resolvedpath);
	return true;
}

bool
file_exists(const char *path)
{
	if (path == NULL)
		return false;
	char *resolvedpath;
	resolvedpath = realpath(path, NULL);
	if (resolvedpath == NULL)
		return false;
	struct stat st;
	if (stat(resolvedpath, &st) != 0) {
		free(resolvedpath);
		return false;
	}
	free(resolvedpath);
	return true;
}

bool
file_same(const char *path1, const char *path2)
{
	if (path1 == NULL || path2 == NULL)
		return false;
	struct stat st1, st2;
	if (stat(path1, &st1) != 0) 
		return false;
	if (stat(path2, &st2) != 0)
		return false;
	if(st1.st_ino != st2.st_ino)
		return false;
	return true;
}