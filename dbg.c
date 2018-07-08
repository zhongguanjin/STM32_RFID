
#include "dbg.h"





char *itoa_my(long value,char *string,int radix);
void printhex (const unsigned int x);

/* 输入 ： -radix =10 表示 10 进制，其他结果为 0
 * -value 要转换的整形数
 * -string 转换后的字符串
 * -radix = 10
 * 输出 ：无
 * 返回 ：无
*/
char *itoa_my(long value,char *string,int radix)
{
    char zm[37]="0123456789abcdefghijklmnopqrstuvwxyz";
    char aa[80]={0};
    char *cp=string;
    int i=0,j=0;
    if(radix<2||radix>36)//增加了对错误的检测
    {
        return string;
    }
    if(value<0)
    {
        return string;
    }
    else if(value ==0)
    {
        *cp=0x30;
        cp++;
        *cp=0x30;
        cp++;
        *cp='\0';
        return string;
    }
    if((value<=0x0f)&&(value>0))
    {
        aa[i++]=zm[value%radix];
        aa[i++]=zm[0];
    }
    if (value>0x0f)
    {
        long temp =value;
        while(temp>0)
        {

            aa[i++]=zm[temp%radix];
            temp/=radix;
        }
    }
    for(j=i-1;j>=0;j--)
    {
        *cp++=aa[j];
    }
    *cp='\0';
    return string;
}

//16进制输出
void printhex (const unsigned int x)
{
    char *s;
    char buf[16];
    itoa_my( x, buf,  16);
    for(s=buf;*s;s++)
    {
        USART_SendData(USART1,*s);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }

}

 void dbg_hex(uint8 *buf,uint8 len)
 {
     uint8 i =0;
     for( i=0;i<len;i++)
     {
         printhex(*buf++);
         USART_SendData(USART1,' ');
         while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

     }
     printf("\r\n");
 }



