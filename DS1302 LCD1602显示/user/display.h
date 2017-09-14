#ifndef _display_2003_7_21_
#define _display_2003_7_21_

sbit wx = P2^6;
sbit dx = P2^7;

void delay()
{
	int x, y;
	for(x=1; x>0; x--)
		for(y=110; y>0; y--);
}

void display( unsigned char d1, unsigned char d2, unsigned char d3, 
			  unsigned char d4, unsigned char d5, unsigned char d6 )
{

	wx = 1;
	P0 = 0x04;
	wx = 0;
	P0 = 0;
	dx = 1;
	P0 = d1;
	dx = 0;
	delay();					   
	dx = 1;	P0 = 0xFF;	dx = 0;

	wx = 1;
	P0 = 0x08;
	wx = 0;
	P0 = 0;
	dx = 1;
	P0 = d2;
	dx = 0;
	delay();
	dx = 1;	P0 = 0xFF;	dx = 0;

	wx = 1;
	P0 = 0x10;
	wx = 0;
	P0 = 0;
	dx = 1;
	P0 = d3;
	dx = 0;
	delay();
	dx = 1;	P0 = 0xFF;	dx = 0;

	wx = 1;
	P0 = 0x20;
	wx = 0;
	P0 = 0;
	dx = 1;
	P0 = d4;
	dx = 0;
	delay();
	dx = 1;	P0 = 0xFF;	dx = 0;

	wx = 1;
	P0 = 0x40;
	wx = 0;
	P0 = 0;
	dx = 1;
	P0 = d5;
	dx = 0;
	delay();
	dx = 1;	P0 = 0xFF;	dx = 0;

	wx = 1;
	P0 = 0x80;
	wx = 0;
	P0 = 0;
	dx = 1;
	P0 = d6;
	dx = 0;
	delay();
	dx = 1;	P0 = 0xFF;	dx = 0;
	
}
#endif
