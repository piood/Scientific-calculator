#include<reg51.h>
#include<math.h>
#include <stdio.h>
//Function declarations
void cct_init(void);
void delay(int);
void lcdinit(void);
void writecmd(int);
void writedata(char);
void Return(void);
char READ_SWITCHES(void);
char get_key(void);
void Wrapline(void);
//********************
sbit RowA = P1^0;
sbit RowB = P1^1;
sbit RowC = P1^2;
sbit RowD = P1^3;

sbit C1   = P1^4;    
sbit C2   = P1^5;    		
sbit C3   = P1^6;     
sbit C4   = P1^7;

sbit E    = P3^6;    
sbit RS   = P3^7;     

sbit row1 = P3^0;
sbit row2 = P3^1;
sbit row3 = P3^2;

sbit col1 = P3^3;
sbit col2 = P3^4;
sbit col3 = P3^5;

#define PI 3.14159265
int i;
int Ctop=-1;
char cstack[10];
unsigned int length=0;

int Frontflag=-1;//前一位标志
int radixblen=0;

void Cstackpush(char Cstring)
{
    Ctop++;
	cstack[Ctop]=Cstring;
}
char Cstackpop(void)
{
    Ctop--;
	return cstack[Ctop+1];
}
char Cstacktop(void)
{
	return cstack[Ctop];
}

int Etop=-1;
long double estack[18];

void Estackpush(long double Estring)
{
    Etop++;
	estack[Etop]=Estring;
}
long double Estackpop(void)
{
    Etop--;
	return estack[Etop+1];
}
long double Estacktop(void)
{
	return estack[Etop];
}

void Fourpop(void)
{
	while((Ctop!=-1)&&(Cstacktop()=='+'||Cstacktop()=='-'||Cstacktop()=='x'||Cstacktop()=='/'))
	{
		Estackpush(Cstackpop());
	}	
}

void Bracketpop(void)
{
	while(Cstacktop()=='+'||Cstacktop()=='-'||Cstacktop()=='x'||Cstacktop()=='/')
	{
		Estackpush(Cstackpop());
	}
	if(Cstacktop()=='(') Cstackpop();
	else Estackpush(Cstackpop());
}

void Ifoutput(long double m)
{
	int a;
    if(m<0) { writedata('-'); length++; Wrapline(); m*=-1; } 
	Ctop=0;
	for(i=1; i<5; i++){
		if(i==1)
		{
			a=m/1000000000000;
			m-=(a*1000000000000);
		}
		else if(i==2)
		{
			a=m/100000000;
			m-=(a*100000000);
		}
		else if(i==3)
		{
			a=m/10000;
			m-=(a*10000);
		}
		else 
		{
			a=m;
			m-=a;
		}
		if(a||Ctop)
		{
			while(a)
			{
				Estackpush(a%10);
				a/=10;	
			}
			if(Ctop){
				for(radixblen=0; radixblen<(4-Etop); radixblen++)
				{
					writedata('0'); length++; Wrapline();	
				}
			}
			while(Etop)
			{
				writedata(Estackpop()+'0'); length++; Wrapline(); Ctop=1;	
			}
		}
	}
	if(Ctop==0) { writedata('0'); length++; Wrapline(); }
	if(m>0.000000001) { writedata('.'); length++; Wrapline(); }
	if(m>0.000000001)
	{
		m*=100000000;
		a=m/10000;
		m-=a*10000;
		while(a) 
		{
			Estackpush(a%10);
			a/=10;
		}
		for(radixblen=0; radixblen<(4-Etop); radixblen++)
		{
			writedata('0'); length++; Wrapline();	
		}
		while(Etop) { if(Estacktop()>0) writedata(Estackpop()+'0'); else  { writedata('0'); Estackpop();} length++; Wrapline();}
		a=m;
		m-=a;
		while(a) 
		{
			Estackpush(a%10);
			a/=10;
		}
		for(radixblen=0; radixblen<(4-Etop); radixblen++)
		{
			writedata('0'); length++; Wrapline();	
		}
		while(Etop) { if(Estacktop()>0) writedata(Estackpop()+'0'); else  { writedata('0'); Estackpop();}  length++; Wrapline();}	
	}	
}

