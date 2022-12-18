#include "FileTransferClient.h"
#include "commonDef.h"
#include "FileHandle.h"
#include <thread>
#include <vector>
using namespace std;
void getFile(FileTransferClient* client, string filename)
{
    //while (true)
    //{
    client->getFile(filename);
    //}
}
FileTransferClient::FileTransferClient(string recvDir, string svrIp, uint16_t port)
{
    m_recvDir = recvDir;
    if(!FileHandle::isExist(m_recvDir.c_str()))
    {
        FileHandle::makeRecurDir(m_recvDir.c_str());
    }
    m_svrIp = svrIp;
    m_port = port;
}

FileTransferClient::~FileTransferClient()
{
    
}
void FileTransferClient::getFile(string filename)
{
    cout << "get file: " << filename << endl;
    WSADATA wsadata;
    int error = WSAStartup(0x0202, &wsadata);
    if (error) return;

    SOCKET sockFd;
    Socket::connect(sockFd, m_svrIp.c_str(), m_port);

    if(INVALID_SOCKET == sockFd)
    {
        cout << "Cannot connect to server: " << m_svrIp << " port: " << m_port << endl;
        return;
    }

    FILE_INFO fi;

    strcpy(fi.filename, filename.c_str());
    fi.filesize = 0;
    //cout << "send request to get file: " << filename << endl;
    if(sizeof(FILE_INFO) != Socket::write(sockFd, &fi, sizeof(FILE_INFO)))
    {
        cout << "socket send failed" << endl;
        Socket::close(sockFd);
        return;
    }

    //cout << "read length of file : " << filename << endl;
    if(sizeof(FILE_INFO) != Socket::read(sockFd, &fi, sizeof(FILE_INFO)))
    {
        cout << "socket read failed" << endl;
        Socket::close(sockFd);
        return;
    }

    if (fi.fileStatus != FILE_OK)
    {
        switch (fi.fileStatus)
        {
        case FILE_NOT_EXISTED:
            cout << "Error. File not found!" << endl;
            break;
        case FILE_ZERO_SIZE:
            cout << "Error. File empty!" << endl;
            break;
        case FILE_CANNOT_OPEN:
            cout << "Error. File cannot open in server" << endl;
            break;
        default:
            cout << "Error. Unknown Reason!" << endl;
        }
        Socket::close(sockFd);
        return;
    }

    //cout << "length:  " << fi.filesize << endl;
    uint64_t nleft = fi.filesize, nReadSize = 0, recievedFileSize = 0;
    char dstFilePath[256];
    sprintf(dstFilePath, "%s/%s", m_recvDir.c_str(), filename.c_str());
    FILE *destFile = fopen(dstFilePath, "wb");
    if(destFile == NULL)
    {
        cout << "Cannot Open File: " << dstFilePath << endl;
        Socket::close(sockFd);
        return ;
    }

    FILE_DATA_BLOCK *blockData = (FILE_DATA_BLOCK *)malloc(sizeof(FILE_DATA_BLOCK));
    if (blockData == NULL)
    {
        cout << "Allocate buffer to receive failed. " << endl;
        Socket::close(sockFd);
        return;
    }
    //cout << "start recieving file:" << endl;
    while (nleft > 0) {
        
		// receive data from socket
		if (0 >= Socket::read (sockFd, blockData, sizeof(FILE_DATA_BLOCK))) {
            cout << "Socket read failed" << endl;
			break;
		}
        //cout << "receive block size: " << blockData->size << endl;
		// save received data to file
		if (0 >= fwrite(blockData->data, blockData->size, 1, destFile)) {
            cout << "write data to file failed" << endl;
			break;
		}

		nleft -= blockData->size;
		recievedFileSize += blockData->size;
	}
    free(blockData);
    fflush(destFile);
    fclose(destFile);

    Socket::close(sockFd);
    cout << "Completed receiving file:  " << filename << "receie size: " << recievedFileSize << endl;
}

#define RECV_DIR "./recvBox"

vector<string> split(string str, string delimiter)
{
    vector<string> ret;
    size_t pos = 0;
    while ((pos = str.find(delimiter)) != string::npos)
    {
        ret.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }
    ret.push_back(str);
    return ret;
}

int main(int argc, char * argv[])
{
    if (argc != 4)
    {
        printf("Usage: FileTransferClient <ip> <port> <filename1,filename2,..,filename_n>\n");
        printf("\t Each file will be requested on 1 individual thread\n");
        return 0;
    }

    string ip = argv[1];
    uint16_t port = atoi(argv[2]);    
    vector<string> fileList = split(argv[3], ",");

    vector<thread> thrList;
    FileTransferClient client(RECV_DIR, ip, port);
    for (size_t i = 0; i < fileList.size(); i++)
    {
        thrList.push_back(thread(getFile, &client, fileList[i]));
    }

    for (size_t i = 0; i < thrList.size(); i++)
    {
        thrList[i].join();
    }
 
    return 0;
}
