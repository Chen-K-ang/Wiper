#include "reg51.h"
#include <ADC0832.H>
#include <LCD1602.H>	   

#define ABS(x) ((x > 0) ? (x) : (-x))
uchar i=0;
uint dat; 
uint dat1=0;
uint dat2=0;
uint dat_value=0;
uint DJ_dat=0,ms=0,ms_1=0;
uchar loop=0;

sbit key_test = P1^3;
sbit key_1=P3^5;   //阈值
sbit key_2=P3^4;   //自动/手动
sbit key_3=P3^2;   //雨刷电机
sbit key_4=P3^3;   //档位选择
sbit key_motor  = P3^1;  //喷水马达
sbit key_add    = P3^6;  //数值加
sbit key_reduce = P3^7;  //数值减

sbit led0 = P1^0;   //led指示
sbit led1 = P1^1;   //led指示

sbit clean_motor = P2^3;

sbit motor_IN1 = P2^4;
sbit motor_IN2 = P2^5;
sbit motor_pwm = P2^6;

bit key_test_flag = 0;
bit key_1_flag=0;   //按键标志位
bit key_2_flag=0;   //按键标志位
bit key_3_flag=0;   //按键标志位
bit key_4_flag=0;   //按键标志位
bit key_motor_flag  = 0;   //按键标志位
bit key_add_flag    = 0;   //按键标志位
bit key_reduce_flag = 0;   //按键标志位

bit A_M=1;
bit motor_start_flag = 0;
uchar speed_flag=0;

uint ir_on=0;
uint Value=60;
uint water_h=0;
uint water_l=0;
uchar state=0;
bit  star_off=1,zheng_fan=1,s0=0, display_flag = 0;

bit display(bit flag)
{
	if (flag) {
		LCD1602_write(0,0x01);
		return 0;
	}
	LCD1602_write(0,0x80);
	LCD1602_writebyte("ND:");    //显示ND
	LCD1602_write(1,0x30+dat/100%10);  //显示百
	LCD1602_write(1,0x30+dat/10%10);   //显示十
	LCD1602_write(1,0x30+dat%10);      //显示个
	LCD1602_writebyte("% ");
	if(A_M==0)
		LCD1602_writebyte("z");  //显示zd
	else
		LCD1602_writebyte("s");   //显示sd
	if(motor_start_flag==0)
		LCD1602_writebyte("F "); //显示on
	else
		LCD1602_writebyte("N "); //显示off
	LCD1602_write(1,0x30+speed_flag%10);
	LCD1602_writebyte(" ");
	LCD1602_write(1,0x30+Value/100%10);
	LCD1602_write(1,0x30+Value/10%10);   //显示十位 
	LCD1602_write(1,0x30+Value%10);//显示个位 
	
	dat_value=dat;
	LCD1602_write(0,0xC0);
	LCD1602_writebyte("MAX:");   //显示MAX
	if(state==1&&s0)
		LCD1602_writebyte("  ");
	else {
		LCD1602_write(1,0x30+water_h/10%10);  
		LCD1602_write(1,0x30+water_h%10);
	}
	LCD1602_writebyte("%  ");		   		  
	LCD1602_writebyte("MIN:"); //显示MIN
	if(state==2&&s0)
		LCD1602_writebyte("  ");
	else {
		LCD1602_write(1,0x30+water_l/10%10);   //显示十位 
		LCD1602_write(1,0x30+water_l%10);		  //显示个位    
	}
	LCD1602_writebyte("%");	  //显示%
	
	return 1;
}

