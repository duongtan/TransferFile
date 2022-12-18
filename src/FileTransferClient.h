#pragma once
#include <string>
#include "Socket.h"

class FileTransferClient
{
public:
    FileTransferClient(string recvDir, string svrIp, uint16_t port);
    ~FileTransferClient();
    void getFile(string filename);
private:
    string m_svrIp;
    uint16_t m_port;
    string m_recvDir;
};
