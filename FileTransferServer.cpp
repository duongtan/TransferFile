#include <iostream>
#include "FileTransferServer.h"
#include "Socket.h"
#include "commonDef.h"
using namespace std;
FileTransferServer *g_svr = NULL; 

FileTransferServer::FileTransferServer (string directory, uint16_t port)
{
    m_thrPool = NULL;
    m_port = port;
    m_dir = directory;
}

FileTransferServer::~FileTransferServer()
{


}

const int FileTransferServer::start()
{
    if(m_thrPool == NULL)
    {
        m_thrPool = new ThreadPool();
    }
    return startListen();
}

void FileTransferServer::addNewClntSock(SOCKET socket)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        cout << "add client socket fromt list:  " << socket << endl;
        m_clntSockets.insert(socket);
        if(m_maxSockId < socket)
            m_maxSockId = socket;
    }
}

void FileTransferServer::removeClntSock(SOCKET socket)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(socket != INVALID_SOCKET) Socket::close(socket);

        if(m_clntSockets.find(socket) != m_clntSockets.end())
        {
            cout << "close and remove client socket fromt list:  " << socket << endl;
            m_clntSockets.erase(socket);
            m_maxSockId = m_listenSocket;
            for (auto it = m_clntSockets.begin(); it != m_clntSockets.end(); it++)
            {
                if(m_maxSockId < *it) m_maxSockId = *it;
            }
        }
    }
}

const int FileTransferServer::startListen()
{
    WSADATA wsadata;

    int error = WSAStartup(0x0202, &wsadata);
    if (error)
        return -1;
    struct sockaddr_in address;  
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(m_port);  

    if(INVALID_SOCKET == Socket::open(m_listenSocket, address))
    {
        cout << "open listen tcp socket fail" << endl;
        return -1; 
    }

    fd_set readfds;
    cout << "Server Sokcet: " << m_listenSocket << endl;
    cout << "Server start listening on port: " << m_port << endl;
    if (listen(m_listenSocket, 100) < 0)  
    {  
        cout<< "listen failed" << endl;  
        return -1;
    } 
    m_maxSockId = m_listenSocket;
    while(true)
    {
        FD_ZERO(&readfds);
        FD_SET(m_listenSocket, &readfds);
        cout << "max socket id: " << m_maxSockId << endl;

        int activity = select(m_maxSockId + 1, &readfds, NULL, NULL, NULL);
        
        cout << "there is something come" << endl;
        if ((activity < 0) && (errno!=EINTR))  
        {  
            cout<< "select error" << endl;  
        }

        if (FD_ISSET(m_listenSocket, &readfds)) // new connection
        {
            SOCKET newClntSock ;
            if ((newClntSock = accept(m_listenSocket, 
                    (struct sockaddr *)&address, &addrlen))<0)  
            {  
                cout << "accept failed" << endl;  
                continue;
            }  
             
            cout << "New connection: sockId: " << newClntSock << ", ip: " << inet_ntoa(address.sin_addr) << ", port : " <<  ntohs(address.sin_port) << endl;
            if (readRequest(newClntSock) == 0)
                addNewClntSock(newClntSock);
        }  
             
        for (auto it = m_clntSockets.begin(); it != m_clntSockets.end(); it++)  
        {  
            SOCKET sd = *it;  
                 
            if (!FD_ISSET( sd , &readfds))  
                continue;

            //Receive the request
            cout << "read request from socket: " << sd << endl;
            if(readRequest(sd) < 0)
                removeClntSock(sd);
        } 
    }
}

const int FileTransferServer::readRequest(SOCKET sock)
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    FILE_INFO fi;
    if (sizeof(FILE_INFO) != Socket::read(sock, &fi, sizeof(FILE_INFO)))
    {
        getpeername(sock, (struct sockaddr*)&address, &addrlen);
        printf("Host disconnected , ip %s , port %d \n",
            inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        return -1;
    }
    char filePath[256];
    cout << "Receiver request to transfer file: " << fi.filename << "   from socket: " << sock << endl;
    sprintf(filePath, "%s/%s", m_dir.c_str(), fi.filename);
    FileInfoTransferJob* fileInfo = new FileInfoTransferJob(m_thrPool, sock, filePath, FileTransferServer::onCompleted);
    m_thrPool->queueWork(fileInfo);
    return 0;
}

void FileTransferServer::onCompleted(SOCKET socket, int status)
{
    switch(status)
    {
        case STATUS_SUCCEED:
            cout << "socketid " << socket << " transfer file successfully!" << endl;
            break;
        case STATUS_CON_BROKEN:
            cout << "socketid " << socket << " transfer file failed! Connection broken" << endl;
            break;
        case STATUS_FILE_NOT_EXITED:
            cout << "socketid " << socket << " transfer file failed! File not existed" << endl;
            break;
        case STATUS_FILE_NOT_OPEN:
            cout << "socketid " << socket << " transfer file failed! File cannot open" << endl;
            break;
        case STATUS_FILE_READ_FILE_ERROR:
            cout << "socketid " << socket << " transfer file failed! File read error" << endl;
            break;
        case STATUS_UNKNOWN:
            cout << "socketid " << socket << " transfer file failed! Unknown Error" << endl;
            break;
        default:
            cout << "onCompleted not defined" << endl;
            break;
    }
    g_svr->removeClntSock(socket);
}

int main (int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Usage: TransferFileServer <directory> <port>\n");
        return -1;
    }
    string directory = argv[1];
    uint16_t port = atoi(argv[2]);

    FileTransferServer *svr = new FileTransferServer(directory, port);
    g_svr = svr;
    svr->start();
    return 0;
}