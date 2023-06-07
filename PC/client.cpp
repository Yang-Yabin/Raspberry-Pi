#include <iostream>
#include <thread>
#include <cstring>
#include <sstream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

//定义全局变量
const int BUF_SIZE = 50;
char msg[BUF_SIZE];//缓冲区
SOCKADDR_IN serverAddr; //服务端地址信息

std::string get_time(std::string input_str) {
    time_t now_time = time(nullptr);  // 获取当前时间，并转换成time_t类型
    char time_str[20] = {0};          // 定义字符数组存放格式化后的时间字符串
    strftime(time_str, sizeof(time_str), "%Y-%m-%d/%H:%M:%S", localtime(&now_time));  // 将time_t类型时间转换成指定格式的字符串
    //std::cout << "当前时间为：" << time_str << std::endl;  // 输出当前时间字符串

    std::string output_str = std::string(time_str) + " " + input_str;  // 在用户输入的字符串前面添加时间信息
    //std::cout << "输出结果为：" << output_str << std::endl;
    return output_str;
}


void recvThread(SOCKET sock)
{
    while (true)
    {
        memset(msg, 0, sizeof(msg));
        int len = recv(sock, msg, sizeof(msg), 0);
        if (len <= 0) break;
        cout << "服务端回复：" << msg << endl;
    }
}

int main()
{
    //初始化WinSocket环境
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //建立套接字
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //准备工作，填写要连接的服务器信息
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.137.240"); //树莓派服务器的IP地址
    serverAddr.sin_port = htons(8001); //树莓派开放的端口号

    //连接服务器
    connect(sock, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR));

    //开辟发送和接收线程
    thread t(recvThread, sock);
    t.detach();

    while (true)
    {
    //cout << "我："<<endl;
    std::string buffer;
    std::getline(std::cin, buffer);
    buffer = get_time(buffer);
    send(sock, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    }


    closesocket(sock); //关闭套接字
    WSACleanup(); //释放WinSocket环境
    return 0;
}