void Dataprocess(void)
{	
    Frontflag=0;
	i=1;
	Ctop=0;
	while(i<=Etop)
	{
		if(estack[i]=='+'||estack[i]=='-'||estack[i]=='x'||estack[i]=='/')
		{
			Frontflag=i-1;
			while(estack[Frontflag]=='z'){Frontflag--;}
			Ctop=Frontflag-1;
			while(estack[Ctop]=='z'){Ctop--;}
			if(estack[i]=='+') { estack[Frontflag]=1.0*estack[Ctop]+estack[Frontflag]; estack[Ctop]='z'; }
			else if(estack[i]=='-') { estack[Frontflag]=1.0*estack[Ctop]-estack[Frontflag]; estack[Ctop]='z';}
			else if(estack[i]=='x') { estack[Frontflag]=1.0*estack[Ctop]*estack[Frontflag]; estack[Ctop]='z';}
			else if(estack[i]=='/') { estack[Frontflag]=1.0*estack[Ctop]/estack[Frontflag]; estack[Ctop]='z';}
			estack[i]='z';
		}
		else if(estack[i]=='s'||estack[i]=='c'||estack[i]=='l'||estack[i]=='e'||estack[i]=='t')
		{
			Frontflag=i-1;
			while(Frontflag>=0&&estack[Frontflag]=='z'){Frontflag--;}
			if(estack[i]=='s') 	{ estack[Frontflag]=sin(estack[Frontflag]*(PI/180)); }
			else if(estack[i]=='c')  {estack[Frontflag]=cos(estack[Frontflag]*(PI/180)); }
			else if(estack[i]=='l')  { estack[Frontflag]=log(estack[Frontflag]); }
			else if(estack[i]=='e')  { estack[Frontflag]=exp(estack[Frontflag]); }
			else if(estack[i]=='t')  { estack[Frontflag]=sqrt(estack[Frontflag]); }
			estack[i]='z'; 
		}
		else i++;
	}
	estack[0]=estack[Frontflag];
	Etop=0;
	Ifoutput(estack[0]);				                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
}
	

void Wrapline(void)
{
    if(length==20) writecmd(0xc0);
	else if(length==40) writecmd(0x80+20);
	else if(length==60) writecmd(0xc0+20);
}
void Lstringinto(char key)
{
	writedata(key);
	length++;
	Wrapline();
}
void Lstringdeal(char key)
{	
	if(key=='s') { Lstringinto('s'); Lstringinto('i'); Lstringinto('n'); Lstringinto('('); }
	else if(key=='c') {Lstringinto('c'); Lstringinto('o'); Lstringinto('s'); Lstringinto('(');}
	else if(key=='l') {Lstringinto('l'); Lstringinto('o'); Lstringinto('g'); Lstringinto('(');}
	else if(key=='e') {Lstringinto('e'); Lstringinto('x'); Lstringinto('p'); Lstringinto('(');}
	else {Lstringinto('s'); Lstringinto('q'); Lstringinto('r'); Lstringinto('t'); Lstringinto('(');}
	Cstackpush(key);
}			
void Stringdeal(void)
{
    char key;
	key = get_key();
	if(key>='0'&&key<='9') 
	{
	writedata(key);
	length++;
	if(Frontflag==1){ estack[Etop]=estack[Etop]*10+(key-'0'); radixblen=0;}
	else if(Frontflag==3)
	{
	    long double xyz=1.0*(key-'0');
		radixblen++;
		for(i=0; i<radixblen; i++) xyz*=0.1;
		estack[Etop]+=xyz;
	} 
	else { Estackpush(key-48); Frontflag=1; radixblen=0;}
	}
	else if(key=='+'||key=='-'||key=='x'||key=='/') 
	{
	writedata(key); length++;
	if(Ctop==-1) Cstackpush(key);
	else if((key=='+'||key=='-')&&(Cstacktop()=='+'||Cstacktop()=='-'||Cstacktop()=='x'||Cstacktop()=='/')){Fourpop(); Cstackpush(key);}
	else Cstackpush(key);
	Frontflag=2;
	radixblen=0;
	}
	else if(key=='o') { writecmd(0x01); writecmd(0x80); length=0; Frontflag=-1; radixblen=0; Etop=-1; Ctop=-1; }
	else if(key=='b') 
	{
		if(length!=20&&length!=40&&length!=60){
	 		writecmd(0x04); writedata(' ');writecmd(0x06); writedata(' '); length--;
	 		writecmd(0x04); writedata(' ');writecmd(0x06);}
		else if(length==20){
		    writecmd(0x80+20);writecmd(0x04); writedata(' ');writecmd(0x06); writedata(' '); length--;
	 		writecmd(0x04); writedata(' ');writecmd(0x06);}
		else if(length==40){
		    writecmd(0xc0+20);writecmd(0x04); writedata(' ');writecmd(0x06); writedata(' '); length--;
	 		writecmd(0x04); writedata(' ');writecmd(0x06);}
		else if(length==60){
			writecmd(0x80+40);writecmd(0x04); writedata(' ');writecmd(0x06); writedata(' '); length--;
	 		writecmd(0x04); writedata(' ');writecmd(0x06);}
	}
	else if(key=='(') { writedata(key); length++; Cstackpush(key); Frontflag=2; radixblen=0;}
	else if(key==')') { writedata(key); length++; Bracketpop(); Frontflag=2; radixblen=0; }
	else if(key=='.') { writedata(key); length++; Frontflag=3; }
	else if(key=='=')
	{
	    writedata(key); length++;
		Wrapline();
		while(Ctop!=-1){
			Estackpush(Cstackpop());
		}
		Dataprocess();
	}
	else Lstringdeal(key);
	Wrapline();  
}
int main(void)
{  
   //char keyy;              // key char for keeping record of pressed key
   cct_init();              // Make input and output pins as required
   lcdinit();               // Initilize LCD
   while(1)
   { 
	 Stringdeal();
   }
}


