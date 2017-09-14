
#include <reg52.h>
#include <intrins.h>
#define uint unsigned int
#define uchar unsigned char
#define Row0 0x80  			//第一行的初始位置
#define Row1 0x80+0x44 		//第二行初始位置


sbit wr = P1^4;
sbit rs = P1^3;
sbit en = P1^5;						  
sbit IO = P2^4;   			// 7  P1.0
sbit RST = P2^3;   			//时钟 5  P1.2
sbit SCLK = P2^5;  			// 6  P1.1



sbit sda=P2^4;                          //IO口定义
sbit scl=P2^5;


sbit K1 = P3^4;
sbit K2 = P3^5;
sbit K3 = P3^6;	
sbit K4 = P3^7;							  
sbit ACC0 = ACC^0;
sbit ACC7 = ACC^7;
sbit beep = P2^2;	
sbit DS_Data = P2^0;
int cnt = 0;
uchar a, miao, shi, fen, ri, yue, nian, week;
uchar flag, temp, K1_CNT;
uchar code tab1[] = {"20  -  -      "};
uchar code tab2[] = {"  :  :  "};
uchar code nothing[] = {"Nothing"};
uchar code clear[] ={"Clear!"};
uchar K4cnt = 0;
uchar count;
uchar warning0 = 0;uchar warning1 = 0;
uchar warningflag;
int tem;
void delay(uint xms)//延时函数
{
	uint x, y;

	for(x=xms; x>0; x--)
		for(y=110; y>0; y--);
}

void DelayMs(unsigned int i)
{
	unsigned int j,k;

	for(j=i; j>0; j--)
		for(k=114; k>0; k--);
}

void nop()
{
	_nop_();
	_nop_();
}


/////////24C02读写驱动程序////////////////////
void delay1(unsigned int m)
{	unsigned int n;
  	for(n=0;n<m;n++);
}

void init()  //24c02初始化子程序
{
	scl=1;
	nop();
	sda=1;
	nop();
}

void start()        //启动I2C总线
{
	sda=1;
	nop();
	scl=1;
	nop();
	sda=0;
	nop();
	scl=0;
	nop();
}

void stop()         //停止I2C总线
{
	sda=0;
	nop();
	scl=1;
	nop();
	sda=1;
	nop();
}

void writebyte(unsigned char j)  //写一个字节
{
	unsigned char i,temp;
   	temp=j;
   	for (i=0;i<8;i++)
   {
	   temp=temp<<1;
	   scl=0;
	   nop();
	   sda=CY;		//temp左移时，移出的值放入了CY中
	   nop();
	   scl=1;		//待sda线上的数据稳定后，将scl拉高
	   nop();
   }
   scl=0;
   nop();
   sda=1;
   nop();
}

unsigned char readbyte()   //读一个字节
{
   unsigned char i,j,k=0;
   scl=0; nop(); sda=1;
   for (i=0;i<8;i++)
   {  
		nop(); scl=1; nop();
      	if(sda==1) 
		j=1;
      	else
		j=0;
      	k=(k<<1)|j;
	  	scl=0;
	}
   	nop();
	return(k);
}

void clock()         //I2C总线时钟
{
   unsigned char i=0;
   scl=1;
   nop();
   while((sda==1)&&(i<255))
   	  i++;
   scl=0;
   nop();
}

////////从24c02的地址address中读取一个字节数据/////
unsigned char read24c02(unsigned char address)
{
   unsigned char i;
   start();
   writebyte(0xa0);
   clock();
   writebyte(address);
   clock();
   start();
   writebyte(0xa1);
   clock();
   i=readbyte();
   stop();
   delay1(100);
   return(i);
}

//////向24c02的address地址中写入一字节数据info/////
void write24c02(unsigned char address,unsigned char info)
{
   start();
   writebyte(0xa0);
   clock();
   writebyte(address);
   clock();
   writebyte(info);
   clock();
   stop();
   delay1(5000); //这个延时一定要足够长，否则会出错。因为24c02在从sda上取得数据后，还需要一定时间的烧录过程。
}
/*
* 功  能：DS18B20 复位
* 参  数：无
* 返回值：无
*/
void DS18B20_Reset(void)
{
	unsigned int i;

	DS_Data = 0;
	i = 103;
	while(i>0)	i--;

	DS_Data = 1;
	i = 4;
	while(i>0)	i--;
}

