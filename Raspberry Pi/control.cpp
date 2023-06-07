#include <iostream>
#include <wiringPi.h>
#include <string>
#include <chrono>
#include <thread>
#include <pthread.h>
#include <ctime>
#define DIRX_PIN 26 // ����X�������ű��
#define PULX_PIN 27// ����X�����������ű��
#define DIRY_PIN 5 // ����Y�������ű��
#define PULY_PIN 6// ����Y�����������ű��

#define xNumber 1000  //x���������貽�����ת��Ȧ��
#define yNumber 1550

//��ݮ�����ų�ʼ��
void init_moto(){
    wiringPiSetup();  // ��ʼ��WiringPi��
    pinMode(DIRX_PIN, OUTPUT); // ����X��������Ϊ���ģʽ
    pinMode(DIRY_PIN, OUTPUT); // ����Y��������Ϊ���ģʽ
    pinMode(PULX_PIN, OUTPUT);
    pinMode(PULY_PIN, OUTPUT); // ����Y��������Ϊ���ģʽ
    std::cout<<"electricMachinery init success."<<std::endl;
}

//�ȴ�������m_delay��1��Ϊ1min
void m_delay(int minute){
    //std::cout << "Start waiting..." << std::endl;
    std::chrono::minutes duration(minute);
    std::this_thread::sleep_for(duration);
    //std::cout << "Done waiting." << std::endl;
}

//�ȴ�������s_delay��1��Ϊs
void s_delay(int sec) {
    //std::cout << "Start waiting..." << std::endl;
    std::chrono::seconds duration(sec);
    std::this_thread::sleep_for(duration);
    //std::cout << "Done waiting." << std::endl;
}

//�����ƶ�һ�����
void right_rotate(int n){
    digitalWrite(DIRX_PIN,HIGH);// ���÷���Ϊright
    for (int i = 0; i<xNumber*n; i++) {
        digitalWrite(PULX_PIN,HIGH);  // ���������ź�
        delay(3);  // �ӳ�5ms
        digitalWrite(PULX_PIN,LOW);
        delay(3);
    }
}

//�����ƶ�һ�����
void left_rotate(int n){
    digitalWrite(DIRX_PIN, LOW);// ���÷���Ϊleft
    for (int i = 0; i<xNumber*n; i++) {
        digitalWrite(PULX_PIN,HIGH);  // ���������ź�
        delay(2);  // �ӳ�5ms
        digitalWrite(PULX_PIN,LOW);
        delay(2);
    }
}

//�����ƶ�һ�����
void up_rotate(int n){
    digitalWrite(DIRY_PIN,HIGH);// ���÷���Ϊup ����
    for (int i = 0; i<yNumber*n; i++) {
        digitalWrite(PULY_PIN, HIGH);  // ���������ź�
        delay(3);  // �ӳ�5ms
        digitalWrite(PULY_PIN, LOW);
        delay(3);
    }
}

//�����ƶ�һ�����
void down_rotate(int n){
    digitalWrite(DIRY_PIN, LOW);// ���÷���Ϊdown ����
    for (int i = 0; i<yNumber*n; i++) {
        digitalWrite(PULY_PIN, HIGH);  // ���������ź�
        delay(2);  // �ӳ�5ms
        digitalWrite(PULY_PIN, LOW);
        delay(2);
    }
}

//��ͣ�ƶ������������������ͣ
void stop(){
    digitalWrite(DIRY_PIN, LOW);
    digitalWrite(DIRX_PIN, LOW);
    digitalWrite(PULY_PIN, LOW);
    digitalWrite(PULX_PIN, LOW);
}

void position(int x,int y){
//�ӣ�0,0������x,y��
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
//�ӣ�x0,y0������x1,y1��
    if((x1>6||x1<0)&&(y1>4||y1<0)){
        std::cout << "Ŀ��λ�ô�����ȷ�����������" << std::endl;
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

//�߳�ִ�к�����x����
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

//�߳�ִ�к�����y����
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

//�ӵ�ǰ�ƶ���ָ��λ�ú��������߳�ʵ�֣�
void together_rotate(int x0,int y0,int x1,int y1){
    //�ӣ�x0,y0������x1,y1��
    if((x1>6||x1<0)&&(y1>4||y1<0)){
        std::cout << "input position error!" << std::endl;
    }
    else{
        int x=x1-x0;
        int y=y1-y0;
        pthread_t myThread_x,myThread_y;
        pthread_create(&myThread_x,NULL,x_rotate,(void*)&x);
        pthread_create(&myThread_y,NULL,y_rotate,(void*)&y);
        pthread_join(myThread_x,NULL); // �ȴ���һ���߳��������
        pthread_join(myThread_y,NULL); // �ȴ��ڶ����߳��������

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