void cct_init(void)
{
	P0 = 0xf0;   //not used
	P1 = 0xf0;   //used for generating outputs and taking inputs from Keypad
	P2 = 0x00;   //used as data port for LCD
	P3 = 0x38;   //used for RS and E    
}

void delay(int a)
{
   int i;
   for(i=0;i<a;i++);   //null statement
}

void writedata(char t)
{
   RS = 1;             // This is data
   P2 = t;             //Data transfer
   E  = 1;             // => E = 1
   delay(150);
   E  = 0;             // => E = 0
   delay(150);
}


void writecmd(int z)
{
   RS = 0;             // This is command
   P2 = z;             //Data transfer
   E  = 1;             // => E = 1
   delay(150);
   E  = 0;             // => E = 0
   delay(150);
}

void lcdinit(void)
{
  ///////////// Reset process from datasheet /////////
     delay(15000);
   writecmd(0x30);
     delay(4500);
   writecmd(0x30);
     delay(300);
   writecmd(0x30);
     delay(650);
  /////////////////////////////////////////////////////
   writecmd(0x38);    //function set
   writecmd(0x0c);    //display on,cursor off,blink off
   writecmd(0x01);    //clear display
   writecmd(0x06);    //entry mode, set increment
   writecmd(0x0f);
}

void Return(void)     //Return to 0 location on LCD
{
  writecmd(0x02);
    delay(1500);
}

char READ_SWITCHES(void)	
{	
	RowA = 0; RowB = 1; RowC = 1; RowD = 1; 	//Test Row A

	if (C1 == 0) { delay(500); while (C1==0); return '7'; }
	if (C2 == 0) { delay(500); while (C2==0); return '8'; }
	if (C3 == 0) { delay(500); while (C3==0); return '9'; }
	if (C4 == 0) { delay(500); while (C4==0); return '/'; }

	RowA = 1; RowB = 0; RowC = 1; RowD = 1; 	//Test Row B

	if (C1 == 0) { delay(500); while (C1==0); return '4'; }
	if (C2 == 0) { delay(500); while (C2==0); return '5'; }
	if (C3 == 0) { delay(500); while (C3==0); return '6'; }
	if (C4 == 0) { delay(500); while (C4==0); return 'x'; }
	
	RowA = 1; RowB = 1; RowC = 0; RowD = 1; 	//Test Row C

	if (C1 == 0) { delay(500); while (C1==0); return '1'; }
	if (C2 == 0) { delay(500); while (C2==0); return '2'; }
	if (C3 == 0) { delay(500); while (C3==0); return '3'; }
	if (C4 == 0) { delay(500); while (C4==0); return '-'; }
	
	RowA = 1; RowB = 1; RowC = 1; RowD = 0; 	//Test Row D

	if (C1 == 0) { delay(500); while (C1==0); return 'o'; }
	if (C2 == 0) { delay(500); while (C2==0); return '0'; }
	if (C3 == 0) { delay(500); while (C3==0); return '='; }
	if (C4 == 0) { delay(500); while (C4==0); return '+'; }

	return 'n';           	// Means no key has been pressed
}

char READ(void)	
{	
	row1 = 0; row2 = 1; row3 = 1; 	//Test Row A

	if (col1 == 0) { delay(500); while (col1==0); return 's'; }
	if (col2 == 0) { delay(500); while (col2==0); return 'c'; }
	if (col3 == 0) { delay(500); while (col3==0); return 'l'; }

	row1 = 1; row2 = 0; row3 = 1; 	//Test Row B

	if (col1 == 0) { delay(500); while (col1==0); return 'e'; }
	if (col2 == 0) { delay(500); while (col2==0); return '('; }
	if (col3 == 0) { delay(500); while (col3==0); return ')'; }
	
	row1 = 1; row2 = 1; row3 = 0; 	//Test Row C

	if (col1 == 0) { delay(500); while (col1==0); return '.'; }
	if (col2 == 0) { delay(500); while (col2==0); return 't'; }
	if (col3 == 0) { delay(500); while (col3==0); return 'b'; }

	return 'n';           	// Means no key has been pressed
}

char get_key(void)           //get key from user
{
	char key = 'n';
	char key1 = 'n';              //assume no key pressed
	while(key=='n'&key1=='n'){
	    key = READ_SWITCHES();              //wait untill a key is pressed
		key1 = READ();//scan the keys again and again	}
    }
    if(key=='n') return key1;                  //when key pressed then return its value
	else return key;
}