/*
* 功  能：DS18B20 读取一位数据
* 参  数：无
* 返回值：读取的位
*/
bit DS18B20_ReadBit(void)	
{
	unsigned int i;
	bit dat;

	DS_Data = 0;
	i++;
	DS_Data = 1;
	i++;
	i++;
	dat = DS_Data;

	i = 8;
	while(i>0)	i--;

	return (dat);
}

/*
* 功  能：DS18B20 读取一个字节
* 参  数：无
* 返回值：读取的字节
*/
uchar DS18B20_ReadByte(void)	
{
	unsigned char i,j,dat;
	
	dat=0;
	for(i=1; i<=8; i++)
	{
		j = DS18B20_ReadBit();
		dat=(j<<7)|(dat>>1);   //读出的数据最低位在最前面，这样刚好一个字节在DAT里
	}

	return(dat);
}

/*
* 功  能：DS18B20 写入一个字节
* 参  数：要写入的字节
* 返回值：无
*/
void DS18B20_WriteByte(unsigned char dat)	
{
	unsigned int i;
	unsigned char j, k;
	bit b;

	k = dat;

	for(j=1; j<=8; j++)
	{
		b = k & 0x01;
		k = k >> 1;
		if(b)     				//write 1
		{
			DS_Data = 0;
			i++; i++;
			DS_Data = 1;
			i = 8;
			while(i>0) i--;
		}
		else
		{
			DS_Data = 0;       		//write 0
			i = 8;
			while(i>0)	i--;
			DS_Data = 1;
			i++; i++;
		}
	
	}
}

/*
* 功  能：DS18B20 开始温度转换
* 参  数：无
* 返回值：无
*/
void DS18B20_Change(void)	
{
	DS18B20_Reset();
	DelayMs(5);
	DS18B20_WriteByte(0xcc); 	 // address all drivers on bus
	DS18B20_WriteByte(0x44);  	//  initiates a single temperature conversion  
}

/*
* 功  能：DS18B20 得到温度值
* 参  数：无
* 返回值：无
*/
	
int DS18B20_GetTemp()			//get the temperature
{
	float t;
	unsigned char a, b;

	int temp;
	DS18B20_Reset();
	DelayMs(5);
	DS18B20_WriteByte(0xcc);
	DS18B20_WriteByte(0xbe);
	a=DS18B20_ReadByte();
	b=DS18B20_ReadByte();
	
	temp = b;
	temp <<= 8;             		//two byte  compose a int variable
	temp = temp|a;
	t = temp*0.0625;
	temp = t*10+0.5;
	return temp;
}

/******************液晶写入************************/
void write_1602com(uchar com)//液晶写入指令函数
{
	rs = 0;//置为写入命令
	P0 = com;//送入数据
	delay(1);
	en = 1;//拉高使能端
	delay(1);
	en = 0;//完成高脉冲
}
void write_1602dat(uchar dat)
{
	rs = 1;//置为写入数据
	P0 = dat;//送入数据
	delay(1);
	en = 1;
	delay(1);
	en = 0;
}
/*********************over***********************/
/********************ds1302****************************/
void write_byte(uchar dat)
{
	ACC = dat;
	RST = 1;
	for(a=8; a>0; a--)
	{
		IO = ACC0;
		SCLK = 0;
		SCLK = 1;
		ACC = ACC>>1;
	}
}

uchar read_byte()
{
	RST = 1;
	for(a=8; a>0; a--)
	{
		ACC7 = IO;
		SCLK = 1;
		SCLK = 0;
		ACC = ACC>>1;

	}
	return (ACC);
}

void write_1302(uchar add, uchar dat)
{
	RST = 0;
	SCLK = 0;
	RST = 1;
	write_byte(add);
	write_byte(dat);
	SCLK = 1;
	RST = 0;
}
uchar read_1302(uchar add)
{
	uchar temp;
	RST = 0;
	SCLK = 0;
	RST = 1;
	write_byte(add);
	temp = read_byte();
	SCLK = 1;
	RST = 0;
	return(temp);
}
uchar BCD_Decimal(uchar bcd)  //BCD码转换
{
	uchar Decimal;

	Decimal = bcd>>4;
	Decimal = Decimal*10 + (bcd&=0x0F);
	
	return( Decimal );
}

