#pragma once
#include <string>
#include "ThreadPool.hpp"
#include "TransferJob.hpp"
#include <unordered_set>

using namespace std;

class FileTransferServer
{
public:
    FileTransferServer(string directory, uint16_t port);
    ~FileTransferServer();

    const int start();
    static void onCompleted(SOCKET socket, int status);
private:
    void addNewClntSock(SOCKET socket);
    void removeClntSock(SOCKET socket);
    const int readRequest(SOCKET sock);
    const int startListen();

    string m_dir;
    ThreadPool *m_thrPool;
    uint16_t m_port;
    SOCKET m_listenSocket;
    SOCKET m_maxSockId;
    unordered_set<SOCKET> m_clntSockets;
    std::mutex m_mutex;
};