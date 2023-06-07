#include <iostream>
#include <thread>
#include <cstring>
#include <sstream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

//����ȫ�ֱ���
const int BUF_SIZE = 50;
char msg[BUF_SIZE];//������
SOCKADDR_IN serverAddr; //����˵�ַ��Ϣ

std::string get_time(std::string input_str) {
    time_t now_time = time(nullptr);  // ��ȡ��ǰʱ�䣬��ת����time_t����
    char time_str[20] = {0};          // �����ַ������Ÿ�ʽ�����ʱ���ַ���
    strftime(time_str, sizeof(time_str), "%Y-%m-%d/%H:%M:%S", localtime(&now_time));  // ��time_t����ʱ��ת����ָ����ʽ���ַ���
    //std::cout << "��ǰʱ��Ϊ��" << time_str << std::endl;  // �����ǰʱ���ַ���

    std::string output_str = std::string(time_str) + " " + input_str;  // ���û�������ַ���ǰ�����ʱ����Ϣ
    //std::cout << "������Ϊ��" << output_str << std::endl;
    return output_str;
}


void recvThread(SOCKET sock)
{
    while (true)
    {
        memset(msg, 0, sizeof(msg));
        int len = recv(sock, msg, sizeof(msg), 0);
        if (len <= 0) break;
        cout << "����˻ظ���" << msg << endl;
    }
}

int main()
{
    //��ʼ��WinSocket����
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //�����׽���
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //׼����������дҪ���ӵķ�������Ϣ
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.137.240"); //��ݮ�ɷ�������IP��ַ
    serverAddr.sin_port = htons(8001); //��ݮ�ɿ��ŵĶ˿ں�

    //���ӷ�����
    connect(sock, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR));

    //���ٷ��ͺͽ����߳�
    thread t(recvThread, sock);
    t.detach();

    while (true)
    {
    //cout << "�ң�"<<endl;
    std::string buffer;
    std::getline(std::cin, buffer);
    buffer = get_time(buffer);
    send(sock, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    }


    closesocket(sock); //�ر��׽���
    WSACleanup(); //�ͷ�WinSocket����
    return 0;
}