void ds1302_init()
{
	RST = 0;
	SCLK = 0;
	write_1302(0x80, miao|0x00);//允许写
}

void write_sfm(uchar add, uchar dat)//写时分秒
{
	uchar gw,sw;

	gw = dat % 10;
	sw = dat / 10;
	write_1602com(Row1 + add);
	write_1602dat(0x30 + sw);
	write_1602dat(0x30 + gw);
}

void write_temp(void)
{	
	int temp ;
	unsigned char d1, d2, d3;

	DS18B20_Change();  					//开始转换
	temp = DS18B20_GetTemp();
	d1 = temp / 100;
	d2 = (temp % 100) / 10;
	d3 = temp % 10;	
	write_1602com(0x80+0x42);
	write_1602dat(0x30 + d1);
	write_1602dat(0x30 + d2);
	write_1602dat('.');
	write_1602dat(0x30 + d3);
	write_1602dat('C');
	tem = 100*d1+10*d2+d3;
}
void writetemp(int temp)
{
		unsigned char d1, d2, d3;
		d1 = temp / 100;
	d2 = (temp % 100) / 10;
	d3 = temp % 10;	
	write_1602com(0x80+0x42);
	write_1602dat(0x30 + d1);
	write_1602dat(0x30 + d2);
	write_1602dat('.');
	write_1602dat(0x30 + d3);
	write_1602dat('C');
}

void writewarn(uchar warn)
{
	write_1602com(0x80+0x40);
	write_1602dat(0x30 + warn);	
}

void write_nyr(uchar add, uchar dat)
{
	uchar gw, sw;
	gw = dat % 10;
	sw = dat / 10;
	write_1602com(Row0 + add);
	write_1602dat(0x30 + sw);
	write_1602dat(0x30 + gw);
}



void write_week(uchar week)//写星期函数
{
	write_1602com(Row0+0x0d);
	switch(week)
	{
		case 1:write_1602dat('M');//delay(5);
			   write_1602dat('O');//delay(5);
			   write_1602dat('N');
			   break;
	   
		case 2:write_1602dat('T');//delay(5);
			   write_1602dat('U');//delay(5);
			   write_1602dat('E');
			   break;
		
		case 3:write_1602dat('W');//delay(5);
			   write_1602dat('E');//delay(5);
			   write_1602dat('D');
			   break;
		
		case 4:write_1602dat('T');//delay(5);
			   write_1602dat('H');//delay(5);
			   write_1602dat('U');
			   break;
		
		case 5:write_1602dat('F');//delay(5);
			   write_1602dat('R');//delay(5);
			   write_1602dat('I');
			   break;
		
		case 6:write_1602dat('S');//delay(5);
			   write_1602dat('T');//delay(5);
			   write_1602dat('A');
			   break;
		
		case 7:write_1602dat('S');//delay(5);
			   write_1602dat('U');//delay(5);
			   write_1602dat('N');
			   break;
}



}
void lcd_init()//液晶初始化函数
{
	wr = 0;
	write_1602com(0x38);//设置液晶工作模式
	write_1602com(0x0c);//开显示不显示光标
	write_1602com(0x06);//整屏不移动，指针加一
	write_1602com(0x01);
	write_1602com(Row0+2);//字符写入的位置
	
	for(a=0; a<14; a++)
	{
		write_1602dat(tab1[a]);
	}
	write_1602com(Row1 + 4);
	for(a=0; a<8; a++)
	{
		write_1602dat(tab2[a]);
	}

}

