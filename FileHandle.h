#pragma once
#include <stdlib.h>
#include <fcntl.h>
#include <direct.h>
#include <basetsd.h>
#include <stdint.h>

class FileHandle
{

public :
	FileHandle ();
	~FileHandle ();

	static const bool isExist (const char *path);
	static const int64_t size (const char *path);
	static const int getStat (const char *path, int64_t&size);
	static const char *convertPath (char *path);
	static const char *convertPath (const char *src, char *dest);
	static const int makeDir (const char *dirNam);
	static const int makeRecurDir (const char *path);
	static const char *convertDelimiter(const char *src, char *dest, const uint8_t origin, const uint8_t newer);
};

