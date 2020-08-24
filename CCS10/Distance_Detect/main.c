//超声波检测
#include <msp430.h>
#include "LCD_128.h"
#include"TCA6416A.h"
#include "HT1621.h"

int REdge=0,FEdge=0,zhengshu=0,xiaoshu=0;
int overflow=0;
unsigned char show_flag=0;
void LCD_Init();
void Distance_Display();
void display_my_info();
void cal_distance();
void timer1_stop();
void gpio_init();
void timer0_init();
void timer1_init();
int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL2=DIVS_1;
  LCD_Init();
  TCA6416A_Init();
  display_my_info();
  gpio_init();
  timer0_init();
  timer1_init();
  _EINT();
  while(1)
  {
      if(show_flag==1)
      {
          _DINT();
          Distance_Display();
          timer1_init();
          show_flag=0;
          _EINT();
      }
      LPM0;
  }
}


void gpio_init()
{
    P1DIR |= BIT1; //P1.1设置为输出
    P1SEL |=BIT1;//使用P1.1复用功能

    P2DIR &=~ BIT1; //P2.1设置为输入
    P2SEL |=BIT1;//使用P2.1复用功能
}


void timer0_init()
{
    TA0CCR0=50000;//触发信号为周期100ms
    TA0CCR1=10;//高点平为20us
    TA0CTL=TASSEL_2+MC_1+TACLR;//时钟源选择SMCLK,是经过2分频以后的0.5MHz,计数模式为增模式，同时清除一下计数器
    TA0CCTL0=OUTMOD_3;
}


void timer1_init()
{
    TA1CCTL1 = CAP + CM_3 + CCIE + SCS + CCIS_0;
    TA1CTL |= TASSEL_1 + MC_2 + TACLR;        // SMCLK, Cont Mode; start timer
}

void timer1_stop()
{
    TA1CTL |= MC_0 + TACLR; // stop
}

void cal_distance()
{
    static char count=0;
    static float sum=0;
    float ON_Period=0;

    ON_Period = (65536.0*overflow+FEdge-REdge)*17000/32768;             //扩大100倍
    sum=sum+ON_Period;
    count++;
    if(count==15)
    {
        show_flag=1;
        ON_Period=sum/15.0;
        //ON_Period=1.0229*ON_Period + 0.4164;
        ON_Period=-0.00001*ON_Period*ON_Period +1.0254*ON_Period+0.3254;
        zhengshu=(int)ON_Period;
        xiaoshu=(ON_Period-zhengshu)*10;
        count=0;
        sum=0;
        overflow=0;
    }
}


#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR (void)
{
  switch(TA1IV)//AIV的值是在0--10内的偶数时才会执行switch函数内的语句
  {
      case  TA1IV_NONE: break;              // Vector  0:  No interrupt
      case  TA1IV_TACCR1:                   // Vector  2:  TACCR1 CCIFG
        if (TA1CCTL1 & CCI)                 // Capture Input Pin Status
        {
            REdge= TA1CCR1;
        }
        else
        {
            FEdge = TA1CCR1;
            timer1_stop();
            cal_distance();
        }
        break;
      case TA1IV_TACCR2: break;             // Vector  4:  TACCR2 CCIFG
      case TA1IV_6: break;                  // Vector  6:  ON_Perioderved CCIFG
      case TA1IV_8: break;                  // Vector  8:  ON_Perioderved CCIFG
      case TA1IV_TAIFG:
              overflow+=1;
      default:  break;
  }
  LPM0_EXIT;
}



void display_my_info()
{
    LCD_DisplaySeg(4);
    LCD_DisplaySeg(9);
    LCD_DisplaySeg(10);//显示J
    LCD_DisplayDigit(8, 2);
    LCD_ClearSeg(12);
    LCD_ClearSeg(19);//显示H
    LCD_DisplayDigit(0, 3);
    LCD_ClearSeg(25);
    LCD_ClearSeg(26);
    LCD_ClearSeg(27);//显示L
    LCD_DisplayDigit(1, 4);
    LCD_DisplayDigit(0, 5);
    LCD_DisplayDigit(7, 6);
    HT1621_Reflash(LCD_Buffer);
    __delay_cycles(1000000);
    LCD_Clear();
    LCD_DisplayDigit(0, 5);
    LCD_DisplayDigit(0, 6);
    LCD_DisplaySeg(_LCD_DOT4);
    HT1621_Reflash(LCD_Buffer);
}

void LCD_Init()
{
    HT1621_init();
    //相关硬件的初始化，其中 I2C 模块的初始化由 TCA6416A 初始化函数在内部完成了， LCD_128 库函数由 HT1621 初始化函数在内部引用了
    //-----显示固定不变的LCD段-----
}

void Distance_Display()
{
      LCD_DisplayDigit(LCD_DIGIT_CLEAR,3);
      LCD_DisplayDigit(LCD_DIGIT_CLEAR,4);

          //-----根据ON_Period拆分并显示数字-----
      if(zhengshu>99)//100~999（3位）
      {
        LCD_DisplayDigit(zhengshu/100,3);
        LCD_DisplayDigit((zhengshu%100)/10,4);
        LCD_DisplayDigit(zhengshu%10,5);
        LCD_DisplayDigit(xiaoshu,6);
      }
        else if(zhengshu>9)//（2位）
      {
        LCD_DisplayDigit(zhengshu/10,4);
        LCD_DisplayDigit(zhengshu%10,5);
        LCD_DisplayDigit(xiaoshu,6);
      }
      else
      {
        LCD_DisplayDigit(zhengshu,5);
        LCD_DisplayDigit(xiaoshu,6);
      }
      HT1621_Reflash(LCD_Buffer);//-----更新缓存，真正显示-----
}

