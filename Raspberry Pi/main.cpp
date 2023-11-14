#include <iostream>
#include <wiringPi.h>
#include <pthread.h>
#include <string>
#include <vector>

#include "ServerSocket.h"
#include "control.h"

#define del 5  
int lastxpos=0;
int lastypos=0;

int send_x=0;                   //server��Ҫ���ظ�PC������
int send_y=0;
int last_x=-1;
int last_y=-1;                  //��һ�η��ص�����

//����������ʼ��
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t myCond = PTHREAD_COND_INITIALIZER;           //��������

//vector<string>���
void vecPrint(std::vector<std::string> vec){
    for (auto s : vec) {
        std::cout << s<<"-";
    }
    std::cout << std::endl;
}
//�����������ƺ��� ����ȡ������SendThread�̵߳��ź�
static void sigSend(){
    int res;
    res=pthread_mutex_lock(&sendMutex);
    if(res!=0){
        std::cout<<"sendMutex lock error"<<std::endl;
    }
    send_x=lastxpos;
    send_y=lastypos;
    pthread_mutex_unlock(&sendMutex);
    if(send_x!=last_x||send_y!=last_y){
        res=pthread_cond_signal(&myCond);
        if (res != 0) {
            std::cout<<"myCond error."<<std::endl;
        }
    }
    last_x=send_x;
    last_y=send_y;
}

