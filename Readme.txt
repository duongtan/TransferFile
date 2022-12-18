* Transfer File: I implemented a transfer file service support mutil concurrent connection.
		To avoid 1 connection 1 thread problem, here I deploy a thread pool to do transfer file block by block.
		1 thead is to listen connection and receive request.
		Thead pool with number of thread equal with number of logical cpu to reduce context switching.
		To improve speed, we can use IO async way such IOCP.
		- Server:
			- cmd: TransferFileServer.exe <directory> <port>
				- directory: the place files stored
				- port: port to listen.
		- Client:
			- cmd: TransferFileClient.exe <ip> <port> <filename1,filename2,..,filename_n>
				- ip: ip of server
				- port: port number server openning.
				- <filename1,filename2,..,filename_n> : file list to download. each file will be request on individual thread at the same time.

			- Receiver file will be stored in fixed folder: ./recvBox
