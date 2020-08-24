//���������
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
    P1DIR |= BIT1; //P1.1����Ϊ���
    P1SEL |=BIT1;//ʹ��P1.1���ù���

    P2DIR &=~ BIT1; //P2.1����Ϊ����
    P2SEL |=BIT1;//ʹ��P2.1���ù���
}


void timer0_init()
{
    TA0CCR0=50000;//�����ź�Ϊ����100ms
    TA0CCR1=10;//�ߵ�ƽΪ20us
    TA0CTL=TASSEL_2+MC_1+TACLR;//ʱ��Դѡ��SMCLK,�Ǿ���2��Ƶ�Ժ��0.5MHz,����ģʽΪ��ģʽ��ͬʱ���һ�¼�����
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

    ON_Period = (65536.0*overflow+FEdge-REdge)*17000/32768;             //����100��
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
  switch(TA1IV)//AIV��ֵ����0--10�ڵ�ż��ʱ�Ż�ִ��switch�����ڵ����
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
    LCD_DisplaySeg(10);//��ʾJ
    LCD_DisplayDigit(8, 2);
    LCD_ClearSeg(12);
    LCD_ClearSeg(19);//��ʾH
    LCD_DisplayDigit(0, 3);
    LCD_ClearSeg(25);
    LCD_ClearSeg(26);
    LCD_ClearSeg(27);//��ʾL
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
    //���Ӳ���ĳ�ʼ�������� I2C ģ��ĳ�ʼ���� TCA6416A ��ʼ���������ڲ�����ˣ� LCD_128 �⺯���� HT1621 ��ʼ���������ڲ�������
    //-----��ʾ�̶������LCD��-----
}

void Distance_Display()
{
      LCD_DisplayDigit(LCD_DIGIT_CLEAR,3);
      LCD_DisplayDigit(LCD_DIGIT_CLEAR,4);

          //-----����ON_Period��ֲ���ʾ����-----
      if(zhengshu>99)//100~999��3λ��
      {
        LCD_DisplayDigit(zhengshu/100,3);
        LCD_DisplayDigit((zhengshu%100)/10,4);
        LCD_DisplayDigit(zhengshu%10,5);
        LCD_DisplayDigit(xiaoshu,6);
      }
        else if(zhengshu>9)//��2λ��
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
      HT1621_Reflash(LCD_Buffer);//-----���»��棬������ʾ-----
}