void key_dispose() //按键扫描
{
	if(key_1==0){    //阈值按下
		delay_ms(8);
		if(key_1_flag){
			key_1_flag=0;
			state=(state+1)%3;
		}
	} else
		key_1_flag=1;

	if(key_2==0){  //自动/手动按下
		delay_ms(8);
		if(key_2_flag){	
			key_2_flag=0;
			if(state==0){
				A_M=~A_M;
			}
			if(A_M==0){ //自动模式
				led0=0;  //led0亮
				led1=1;  //led1灭
			} else { //手动模式
				motor_start_flag = 0;
				led1=0;//led1亮
				led0=1;//led0灭
			}
		}
	} else
		key_2_flag=1;

	if(key_3==0){     //雨刷按下
		LCD1602_delay(20);  //延时
		if(key_3_flag) {
			key_3_flag = 0;
			motor_start_flag = ~motor_start_flag; //雨刷开关
		}
	} else
		key_3_flag=1;
	
	if(key_4==0){ //档位按下
		LCD1602_delay(20);//延时消抖
		if(key_4_flag){
			key_4_flag=0;
			if(A_M){
				speed_flag=(speed_flag+1)%3;  //档位速度控制
			}
			if(speed_flag==2){
				Value=200;   //2档
			} else if(speed_flag==1) {
				Value=120;   //1档
			} else
				Value=60;
		}
	} else
		key_4_flag=1;
	
	if(key_motor==0){ //喷水电机按下
		LCD1602_delay(20);//延时消抖
		if(key_motor_flag){
			key_motor_flag=0;
			clean_motor = ~clean_motor;
		}
	} else
		key_motor_flag=1;
	
	if(key_add == 0){ //加按键按下
		LCD1602_delay(20);//延时消抖
		if(key_add_flag){
			key_add_flag=0;
			if(state==1 && water_h<100)
				water_h++;
			else if(state==2 && (water_l < (water_h-1)))
				water_l++;
			else if (state == 0 && Value < 200)
				Value += 10;
		}
	} else
		key_add_flag=1;

	if(key_reduce == 0){ //减按键按下
		LCD1602_delay(20);//延时消抖
		if(key_reduce_flag){
			key_reduce_flag=0;
			if(state==1 && water_h > 1)
				water_h--;
			else if(state==2 && (water_l < (water_h-1)))
				water_l--;
			else if (state == 0 && Value > 0)
				Value -= 10;
		}
	} else
		key_reduce_flag=1;

	if (key_test == 0) { //减按键按下
		LCD1602_delay(20);//延时消抖
		if(key_test_flag){
			key_test_flag=0;
			display_flag = ~display_flag;
		}
	} else
		key_test_flag=1;
			
}


void police_dispose()		//报警函数
{	
	if(dat_value>water_h) {
		motor_start_flag = 1; speed_flag = 2;
	} else if (dat_value>=water_l&&dat_value<=water_h) {
		motor_start_flag = 1; speed_flag = 1;
	} else if (dat_value<water_l) {
		motor_start_flag = 0; speed_flag = 0;
	}
	if (ir_on != speed_flag)
	{
		ir_on = speed_flag;
		if (speed_flag == 1)
			Value=120;
		else if (speed_flag == 2)
			Value=200;
		else
			Value=60;			
	}

}

void main()
{
	water_h=60;  //初始化定义
	water_l=30;
	TMOD=0x11;   //定时器初始化定义
	TH1=0x3c;   //50ms中断一次
	TL1=0xb0;
	TL0 = 0x9C; //100us
        TH0 = 0xFF;
	ET1=1;

	ET0=1;
	EA=1;

	TR0=1;
	TR1=1;
	LCD1602_cls(); //lcd初始化
	clean_motor = 1;
	if(A_M==0){ //自动模式手动模式选择
		led0=0;  //led0亮
		led1=1;  //led1灭
	} else {
		led1=0;//led1亮
		led0=1;//led0灭
	}
	while (1) {
		if (A_M == 0) {  //自动模式
			police_dispose();
			if (motor_start_flag) {
					motor_IN1 = 1;
					motor_IN2 = 0;
			}
		} else {//手动模式
			if (motor_start_flag) {
					motor_IN1 = 1;
					motor_IN2 = 0;
			} else {
				motor_IN1 = 0;
				motor_IN2 = 0;
			}
		}	   
	}
}

void time_0(void) interrupt 1
{
	static unsigned char count = 0;
	TR0 = 0; 
	TL0 = 0x9C;
        TH0 = 0xFF;
	count++;
	if (count <= Value && motor_start_flag)
		motor_pwm = 1;
	else
		motor_pwm = 0;
	count++;
	if (count >= 200) {//T = 20ms清零
		count = 0;
	}
	
	TR0 = 1; //开启T0
}

void time_1(void) interrupt 3   //中断函数
{
	TH1=0x3c;
	TL1=0xb0;
	ms_1++;
	if (i < 10) {
		i++;
		dat1+=A_D(0);   //满量程0-255   实际完全浸入水中是255-36= 219     0-219  
	} else {
		i=0;  
		dat1=dat1/10;
		if(dat1<=36)
			dat1=36;
		dat1=dat1-36;
		dat=100-(dat1/2.19);
		dat1=0;		 
	}
	if (ms_1 >= 10) {
		ms_1=0;
		s0=~s0;
	}
	
	key_dispose();   //按键扫描
	display(display_flag);        //显示
}
