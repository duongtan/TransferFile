#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fileHandle.h"
#include <string>


FileHandle::FileHandle ()
{
}

FileHandle::~FileHandle ()
{
}

const bool FileHandle::isExist (const char *path)
{
	if (NULL == path) {
		return (false);
	}

	if (_access (path, 0) < 0) {
		return (false);
	}

	return (true);
}

const int64_t FileHandle::size(const char *path)
{
	int64_t 	size;

	if (0 != getStat (path, size)) {
		return (-1);
	}
	return (size);
}

const int FileHandle::getStat (const char *path, int64_t &size)
{
	struct _stat64	statbuf;
	if (_stat64(path, &statbuf) < 0) {
		return (-1);
	}

	size = statbuf.st_size;
	return (0);
}


// convert path delimiter
const char *FileHandle::convertDelimiter (const char *src, char *dest, const uint8_t origin, const uint8_t newer)
{
	if ((NULL == src) || (NULL == dest)) {
		return (NULL);
	}

	uint32_t	i;
	for (i = 0; i < strlen(src); i++) {
		dest[i]	= (origin == src[i]) ? newer : src[i];
	}
	dest[i]	= 0x00;

#ifdef SYS_DEBUG
	printf ("[FileHandle::convertDelimiter] %s -> %s\n", src, dest);
#endif

	return (dest);
}

// convert path delimiter to internal style ('/')
const char *FileHandle::convertPath (char *path)
{
	return ( convertPath (path, path) );
}

const char *FileHandle::convertPath (const char *src, char *dest)
{
	return ( convertDelimiter (src, dest, '\\', '/') );
}

const int FileHandle::makeRecurDir (const char *pPath)
{
	if (NULL == pPath) {
		return (-1);
	}

	char	path[256];
	convertPath (pPath, path);

	int			nDirNameLen;
	char		dirName[128], tmp[64];
	char		*ptr;
	const char	*pDir;

	ptr			= NULL;
	pDir		= path;
	dirName[0]	= 0x00;

	do {
		ptr = (char *)strchr (pDir, '/');

		if (NULL != ptr) {
			nDirNameLen = strlen(pDir) - strlen(ptr);

			strncpy (tmp, pDir, nDirNameLen);
			tmp[nDirNameLen] = 0x00;

			pDir = ptr + 1;
		}
		else {
			strcpy (tmp, pDir);
		}

#if !defined(WIN32)
		strcat (dirName, tmp);
		strcat (dirName, "/");
#else
		if (dirName[0] == 0x00) {
			strcat (dirName, tmp);
			strcat (dirName, "/");
		}
		else {
			strcat (dirName, "/");
			strcat (dirName, tmp);
		}
#endif

		if ( (0 != strcmp (dirName, ".")) && (0 != strcmp (dirName, "..")) ) {
			if (!isExist(dirName) && 0 != makeDir (dirName)) {
				return (-1);
			}
		}
	} while (NULL != ptr);

	return (0);
}

const int FileHandle::makeDir (const char *pDirName)
{
	if (NULL == pDirName) {
		return (-1);
	}

	// make directory
	if (0 > _mkdir (pDirName)) {
		return (-1);
	}

	return (0);
}
