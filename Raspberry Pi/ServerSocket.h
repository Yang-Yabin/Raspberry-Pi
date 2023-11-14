#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

extern std::vector<std::string> recvMsg;
extern void* recvThread(void* arg);
extern void* sendThread(void* arg);

#endif 
