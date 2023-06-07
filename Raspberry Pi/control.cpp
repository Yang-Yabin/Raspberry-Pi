#include <iostream>
#include <wiringPi.h>
#include <string>
#include <chrono>
#include <thread>
#include <pthread.h>
#include <ctime>
#define DIRX_PIN 26 // 定义X方向引脚编号
#define PULX_PIN 27// 定义X方向脉冲引脚编号
#define DIRY_PIN 5 // 定义Y方向引脚编号
#define PULY_PIN 6// 定义Y方向脉冲引脚编号

#define xNumber 1000  //x方向间隔所需步进电机转动圈数
#define yNumber 1550

//树莓派引脚初始化
void init_moto(){
    wiringPiSetup();  // 初始化WiringPi库
    pinMode(DIRX_PIN, OUTPUT); // 设置X方向引脚为输出模式
    pinMode(DIRY_PIN, OUTPUT); // 设置Y方向引脚为输出模式
    pinMode(PULX_PIN, OUTPUT);
    pinMode(PULY_PIN, OUTPUT); // 设置Y方向引脚为输出模式
    std::cout<<"electricMachinery init success."<<std::endl;
}

//等待函数，m_delay（1）为1min
void m_delay(int minute){
    //std::cout << "Start waiting..." << std::endl;
    std::chrono::minutes duration(minute);
    std::this_thread::sleep_for(duration);
    //std::cout << "Done waiting." << std::endl;
}

//等待函数，s_delay（1）为s
void s_delay(int sec) {
    //std::cout << "Start waiting..." << std::endl;
    std::chrono::seconds duration(sec);
    std::this_thread::sleep_for(duration);
    //std::cout << "Done waiting." << std::endl;
}

//向右移动一个间隔
void right_rotate(int n){
    digitalWrite(DIRX_PIN,HIGH);// 设置方向为right
    for (int i = 0; i<xNumber*n; i++) {
        digitalWrite(PULX_PIN,HIGH);  // 发送脉冲信号
        delay(3);  // 延迟5ms
        digitalWrite(PULX_PIN,LOW);
        delay(3);
    }
}

//向左移动一个间隔
void left_rotate(int n){
    digitalWrite(DIRX_PIN, LOW);// 设置方向为left
    for (int i = 0; i<xNumber*n; i++) {
        digitalWrite(PULX_PIN,HIGH);  // 发送脉冲信号
        delay(2);  // 延迟5ms
        digitalWrite(PULX_PIN,LOW);
        delay(2);
    }
}

//向上移动一个间隔
void up_rotate(int n){
    digitalWrite(DIRY_PIN,HIGH);// 设置方向为up 向上
    for (int i = 0; i<yNumber*n; i++) {
        digitalWrite(PULY_PIN, HIGH);  // 发送脉冲信号
        delay(3);  // 延迟5ms
        digitalWrite(PULY_PIN, LOW);
        delay(3);
    }
}

//向下移动一个间隔
void down_rotate(int n){
    digitalWrite(DIRY_PIN, LOW);// 设置方向为down 向下
    for (int i = 0; i<yNumber*n; i++) {
        digitalWrite(PULY_PIN, HIGH);  // 发送脉冲信号
        delay(2);  // 延迟5ms
        digitalWrite(PULY_PIN, LOW);
        delay(2);
    }
}

//暂停移动，两个步进电机都暂停
void stop(){
    digitalWrite(DIRY_PIN, LOW);
    digitalWrite(DIRX_PIN, LOW);
    digitalWrite(PULY_PIN, LOW);
    digitalWrite(PULX_PIN, LOW);
}

void position(int x,int y){
//从（0,0）到（x,y）
    int m=0,n=0;
    //start();
    while(n<y){
        up_rotate(1);
        n++;
    }
    while(m<x){
        right_rotate(1);
        m++;
    }
}

void rela_position(int x0,int y0,int x1,int y1){
//从（x0,y0）到（x1,y1）
    if((x1>6||x1<0)&&(y1>4||y1<0)){
        std::cout << "目标位置错误，请确认输入参数！" << std::endl;
    }
    else{
        int route_y=y1-y0;
        int route_x=x1-x0;
        if(route_y>=0){
            up_rotate(route_y);
        }
        else{
            down_rotate(abs(route_y));
        }
        if(route_x>=0){
            right_rotate(route_x);
        }
        else{
            left_rotate(abs(route_x));
        }
    }
}

//线程执行函数：x方向
void* x_rotate(void* arg){
    int x=*(int*)arg;
    if(x>=0){
        right_rotate(x);
    }
    else{
        left_rotate(abs(x));
    }
    pthread_exit(NULL);
}

//线程执行函数：y方向
void* y_rotate(void* arg){
    int y=*(int*)arg;
    if(y>=0){
        up_rotate(y);
    }
    else{
        down_rotate(abs(y));
    }
    pthread_exit(NULL);
}

//从当前移动到指定位置函数（多线程实现）
void together_rotate(int x0,int y0,int x1,int y1){
    //从（x0,y0）到（x1,y1）
    if((x1>6||x1<0)&&(y1>4||y1<0)){
        std::cout << "input position error!" << std::endl;
    }
    else{
        int x=x1-x0;
        int y=y1-y0;
        pthread_t myThread_x,myThread_y;
        pthread_create(&myThread_x,NULL,x_rotate,(void*)&x);
        pthread_create(&myThread_y,NULL,y_rotate,(void*)&y);
        pthread_join(myThread_x,NULL); // 等待第一个线程运行完毕
        pthread_join(myThread_y,NULL); // 等待第二个线程运行完毕

    }
}

void moveone(int &lastxpos,int &lastypos,std::string str){
    if (str=="up"){
        if (lastypos!=4)
        {
        rela_position(lastxpos,lastypos,lastxpos,lastypos+1);
        lastypos++;
        }

    }
    else if (str=="down"){
        if (lastypos!=0)
        {
        rela_position(lastxpos,lastypos,lastxpos,lastypos-1);
        lastypos--;
        }
    }
    else if (str=="right"){
        if (lastxpos!=6)
        {
        rela_position(lastxpos,lastypos,lastxpos+1,lastypos);
        lastxpos++;
        }
    }
    else if (str=="left"){
        if (lastxpos!=0)
        {
        rela_position(lastxpos,lastypos,lastxpos-1,lastypos);
        lastxpos--;
        }
    }
}

void getpos(std::string str, int &x,int &y){
    char c = str[0];
    if (c>='0'&&c<='9'){
        int ib = c-'0';
        x = ib%7;
        y = ib/7;
        if(y==1) x=6-x;
    }
    else if (c>='a'&&c<='z'){
        int ib = c-'a';
        x = (ib+3)%7 ;
        y = (ib+3)/7+1;
        if(y==3) x=6-x;
    }
    else if (c>='A'&&c<='Z'){
        int ib = c-'A';
        x = (ib+3)%7 ;
        y = (ib+3)/7+1;
        if(y==3) x=6-x;
    }
}