void keyscan()
{
	if(K1 == 0)//K1为功能键
	{
		delay(5);
		if(K1==0)
		{
			while(!K1);
			K1_CNT++;
			if(K1_CNT==9)
			K1_CNT=1;
			switch(K1_CNT)
			{
				case 1:	TR0 = 0;//关闭定时器
						write_1602com(Row1+0x0b);//写入光标位置
		   				write_1602com(0x0f);//设置光标为闪烁
		   				temp = (miao)/10*16+(miao)%10;
		   				write_1302(0x8e,0x00);
		   				write_1302(0x80,0x80|temp);//miao
		   				write_1302(0x8e,0x80);
		   				break;
				case 2:	write_1602com(Row1+8);//fen
						break;
				case 3: write_1602com(Row1+5);//shi
						break;
				case 4: write_1602com(Row0+0x0f);//week
						break;
				case 5: write_1602com(Row0+0x0b);//ri
						break;
				case 6: write_1602com(Row0+0x08);//yue
						break;
				case 7: write_1602com(Row0+0x05);//nian
						break;
				case 8:	write_1602com(0x0c);//设置光标不闪烁
						TR0 = 1;//打开定时器
						temp = (miao)/10*16+(miao)%10;
						write_1302(0x8e,0x00);
						write_1302(0x80,0x00|temp);//miao
						write_1302(0x8e,0x80);
						break;		
			}
		}
	}

	if(K1_CNT==0 ||K1_CNT==8)
	{
		 if(K2 == 0)
		 {
		 	delay(5);
			if(K2 == 0)
			{
				while(!K2);
				if(K4cnt!=0) 
				{
					K4cnt = 0;
				 	warningflag = 0;
					write_1602com(0x01);
					write_1602com(Row0+2);//字符写入的位置
					
					for(count=0; count<6; count++)
					{
						write_1602dat(clear[count]);
					}
					DelayMs(1000);
					write_1602com(0x01);
					write_1602com(Row0+2);//字符写入的位置
					
					for(count=0; count<14; count++)
					{
						write_1602dat(tab1[count]);
					}
					write_1602com(Row1 + 4);
					for(count=0; count<8; count++)
					{
						write_1602dat(tab2[count]);
					}
					TR0 = 1; 	
				
				}
				else
				{
				beep = 0;
				delay(200);
				beep = 1;
				warningflag = 1;
				warning1++;
				if(warning1 >5) warning1 = 0;
					write24c02(warning0, miao);
			//	DelayMs(10);
				write24c02(warning0+1, fen);
			//	DelayMs(10);
				write24c02(warning0+2, shi);
			//	DelayMs(10);
				write24c02(warning0+3, ri);
			//	DelayMs(10);
				write24c02(warning0+4, yue);
			//	DelayMs(10);
				write24c02(warning0+5, nian);
			//	DelayMs(10);
				write24c02(warning0+6, week);
			//	DelayMs(10);
				write24c02(warning0+7, tem/256);
			//	DelayMs(10);
				write24c02(warning0+8, tem%256);
 				write24c02(warning0+9, warningflag);
				warning0+=10;
				}
			}
		 }


		 if(K3 == 0)
		 {
		 	delay(5);
			if(K3 == 0)
			{
				while(!K3);
				beep = 0;
				delay(100);
				beep = 1;
				delay(100);
				beep = 0;
				delay(100);
				beep = 1;
				warningflag = 2;
				warning1++;
				write24c02(warning0, miao);
			//	DelayMs(10);
				write24c02(warning0+1, fen);
			//	DelayMs(10);
				write24c02(warning0+2, shi);
			//	DelayMs(10);
				write24c02(warning0+3, ri);
			//	DelayMs(10);
				write24c02(warning0+4, yue);
			//	DelayMs(10);
				write24c02(warning0+5, nian);
			//	DelayMs(10);
				write24c02(warning0+6, week);
			//	DelayMs(10);
				write24c02(warning0+7, tem/256);
				write24c02(warning0+8, tem%256);
 				write24c02(warning0+9, warningflag);
				warning0+=10;
			}
		 }
		 if(warning0 >=25 )	warning0 = 0;
		 if(K4 == 0)
		 {
		 	delay(5);
			if(K4 == 0)
			{
				while(!K4);
				
//				if(K4cnt >= 5) K4cnt = 0;

				TR0 = 0;
//				write_1602com(Row1+0x0b);//写入光标位置
//   				write_1602com(0x0f);//设置光标为闪烁
				if(warningflag == 0)
				{
						write_1602com(0x01);
						write_1602com(Row0+2);//字符写入的位置
						
						for(count=0; count<7; count++)
						{
							write_1602dat(nothing[count]);
						}
						delay(500);
						
						write_1602com(0x01);
						write_1602com(Row0+2);//字符写入的位置
						
						for(a=0; a<14; a++)
						{
							write_1602dat(tab1[a]);
						}
						write_1602com(Row1 + 4);
						for(a=0; a<8; a++)
						{
							write_1602dat(tab2[a]);
						}
						K4cnt = 0;
						TR0 = 1; 
						
				}
				else
				{
					K4cnt++;
					if(K4cnt>warning1) 
					{
						
						K4cnt = 0;
						write_1602com(0x01);
						write_1602com(Row0+2);//字符写入的位置
						
						for(a=0; a<14; a++)
						{
							write_1602dat(tab1[a]);
						}
						write_1602com(Row1 + 4);
						for(a=0; a<8; a++)
						{
							write_1602dat(tab2[a]);
						}
						write_1602com(0x80+0x40);
						write_1602dat(' ');
						write_1602com(0x80);
						write_1602dat(' ');
//						write_1602com(Row0);
//						write_1602dat(' ');
//						write_1602com(Row1);
//						write_1602dat(' ');
						TR0 = 1;
					}
						write_1602com(0x01);
						write_1602com(Row0+2);//字符写入的位置
						for(count=0; count<14; count++)
						{
							write_1602dat(tab1[count]);
						}
						write_1602com(Row1 + 4);
						for(count=0; count<8; count++)
						{
							write_1602dat(tab2[count]);
						}
						write_1602com(Row0);
						write_1602dat('0'+K4cnt);
						write_sfm(10, read24c02(((K4cnt-1)*10)+0));	//second
						write_sfm(7, read24c02(((K4cnt-1)*10)+1));	//min
						write_sfm(4, read24c02(((K4cnt-1)*10)+2));	//hour
						write_nyr(10, read24c02(((K4cnt-1)*10)+3));
						write_nyr(7, read24c02(((K4cnt-1)*10)+4));
						write_nyr(4, read24c02(((K4cnt-1)*10)+5));
						write_week(read24c02(((K4cnt-1)*10)+6));
						writetemp(read24c02(((K4cnt-1)*10)+7)*256+read24c02(((K4cnt-1)*10)+8));
						writewarn(read24c02(((K4cnt-1)*10)+9));					
					
				}
				


	
			}
		 }

	}

	if(K1_CNT!=0)//当K1按下以下。再按以下键才有效
	{
		if(K2==0)  //上调键
		{
			delay(5);
			if(K2==0)
			{
				while(!K2);
				switch(K1_CNT)
				{
					case 1:	miao++;
							if(miao==60)
								miao = 0;
							write_sfm(0x0a,miao);//写入新的秒数
							temp = (miao)/10*16+(miao)%10;
		   					write_1302(0x8e,0x00);
		   					write_1302(0x80,0x80|temp);
		   					write_1302(0x8e,0x80);
							write_1602com(Row1+0x0b);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
							break;
					case 2:	fen++;
							if(fen==60)
								fen = 0;
							write_sfm(0x07,fen);
							temp=(fen)/10*16+(fen)%10;
						   	write_1302(0x8e,0x00);
						   	write_1302(0x82,temp);
						   	write_1302(0x8e,0x80);
							write_1602com(Row1+8);
							break;
					case 3:	shi++;
							if(shi==24)
								shi = 0;
							write_sfm(4,shi);
							temp = (shi)/10*16+(shi)%10;
						   	write_1302(0x8e,0x00);
						   	write_1302(0x84,temp);
						   	write_1302(0x8e,0x80);
							write_1602com(Row1+5);
							break;
					case 4:	week++;
							if(week==8)
								week = 0;
							write_week(week);
							temp = (week)/10*16+(week)%10;
							write_1302(0x8e,0x00);
						   	write_1302(0x8a,temp);
						   	write_1302(0x8e,0x80);
							write_1602com(Row0+0x0f);
							break;
					case 5:	ri++;
							if(ri==31)
								ri = 0;
							write_nyr(10,ri);
							temp = (ri)/10*16+(ri)%10;
							write_1302(0x8e,0x00);
							write_1302(0x86,temp);
							write_1302(0x8e,0x80);
							write_1602com(Row0+11);
							break;
					case 6:	yue++;
							if(yue==13)
								yue = 1;
							write_nyr(7,yue);
							temp = (yue)/10*16+(yue)%10;
							write_1302(0x8e,0x00);
							write_1302(0x88,temp);
							write_1302(0x8e,0x80);
							write_1602com(Row0+8);
							break;
					case 7:	nian++;
							temp = (nian)/10*16+(nian)%10;
							write_nyr(4,nian);
							write_1302(0x8e,0x00);
						   	write_1302(0x8c,temp);
						   	write_1302(0x8e,0x80); 
							write_1602com(Row0+5);
							break;
			

				}

			}

		}
		if(K3==0)
		{
			delay(5);
			if(K3==0)
			{
				while(!K3);
				switch(K1_CNT)
				{
					case 1:	miao--;
							if(miao==-1)
								miao = 59;
							write_sfm(0x0a,miao);//写入新的秒数
							write_1602com(Row1+0x0b);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
							break;
					case 2:	fen--;
							if(fen==-1)
								fen = 59;
							write_sfm(7,fen);
							temp = (fen)/10*16+(fen)%10;
						   	write_1302(0x8e,0x00);
						   	write_1302(0x82,temp);
						   	write_1302(0x8e,0x80);
							write_1602com(Row1+8);
							break;
					case 3:	shi--;
							if(shi==-1)
								shi = 23;
							write_sfm(4,shi);
							temp = (shi)/10*16+(shi)%10;
						   	write_1302(0x8e,0x00);
						   	write_1302(0x84,temp);
						   	write_1302(0x8e,0x80);
							write_1602com(Row1+5);
							break;
					case 4:	week--;
							if(week==-1)
								week = 7;
							write_week(week);
							temp = (week)/10*16+(week)%10;
							write_1302(0x8e,0x00);
						   	write_1302(0x8a,temp);
						   	write_1302(0x8e,0x80);
						//	write_week(week);
							write_1602com(Row0+0x0f);
							break;
					case 5:	ri--;
							if(ri==-1)
								ri = 30;
							write_nyr(10,ri);
							temp = (ri)/10*16+(ri)%10;
						   	write_1302(0x8e,0x00);
						   	write_1302(0x86,temp);
						   	write_1302(0x8e,0x80);
							write_1602com(Row0+11);
							break;
					case 6:	yue--;
							if(yue==-1)
								yue = 12;
							write_nyr(7,yue);
							temp = (yue)/10*16+(yue)%10;
						   	write_1302(0x8e,0x00);
						   	write_1302(0x88,temp);
						   	write_1302(0x8e,0x80);
							write_1602com(Row0+8);
							break;	
					case 7:	nian--;
		 					if(nian==-1)
								nian = 99;
							temp = (nian)/10*16+(nian)%10;
							write_nyr(4,nian);
							write_1302(0x8e,0x00);
						   	write_1302(0x8c,temp);
						   	write_1302(0x8e,0x80);
							
							write_1602com(Row0+5);
							break;
			
				}
			}
		}
	}
}

