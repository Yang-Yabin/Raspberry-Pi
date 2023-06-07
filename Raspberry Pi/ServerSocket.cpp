
#include <iostream>
#include <pthread.h>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <sys/socket.h>  //套接字 API 头文件
#include <arpa/inet.h>   //定义了 in_addr、in_port_t、inet_addr() 和 struct sockaddr_in 等函数或结构体
#include <unistd.h>      //定义了 read() 和 write() 等函数
using namespace std;

extern int send_x;
extern int send_y;
extern pthread_mutex_t myMutex;
extern pthread_mutex_t sendMutex;
extern pthread_cond_t myCond;
extern void vecPrint(std::vector<std::string> vec);

//定义全局变量
const int BUF_SIZE = 50;
char msg[BUF_SIZE];
std::string send_msg;
std::vector<std::string> recvMsg(3,"0");//存储接收到的分割后的字符串的容器
int listen_fd, client_fd; //监听套接字和连接套接字，需要在主线程和接收线程之间共享

//分割存储字符串
std::vector<std::string> div_msg(const std::string& msg) {
    std::istringstream iss(msg); // 将输入的字符串转换为 std::istringstream 对象
    std::vector<std::string> vec;

    // 将分割后的子字符串存储到 vector 容器中
    std::string token;
    while (iss >> token) {
        vec.push_back(token);
    }

    return vec;
}


void* recvThread(void* arg){
    int res;
    while (true){
        memset(msg, 0, sizeof(msg));
        int len = read(client_fd, msg, sizeof(msg)); //从客户端接收数据
        if (len <= 0) break;
        cout <<"The slient send a message:" << msg << endl;
        res=pthread_mutex_lock(&myMutex);
        if(res!=0){
            std::cout<<"lock error"<<std::endl;
        }
        recvMsg=div_msg(msg);//recvMsg[0]=时间，recvMsg[1]=命令，recvMsg[2]=目标位置
        vecPrint(recvMsg);
        res=pthread_mutex_unlock(&myMutex);

        if(res!=0){
            std::cout<<"unlock error"<<std::endl;
        }
        string s = msg;
        if (s == "exit") break; //如果接收到客户端发送的 exit 命令，则退出程序
    }
    return NULL;
}


void* sendThread(void* arg){
    int res;
    //建立监听套接字
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    //准备工作，填写监听端口等信息
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //任意IP地址，本地回环测试可以用 INADDR_LOOPBACK
    serverAddr.sin_port = htons(8001); //开放的端口号

    //将监听套接字绑定到指定的 IP 地址和端口号上
    bind(listen_fd, (sockaddr*)&serverAddr, sizeof(serverAddr));

    //开始监听，最多允许3个客户端连接
    listen(listen_fd, 3);

    while (true){
        //等待客户端连接请求
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(sockaddr);
        client_fd = accept(listen_fd, (sockaddr*)&clientAddr, &clientAddrLen);

        cout << "The slient requests a connection:" << endl;

        string ip = inet_ntoa(clientAddr.sin_addr);
        cout << "slient_IP:" << ip << endl;
        cout << "slient_port:" << ntohs(clientAddr.sin_port) << endl;

        //客户端成功连接后，开辟接收线程
        pthread_t RecvThread;
        pthread_create(&RecvThread,NULL,recvThread,NULL);
        //开辟send线程，与客户端进行通信
        close(listen_fd); //因为需要把套接字用于通信，所以这里关闭监听套接字
        while (true){

            res=pthread_mutex_lock(&sendMutex);
            if (pthread_cond_wait(&myCond, &sendMutex) == 0) {
                send_msg = "(" + std::to_string(send_x) + "," + std::to_string(send_y) + ")";
                //send(client_fd, send_msg, strlen(send_msg) + 1, 0); //向客户端发送数据
                send(client_fd, send_msg.c_str(), send_msg.length() + 1, 0);
            }
           //最终将互斥锁解锁
            res=pthread_mutex_unlock(&sendMutex);


            //if (string(send_msg) == "exit") break; //如果输入 exit 命令，则退出程序
        }
        pthread_join(RecvThread, NULL);
        close(client_fd); //关闭连接套接字
    }
    return NULL;
}




