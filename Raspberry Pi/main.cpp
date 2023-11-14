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

int send_x=0;                   //server需要返回给PC的坐标
int send_y=0;
int last_x=-1;
int last_y=-1;                  //上一次返回的坐标

//互斥锁及初始化
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t myCond = PTHREAD_COND_INITIALIZER;           //条件变量

//vector<string>输出
void vecPrint(std::vector<std::string> vec){
    for (auto s : vec) {
        std::cout << s<<"-";
    }
    std::cout << std::endl;
}
//条件变量控制函数 发送取消阻塞SendThread线程的信号
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

    int index;                      //自动巡检时计数

    int xpause=0;                   //记录暂停时的坐标
    int ypause=0;

    int xpos = 0;//
    int ypos = 0;
    
    int res;
    std::vector<std::string> buffer(3,"0");             //存放PC端命令的容器
    std::vector<std::string> buffer1(3,"0");

    init_moto();                                        //初始化树莓派硬件

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

            //自动巡检
            if(buffer[1]=="auto"){
                index = 0;
                while (index < 35) {
                    int i = index % 7;
                    int j = index / 7;
                    if (j % 2 == 0) {  // 从左向右
                        rela_position(lastxpos, lastypos, i, j);
                    }
                    else { // 从右向左
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

                    if (i==6&&j!=4) {  //每一行巡检结束进入下一行
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

            //指定位置巡检
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

            //上、下、左、右
            else if (buffer[1]=="move"){
                moveone(lastxpos,lastypos,buffer[2]);
                std::cout << lastxpos << lastypos << std::endl;
                sigSend();
                std::cout<<"move finish."<<std::endl;
            }
            //暂停
            else if (buffer[1]=="pause"){
                std::cout<<"pause"<<std::endl;
                xpause=lastxpos;
                ypause=lastypos;
            }
            //继续自动巡检
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
                    if (j % 2 == 0) {  // 从左向右
                        rela_position(lastxpos, lastypos, i, j);
                    }
                    else { // 从右向左
                        i=6-i;
                        rela_position(lastxpos, lastypos, i, j);
                    }
                    
                    lastxpos = i;
                    lastypos = j;
                    index++;
                    sigSend();//发送信号
                    s_delay(del);

                    pthread_mutex_lock(&myMutex);
                    buffer=recvMsg;
                    pthread_mutex_unlock(&myMutex);
                    if(buffer!=buffer1) {
                        break;
                    }

                    if (i==6&&j!=4) {  // 每一行巡检结束进入下一行
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
            //结束程序
            else if (buffer[1]=="finish"){
                std::cout<<"finish"<<std::endl;
                return 0;
            }
            //输入错误
            else{
                std::cout<<"input wrong!!!"<<std::endl;
            }

        }
        s_delay(1);
    }

    pthread_join(ServerSocket,NULL);
    return 0;
}
