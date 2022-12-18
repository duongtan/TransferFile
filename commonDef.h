#pragma once
#include <iostream>
#include <string>
#define BLOCK_DATA_SIZE 8192
enum{
    FILE_OK,
    FILE_NOT_EXISTED,
    FILE_ZERO_SIZE,
    FILE_CANNOT_OPEN
}FILE_STATUS;
#pragma pack(1)

typedef struct {
    char filename[256];
    uint64_t fileStatus : 2;
    uint64_t filesize   : 62;

}FILE_INFO;

typedef struct 
{
    uint32_t size;
    char data[BLOCK_DATA_SIZE];
}FILE_DATA_BLOCK;