int main(){
    pthread_t ServerSocket;
    pthread_create(&ServerSocket, NULL,sendThread,NULL);

    int index;                      //�Զ�Ѳ��ʱ����

    int xpause=0;                   //��¼��ͣʱ������
    int ypause=0;

    int xpos = 0;//
    int ypos = 0;
    
    int res;
    std::vector<std::string> buffer(3,"0");             //���PC�����������
    std::vector<std::string> buffer1(3,"0");

    init_moto();                                        //��ʼ����ݮ��Ӳ��

    while(1){
        int res=pthread_mutex_lock(&myMutex);
        if(res!=0){
            std::cout<<"lock error"<<std::endl;
        }
        buffer=recvMsg;
        res=pthread_mutex_unlock(&myMutex);
        if(res!=0){
            std::cout<<"unlock error"<<std::endl;
        }
        if(buffer1!=buffer){
            buffer1=buffer;
            vecPrint(buffer);

            //�Զ�Ѳ��
            if(buffer[1]=="auto"){
                index = 0;
                while (index < 35) {
                    int i = index % 7;
                    int j = index / 7;
                    if (j % 2 == 0) {  // ��������
                        rela_position(lastxpos, lastypos, i, j);
                    }
                    else { // ��������
                        i=6-i;
                        rela_position(lastxpos, lastypos, i, j);
                    }
                    
                    lastxpos = i;
                    lastypos = j;
                    index++;
                    sigSend();
                    s_delay(del);

                    pthread_mutex_lock(&myMutex);
                    buffer=recvMsg;
                    pthread_mutex_unlock(&myMutex);
                    if(buffer!=buffer1) {
                        break;
                    }

                    if (i==6&&j!=4) {  //ÿһ��Ѳ�����������һ��
                        if (j% 2 == 0) {
                            i=6;
                            j++;
                            rela_position(lastxpos, lastypos, i, j);
                        }
                        
                        lastxpos = i;
                        lastypos = j;
                        index++;
                        sigSend();
                        s_delay(del);

                        pthread_mutex_lock(&myMutex);
                        buffer=recvMsg;
                        pthread_mutex_unlock(&myMutex);
                        if(buffer!=buffer1) {
                            break;
                        }
                    }

                    while(index==35){
                        int index = 0;
                        int i = index%7;
                        int j = index/7;
                        together_rotate(lastxpos,lastypos,i,j);
                        lastxpos = i;
                        lastypos = j;
                        sigSend();
                        std::cout<<"Automatic inspection completed."<<std::endl;
                        break;
                    }
                }
            }

            //ָ��λ��Ѳ��
            else if (buffer[1]=="book"){
                getpos(buffer[2] ,xpos,ypos);
                std::cout<<"target:"<<xpos<<" "<<ypos<<std::endl;
                std::cout<<"now:"<<lastxpos<<" "<<lastypos<<std::endl;
                together_rotate(lastxpos,lastypos,xpos,ypos);
                lastxpos = xpos;
                lastypos = ypos;

                res=pthread_mutex_lock(&sendMutex);
                if(res!=0){
                    std::cout<<"sendMutex lock error"<<std::endl;
                }
                send_x=lastxpos;
                send_y=lastypos;
                pthread_mutex_unlock(&sendMutex);
                if(send_x!=last_x||send_y!=last_y){
                    res=pthread_cond_signal(&myCond);
                    if (res != 0) {
                        std::cout<<"myCond error."<<std::endl;
                    }
                }
                last_x=send_x;
                last_y=send_y;

                std::cout << "book (" << lastxpos << "," << lastypos << ") finish." << std::endl;

                pthread_mutex_lock(&myMutex);
                buffer=recvMsg;
                pthread_mutex_unlock(&myMutex);
                if(buffer!=buffer1) {
                    break;
                }
            }

            //�ϡ��¡�����
            else if (buffer[1]=="move"){
                moveone(lastxpos,lastypos,buffer[2]);
                std::cout << lastxpos << lastypos << std::endl;
                sigSend();
                std::cout<<"move finish."<<std::endl;
            }
            //��ͣ
            else if (buffer[1]=="pause"){
                std::cout<<"pause"<<std::endl;
                xpause=lastxpos;
                ypause=lastypos;
            }
            //�����Զ�Ѳ��
            else if (buffer[1]=="continue"){
                std::cout<<"continue automatic inspection."<<std::endl;
                //rela_position(lastxpos,lastypos,xpause,ypause);
                if(index==35){
                    break;
                }
                int i = index % 7;
                int j = index / 7;
                if (j % 2 == 0) {  
                    rela_position(lastxpos, lastypos, i, j);
                }
                else { 
                    i=6-i;
                    rela_position(lastxpos, lastypos, i, j);
                }
                lastxpos = i;
                lastypos = j;
                index++;
                sigSend();
                s_delay(1);

                while (index < 35) {
                    int i = index % 7;
                    int j = index / 7;
                    if (j % 2 == 0) {  // ��������
                        rela_position(lastxpos, lastypos, i, j);
                    }
                    else { // ��������
                        i=6-i;
                        rela_position(lastxpos, lastypos, i, j);
                    }
                    
                    lastxpos = i;
                    lastypos = j;
                    index++;
                    sigSend();//�����ź�
                    s_delay(del);

                    pthread_mutex_lock(&myMutex);
                    buffer=recvMsg;
                    pthread_mutex_unlock(&myMutex);
                    if(buffer!=buffer1) {
                        break;
                    }

                    if (i==6&&j!=4) {  // ÿһ��Ѳ�����������һ��
                        if (j% 2 == 0) {
                            i=6;
                            j++;
                            rela_position(lastxpos, lastypos, i, j);
                        }
                        
                        lastxpos = i;
                        lastypos = j;
                        index++;
                        sigSend();
                        s_delay(del);

                        pthread_mutex_lock(&myMutex);
                        buffer=recvMsg;
                        pthread_mutex_unlock(&myMutex);
                        if(buffer!=buffer1) {
                            break;
                        }
                    }

                    while(index==35){
                        int index = 0;
                        int i = index%7;
                        int j = index/7;
                        together_rotate(lastxpos,lastypos,i,j);
                        lastxpos = i;
                        lastypos = j;
                        sigSend();
                        std::cout<<"Automatic inspection continue completed."<<std::endl;
                        break;
                    }
                }
            }
            //��������
            else if (buffer[1]=="finish"){
                std::cout<<"finish"<<std::endl;
                return 0;
            }
            //�������
            else{
                std::cout<<"input wrong!!!"<<std::endl;
            }

        }
        s_delay(1);
    }

    pthread_join(ServerSocket,NULL);
    return 0;
}
