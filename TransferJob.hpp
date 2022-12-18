#pragma once
#include "ThreadPool.hpp"
#include "Socket.h"
#include "commonDef.h"
#include "FileHandle.h"
#include <fcntl.h>
#include <stdlib.h>
#include <io.h>
enum
{
    STATUS_SUCCEED= 0,
    STATUS_CON_BROKEN,
    STATUS_FILE_NOT_OPEN,
    STATUS_FILE_READ_FILE_ERROR,
    STATUS_FILE_NOT_EXITED,
    STATUS_UNKNOWN
};

typedef void(*jobCallback)(SOCKET, int);
class DataTransferJob : public JobItem
{
public:
    DataTransferJob(ThreadPool *thrPool, int clntSocket, FILE *fp, int64_t offset, int64_t fileSize, jobCallback callback)
    {
        m_callback = callback;
        m_clntSocket = clntSocket;
        m_fp = fp;
        m_offset = offset;
        m_fileSize = fileSize;
        m_thrPool = thrPool;
    }
    ~DataTransferJob(){}
    void execute()
    {
        FILE_DATA_BLOCK *dataBlock = (FILE_DATA_BLOCK*)malloc(sizeof(FILE_DATA_BLOCK));

        dataBlock->size = BLOCK_DATA_SIZE;
        if((m_fileSize - m_offset) < BLOCK_DATA_SIZE)
        {
            dataBlock->size = (uint32_t)(m_fileSize - m_offset);
        }

       // _lseeki64(m_fp, m_offset, SEEK_SET);
        if(1 != fread(dataBlock->data, dataBlock->size, 1, m_fp))
        { // error come, 
            free(dataBlock);
            fclose(m_fp);
            m_callback(m_clntSocket, STATUS_FILE_READ_FILE_ERROR);
            return;
        }

        if(sizeof(FILE_DATA_BLOCK) != Socket::write(m_clntSocket, dataBlock, sizeof(FILE_DATA_BLOCK)))
        {
            free(dataBlock);
            fclose(m_fp);
            m_callback(m_clntSocket, STATUS_CON_BROKEN);
            return;
        }
       
        uint64_t next_offset = m_offset + dataBlock->size;
        free(dataBlock);

        if(next_offset >= m_fileSize)
        {
            fclose(m_fp);
            m_callback(m_clntSocket, STATUS_SUCCEED);
            return;
        }
        if(m_thrPool) // add job to continue transfer data file
        {
            DataTransferJob* dataJob = new DataTransferJob(m_thrPool, m_clntSocket, m_fp, next_offset, m_fileSize, m_callback);
            m_thrPool->queueWork(dataJob);
        }
        else 
        {
            fclose(m_fp);
            m_callback(m_clntSocket, STATUS_UNKNOWN);
        }
    }
private:
    int m_clntSocket;
    FILE* m_fp;
    int64_t m_offset;
    int64_t m_fileSize;
    ThreadPool *m_thrPool;
    jobCallback m_callback;
};

class FileInfoTransferJob : public JobItem
{
public:
    FileInfoTransferJob(ThreadPool *thrPool, int clntSocket, string filename, jobCallback callback)
    {
        m_callback = callback;
        m_clntSocket = clntSocket;
        m_fileName = filename;
        m_thrPool = thrPool;
    }
    ~FileInfoTransferJob(){}
    void execute()
    {
        FILE *fp = NULL;
        FILE_INFO fi;
        strcpy(fi.filename, m_fileName.c_str());
        fi.fileStatus = FILE_OK;
        if(!FileHandle::isExist(m_fileName.c_str()))
        {
            fi.fileStatus = FILE_NOT_EXISTED;
            fi.filesize = 0;
        }
        else{
            fi.filesize = FileHandle::size(m_fileName.c_str());
            if(fi.filesize == 0) fi.fileStatus = FILE_ZERO_SIZE;
            if(NULL == (fp = fopen(m_fileName.c_str(), "rb")))
                fi.fileStatus = FILE_CANNOT_OPEN;
        }
        
        if(sizeof(FILE_INFO) == Socket::write(m_clntSocket, &fi, sizeof(FILE_INFO)))
        {
            if(m_thrPool && fi.fileStatus == FILE_OK) // add job to transfer data file
            {
                DataTransferJob* dataJob = new DataTransferJob(m_thrPool, m_clntSocket, fp, 0, fi.filesize, m_callback);
                m_thrPool->queueWork(dataJob);
            }
            else 
            {
                if(fp) fclose(fp);
                if(fi.fileStatus == FILE_CANNOT_OPEN)
                    m_callback(m_clntSocket, STATUS_FILE_NOT_OPEN);
                else if(fi.fileStatus == FILE_NOT_EXISTED)
                    m_callback(m_clntSocket, STATUS_FILE_NOT_EXITED);
                else
                    m_callback(m_clntSocket, STATUS_SUCCEED);
            }
        }
        else 
        {
            if(fp) fclose(fp);
            m_callback(m_clntSocket, STATUS_CON_BROKEN);
        }
    }
private:
    int m_clntSocket;
    string m_fileName;
    int64_t m_offset;
    int64_t m_fileSize;
    ThreadPool *m_thrPool;
    jobCallback m_callback;
};

