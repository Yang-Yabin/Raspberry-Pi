
#include <iostream>
#include <pthread.h>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <sys/socket.h>  //�׽��� API ͷ�ļ�
#include <arpa/inet.h>   //������ in_addr��in_port_t��inet_addr() �� struct sockaddr_in �Ⱥ�����ṹ��
#include <unistd.h>      //������ read() �� write() �Ⱥ���
using namespace std;

extern int send_x;
extern int send_y;
extern pthread_mutex_t myMutex;
extern pthread_mutex_t sendMutex;
extern pthread_cond_t myCond;
extern void vecPrint(std::vector<std::string> vec);

//����ȫ�ֱ���
const int BUF_SIZE = 50;
char msg[BUF_SIZE];
std::string send_msg;
std::vector<std::string> recvMsg(3,"0");//�洢���յ��ķָ����ַ���������
int listen_fd, client_fd; //�����׽��ֺ������׽��֣���Ҫ�����̺߳ͽ����߳�֮�乲��

//�ָ�洢�ַ���
std::vector<std::string> div_msg(const std::string& msg) {
    std::istringstream iss(msg); // ��������ַ���ת��Ϊ std::istringstream ����
    std::vector<std::string> vec;

    // ���ָ������ַ����洢�� vector ������
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
        int len = read(client_fd, msg, sizeof(msg)); //�ӿͻ��˽�������
        if (len <= 0) break;
        cout <<"The slient send a message:" << msg << endl;
        res=pthread_mutex_lock(&myMutex);
        if(res!=0){
            std::cout<<"lock error"<<std::endl;
        }
        recvMsg=div_msg(msg);//recvMsg[0]=ʱ�䣬recvMsg[1]=���recvMsg[2]=Ŀ��λ��
        vecPrint(recvMsg);
        res=pthread_mutex_unlock(&myMutex);

        if(res!=0){
            std::cout<<"unlock error"<<std::endl;
        }
        string s = msg;
        if (s == "exit") break; //������յ��ͻ��˷��͵� exit ������˳�����
    }
    return NULL;
}


void* sendThread(void* arg){
    int res;
    //���������׽���
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    //׼����������д�����˿ڵ���Ϣ
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //����IP��ַ�����ػػ����Կ����� INADDR_LOOPBACK
    serverAddr.sin_port = htons(8001); //���ŵĶ˿ں�

    //�������׽��ְ󶨵�ָ���� IP ��ַ�Ͷ˿ں���
    bind(listen_fd, (sockaddr*)&serverAddr, sizeof(serverAddr));

    //��ʼ�������������3���ͻ�������
    listen(listen_fd, 3);

    while (true){
        //�ȴ��ͻ�����������
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(sockaddr);
        client_fd = accept(listen_fd, (sockaddr*)&clientAddr, &clientAddrLen);

        cout << "The slient requests a connection:" << endl;

        string ip = inet_ntoa(clientAddr.sin_addr);
        cout << "slient_IP:" << ip << endl;
        cout << "slient_port:" << ntohs(clientAddr.sin_port) << endl;

        //�ͻ��˳ɹ����Ӻ󣬿��ٽ����߳�
        pthread_t RecvThread;
        pthread_create(&RecvThread,NULL,recvThread,NULL);
        //����send�̣߳���ͻ��˽���ͨ��
        close(listen_fd); //��Ϊ��Ҫ���׽�������ͨ�ţ���������رռ����׽���
        while (true){

            res=pthread_mutex_lock(&sendMutex);
            if (pthread_cond_wait(&myCond, &sendMutex) == 0) {
                send_msg = "(" + std::to_string(send_x) + "," + std::to_string(send_y) + ")";
                //send(client_fd, send_msg, strlen(send_msg) + 1, 0); //��ͻ��˷�������
                send(client_fd, send_msg.c_str(), send_msg.length() + 1, 0);
            }
           //���ս�����������
            res=pthread_mutex_unlock(&sendMutex);


            //if (string(send_msg) == "exit") break; //������� exit ������˳�����
        }
        pthread_join(RecvThread, NULL);
        close(client_fd); //�ر������׽���
    }
    return NULL;
}