void Timer0_init()
{
	TMOD = 0x11;
	TH0  = 0;
	TL0  = 0;
	EA   = 1;
	ET0  = 1;
	TR0  = 1;
}


void main()
{
	init();
	lcd_init();
	ds1302_init();
	Timer0_init();
	write_1302(0x8e,0x00);
   	write_1302(0x8c,(17)/10*16+(17)%10);
   	write_1302(0x8e,0x80);
	delay1(50);
    write_1302(0x8e,0x00);
   	write_1302(0x8a,4);
   	write_1302(0x8e,0x80);		
	while(1)
	{
		keyscan();
		
	}
}

void timer0() interrupt 1
{
	cnt++;
	if(cnt >= 10)
	{
		cnt = 0;
		write_temp();
	}

	miao = BCD_Decimal(read_1302(0x81));
	fen  = BCD_Decimal(read_1302(0x83));
	shi  = BCD_Decimal(read_1302(0x85));
	ri   = BCD_Decimal(read_1302(0x87));
	yue  = BCD_Decimal(read_1302(0x89));
	nian = BCD_Decimal(read_1302(0x8d));
	week = BCD_Decimal(read_1302(0x8b));

	
		
			
	write_sfm(10, miao);
	write_sfm(7, fen);
	write_sfm(4, shi);
	write_nyr(10, ri);
	write_nyr(7, yue);
	write_nyr(4, nian);
	write_week( week );


}
