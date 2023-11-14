#ifndef CONTROL_H
#define CONTROL_H

extern void init_moto();                //初始化
extern void m_delay(int minute);        //等待函数，m_delay（1）为1min
extern void s_delay(int sec);           //等待函数，s_delay（1）为s
extern void right_rotate(int n);
extern void left_rotate(int n);
extern void up_rotate(int n);
extern void down_rotate(int n);
extern void stop();
extern void rela_position(int x0,int y0,int x1,int y1);
extern void* x_rotate(void* arg);
extern void* y_rotate(void* arg);
extern void together_rotate(int x0,int y0,int x1,int y1);
extern void getpos(std::string str, int &x,int &y);
extern void moveone(int &lastxpos,int &lastypos,std::string str);

#endif // 