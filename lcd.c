/*********************************************************************************************
* Fichero:	lcd.c
* Autor:	
* Descrip:	funciones de visualizacion y control LCD
* Version:	<P6-ARM>
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "def.h"
#include "44b.h"
#include "44blib.h"
#include "lcd.h"
#include "bmp.h"
#include "stdlib.h"
#include <stdio.h>
#include <string.h>

/*--- definicion de macros ---*/
#define DMA_Byte  (0)
#define DMA_HW    (1)
#define DMA_Word  (2)
#define DW 		  DMA_Byte		//configura  ZDMA0 como media palabras

// Definimos las posiciones de inicio y final del tablero
static int limitX = SCR_XSIZE-5;
static int limitY = SCR_YSIZE-18;
static int initX = 0;
static int initY = 15;
static int spawnX = 35;
static int spawnY = 23;
//static int tiempo_calculo = 0;
//static int tiempo_total = 0;
	
/*--- variables externas ---*/
extern INT8U g_auc_Ascii8x16[];
extern STRU_BITMAP Stru_Bitmap_gbMouse;
//extern const char *num_ascii[];
extern unsigned int tiempoCalculo;
extern unsigned int tiempoTotal;
/*--- codigo de la funcion ---*/
void Lcd_Init(void)
{       
	rDITHMODE=0x1223a;
	rDP1_2 =0x5a5a;      
	rDP4_7 =0x366cd9b;
	rDP3_5 =0xda5a7;
	rDP2_3 =0xad7;
	rDP5_7 =0xfeda5b7;
	rDP3_4 =0xebd7;
	rDP4_5 =0xebfd7;
	rDP6_7 =0x7efdfbf;

	rLCDCON1=(0)|(1<<5)|(MVAL_USED<<7)|(0x0<<8)|(0x0<<10)|(CLKVAL_GREY16<<12);
	rLCDCON2=(LINEVAL)|(HOZVAL<<10)|(10<<21); 
	rLCDSADDR1= (0x2<<27) | ( ((LCD_ACTIVE_BUFFER>>22)<<21 ) | M5D(LCD_ACTIVE_BUFFER>>1));
 	rLCDSADDR2= M5D(((LCD_ACTIVE_BUFFER+(SCR_XSIZE*LCD_YSIZE/2))>>1)) | (MVAL<<21);
	rLCDSADDR3= (LCD_XSIZE/4) | ( ((SCR_XSIZE-LCD_XSIZE)/4)<<9 );
	// enable,4B_SNGL_SCAN,WDLY=8clk,WLH=8clk,
	rLCDCON1=(1)|(1<<5)|(MVAL_USED<<7)|(0x3<<8)|(0x3<<10)|(CLKVAL_GREY16<<12);
	rBLUELUT=0xfa40;
	//Enable LCD Logic and EL back-light.
	rPDATE=rPDATE&0x0e;
	
	//DMA ISR
	rINTMSK &= ~(BIT_GLOBAL|BIT_ZDMA0);
    pISR_ZDMA0=(int)Zdma0Done;
}

/*********************************************************************************************
* name:		Lcd_Active_Clr()
* func:		clear LCD screen
* para:		none 
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Active_Clr(void)
{
	INT32U i;
	INT32U *pDisp = (INT32U *)LCD_ACTIVE_BUFFER;
	
	for( i = 0; i < (SCR_XSIZE*SCR_YSIZE/2/4); i++ )
	{
		*pDisp++ = WHITE;
	}
}

/*********************************************************************************************
* name:		Lcd_GetPixel()
* func:		Get appointed point's color value
* para:		usX,usY -- pot's X-Y coordinate 
* ret:		pot's color value
* modify:
* comment:		
*********************************************************************************************/
INT8U LCD_GetPixel(INT16U usX, INT16U usY)
{
	INT8U ucColor;

	ucColor = *((INT8U*)(LCD_VIRTUAL_BUFFER + usY*SCR_XSIZE/2 + usX/8*4 + 3 - (usX%8)/2));
	ucColor = (ucColor >> ((1-(usX%2))*4)) & 0x0f;
	return ucColor;
}

/*********************************************************************************************
* name:		Lcd_Active_Clr()
* func:		clear virtual screen
* para:		none 
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Clr(void)
{
	INT32U i;
	INT32U *pDisp = (INT32U *)LCD_VIRTUAL_BUFFER;
	
	for( i = 0; i < (SCR_XSIZE*SCR_YSIZE/2/4); i++ )
	{
		*pDisp++ = WHITE;
	}
}

/*********************************************************************************************
* name:		LcdClrRect()
* func:		fill appointed area with appointed color
* para:		usLeft,usTop,usRight,usBottom -- area's rectangle acme coordinate
*			ucColor -- appointed color value
* ret:		none
* modify:
* comment:	also as clear screen function 
*********************************************************************************************/
void LcdClrRect(INT16 usLeft, INT16 usTop, INT16 usRight, INT16 usBottom, INT8U ucColor)
{
	INT16 i,k,l,m;
	
	INT32U ulColor = (ucColor << 28) | (ucColor << 24) | (ucColor << 20) | (ucColor << 16) | 
				     (ucColor << 12) | (ucColor << 8) | (ucColor << 4) | ucColor;

	i = k = l = m = 0;	
	if( (usRight-usLeft) <= 8 )
	{
		for( i=usTop; i<=usBottom; i++)
		{
			for( m=usLeft; m<=usRight; m++)
			{
				(LCD_PutPixel(m, i, ucColor));
			}
		}	
		return;
	}

	/* check borderline */
	if( 0 == (usLeft%8) )
		k=usLeft;
	else
	{
		k=(usLeft/8)*8+8;
	}
	if( 0 == (usRight%8) )
		l= usRight;
	else
	{
		l=(usRight/8)*8;
	}

	for( i=usTop; i<=usBottom; i++ )
	{
		for( m=usLeft; m<=(k-1); m++ )
		{
			(LCD_PutPixel(m, i, ucColor));
		}
		for( m=k; m<l; m+=8 )
		{
			(*(INT32U*)(LCD_VIRTUAL_BUFFER + i * SCR_XSIZE / 2 + m / 2)) = ulColor;
		}
		for( m=l; m<=usRight; m++ )
		{
			(LCD_PutPixel(m, i, ucColor));
		}
	}
}

/*********************************************************************************************
* name:		Lcd_Draw_Box()
* func:		Draw rectangle with appointed color
* para:		usLeft,usTop,usRight,usBottom -- rectangle's acme coordinate
*			ucColor -- appointed color value
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Draw_Box(INT16 usLeft, INT16 usTop, INT16 usRight, INT16 usBottom, INT8U ucColor)
{
	Lcd_Draw_HLine(usLeft, usRight,  usTop,    ucColor, 3);
	Lcd_Draw_HLine(usLeft, usRight,  usBottom, ucColor, 3);
	Lcd_Draw_VLine(usTop,  usBottom, usLeft,   ucColor, 3);
	Lcd_Draw_VLine(usTop,  usBottom, usRight,  ucColor, 3);
}

/*********************************************************************************************
* name:		Lcd_Draw_Line()
* func:		Draw line with appointed color
* para:		usX0,usY0 -- line's start point coordinate
*			usX1,usY1 -- line's end point coordinate
*			ucColor -- appointed color value
*			usWidth -- line's width
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Draw_Line(INT16 usX0, INT16 usY0, INT16 usX1, INT16 usY1, INT8U ucColor, INT16U usWidth)
{
	INT16 usDx;
	INT16 usDy;
	INT16 y_sign;
	INT16 x_sign;
	INT16 decision;
	INT16 wCurx, wCury, wNextx, wNexty, wpy, wpx;

	if( usY0 == usY1 )
	{
		Lcd_Draw_HLine (usX0, usX1, usY0, ucColor, usWidth);
		return;
	}
	if( usX0 == usX1 )
	{
		Lcd_Draw_VLine (usY0, usY1, usX0, ucColor, usWidth);
		return;
	}
	usDx = abs(usX0 - usX1);
	usDy = abs(usY0 - usY1);
	if( ((usDx >= usDy && (usX0 > usX1)) ||
        ((usDy > usDx) && (usY0 > usY1))) )
    {
        GUISWAP(usX1, usX0);
        GUISWAP(usY1, usY0);
    }
    y_sign = (usY1 - usY0) / usDy;
    x_sign = (usX1 - usX0) / usDx;

    if( usDx >= usDy )
    {
        for( wCurx = usX0, wCury = usY0, wNextx = usX1,
             wNexty = usY1, decision = (usDx >> 1);
             wCurx <= wNextx; wCurx++, wNextx--, decision += usDy )
        {
            if( decision >= usDx )
            {
                decision -= usDx;
                wCury += y_sign;
                wNexty -= y_sign;
            }
            for( wpy = wCury - usWidth / 2;
                 wpy <= wCury + usWidth / 2; wpy++ )
            {
                LCD_PutPixel(wCurx, wpy, ucColor);
            }

            for( wpy = wNexty - usWidth / 2;
                 wpy <= wNexty + usWidth / 2; wpy++ )
            {
                LCD_PutPixel(wNextx, wpy, ucColor);
            }
        }
    }
    else
    {
        for( wCurx = usX0, wCury = usY0, wNextx = usX1,
             wNexty = usY1, decision = (usDy >> 1);
             wCury <= wNexty; wCury++, wNexty--, decision += usDx )
        {
            if( decision >= usDy )
            {
                decision -= usDy;
                wCurx += x_sign;
                wNextx -= x_sign;
            }
            for( wpx = wCurx - usWidth / 2;
                 wpx <= wCurx + usWidth / 2; wpx++ )
            {
                LCD_PutPixel(wpx, wCury, ucColor);
            }

            for( wpx = wNextx - usWidth / 2;
                 wpx <= wNextx + usWidth / 2; wpx++ )
            {
                LCD_PutPixel(wpx, wNexty, ucColor);
            }
        }
    }
}

/*********************************************************************************************
* name:		Lcd_Draw_HLine()
* func:		Draw horizontal line with appointed color
* para:		usX0,usY0 -- line's start point coordinate
*			usX1 -- line's end point X-coordinate
*			ucColor -- appointed color value
*			usWidth -- line's width
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Draw_HLine(INT16 usX0, INT16 usX1, INT16 usY0, INT8U ucColor, INT16U usWidth)
{
	INT16 usLen;

    if( usX1 < usX0 )
    {
        GUISWAP (usX1, usX0);
    }

    while( (usWidth--) > 0 )
    {
        usLen = usX1 - usX0 + 1;
        while( (usLen--) > 0 )
        {
        	LCD_PutPixel(usX0 + usLen, usY0, ucColor);
        }
        usY0++;
    }
}

/*********************************************************************************************
* name:		Lcd_Draw_VLine()
* func:		Draw vertical line with appointed color
* para:		usX0,usY0 -- line's start point coordinate
*			usY1 -- line's end point Y-coordinate
*			ucColor -- appointed color value
*			usWidth -- line's width
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Draw_VLine (INT16 usY0, INT16 usY1, INT16 usX0, INT8U ucColor, INT16U usWidth)
{
	INT16 usLen;

    if( usY1 < usY0 )
    {
        GUISWAP (usY1, usY0);
    }

    while( (usWidth--) > 0 )
    {
        usLen = usY1 - usY0 + 1;
        while( (usLen--) > 0 )
        {
        	LCD_PutPixel(usX0, usY0 + usLen, ucColor);
        }
        usX0++;
    }
}

/*********************************************************************************************
* name:		Lcd_DspAscII8x16()
* func:		display 8x16 ASCII character string 
* para:		usX0,usY0 -- ASCII character string's start point coordinate
*			ForeColor -- appointed color value
*			pucChar   -- ASCII character string
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_DspAscII8x16(INT16U x0, INT16U y0, INT8U ForeColor, INT8U * s)
{
	INT16 i,j,k,x,y,xx;
	INT8U qm;
	INT32U ulOffset;
	INT8 ywbuf[16],temp[2];
    
	for( i = 0; i < strlen((const char*)s); i++ )
	{
		if( (INT8U)*(s+i) >= 161 )
		{
			temp[0] = *(s + i);
			temp[1] = '\0';
			return;
		}
		else
		{
			qm = *(s+i);
			ulOffset = (INT32U)(qm) * 16;		//Here to be changed tomorrow
			for( j = 0; j < 16; j ++ )
			{
				ywbuf[j] = g_auc_Ascii8x16[ulOffset + j];
            }

            for( y = 0; y < 16; y++ )
            {
            	for( x = 0; x < 8; x++ ) 
               	{
                	k = x % 8;
			    	if( ywbuf[y]  & (0x80 >> k) )
			       	{
			       		xx = x0 + x + i*8;
			       		LCD_PutPixel(xx, y + y0, (INT8U)ForeColor);
			       	}
			   	}
            }
		}
	}
}

/*********************************************************************************************
* name:		ReverseLine()
* func:		Reverse display some lines 
* para:		ulHeight -- line's height
*			ulY -- line's Y-coordinate
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void ReverseLine(INT32U ulHeight, INT32U ulY)
{
	INT32U i, j, temp;
	
	for( i = 0; i < ulHeight; i++ )
	{
		for( j = 0; j < (SCR_XSIZE/4/2) ; j++ )
		{
			temp = *(INT32U*)(LCD_VIRTUAL_BUFFER + (ulY+i)*SCR_XSIZE/2 + j*4);
			temp ^= 0xFFFFFFFF;
			*(INT32U*)(LCD_VIRTUAL_BUFFER + (ulY+i)*SCR_XSIZE/2 + j*4) = temp;
		}
	}
}

/*********************************************************************************************
* name:		Zdma0Done()
* func:		LCD dma interrupt handle function
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
static INT8U ucZdma0Done=1;	//When DMA is finish,ucZdma0Done is cleared to Zero
void Zdma0Done(void)
{
	rI_ISPC=BIT_ZDMA0;	    //clear pending
	ucZdma0Done=0;
}

/*********************************************************************************************
* name:		Lcd_Dma_Trans()
* func:		dma transport virtual LCD screen to LCD actual screen
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Dma_Trans(void)
{
	INT8U err;
	
	ucZdma0Done=1;
	//#define LCD_VIRTUAL_BUFFER	(0xc400000)
	//#define LCD_ACTIVE_BUFFER	(LCD_VIRTUAL_BUFFER+(SCR_XSIZE*SCR_YSIZE/2))	//DMA ON
	//#define LCD_ACTIVE_BUFFER	LCD_VIRTUAL_BUFFER								//DMA OFF
	//#define LCD_BUF_SIZE		(SCR_XSIZE*SCR_YSIZE/2)
	//So the Lcd Buffer Low area is from LCD_VIRTUAL_BUFFER to (LCD_ACTIVE_BUFFER+(SCR_XSIZE*SCR_YSIZE/2))
	rNCACHBE1=(((unsigned)(LCD_ACTIVE_BUFFER)>>12) <<16 )|((unsigned)(LCD_VIRTUAL_BUFFER)>>12);
  	rZDISRC0=(DW<<30)|(1<<28)|LCD_VIRTUAL_BUFFER; // inc
  	rZDIDES0=( 2<<30)  |(1<<28)|LCD_ACTIVE_BUFFER; // inc
        rZDICNT0=( 2<<28)|(1<<26)|(3<<22)|(0<<20)|(LCD_BUF_SIZE);
        //                      |            |            |             |            |---->0 = Disable DMA
        //                      |            |            |             |------------>Int. whenever transferred
        //                      |            |            |-------------------->Write time on the fly
        //                      |            |---------------------------->Block(4-word) transfer mode
        //                      |------------------------------------>whole service
	//reEnable ZDMA transfer
  	rZDICNT0 |= (1<<20);		//after ES3
    rZDCON0=0x1; // start!!!  

	//Delay(500);
	while(ucZdma0Done);		//wait for DMA finish
}

/*********************************************************************************************
* name:		Lcd_Test()
* func:		LCD test function
* para:		none
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void Lcd_Test(void)
{
	/* initial LCD controller */
	Lcd_Init();
	/* clear screen */
	Lcd_Clr();
	Lcd_Active_Clr();

	/* draw rectangle pattern */ 
    #ifdef Eng_v // english version
	Lcd_DspAscII8x16(10,0,DARKGRAY,"Embest S3CEV40 ");
	#else
//	Lcd_DspHz16(10,0,DARKGRAY,"英蓓特三星实验评估板");
	#endif
	Lcd_DspAscII8x16(10,20,BLACK,"Codigo del puesto: ");
	Lcd_Draw_Box(10,40,310,230,14);
	Lcd_Draw_Box(20,45,300,225,13);
	Lcd_Draw_Box(30,50,290,220,12);
	Lcd_Draw_Box(40,55,280,215,11);
	Lcd_Draw_Box(50,60,270,210,10);
	Lcd_Draw_Box(60,65,260,205,9);
	Lcd_Draw_Box(70,70,250,200,8);
	Lcd_Draw_Box(80,75,240,195,7);
	Lcd_Draw_Box(90,80,230,190,6);
	Lcd_Draw_Box(100,85,220,185,5);
	Lcd_Draw_Box(110,90,210,180,4);
	Lcd_Draw_Box(120,95,200,175,3);
	Lcd_Draw_Box(130,100,190,170,2);
	BitmapView(125,135,Stru_Bitmap_gbMouse);
	Lcd_Dma_Trans();

}

void pantalla_inicial () {
	Lcd_Clr();
	Lcd_Active_Clr();
	Lcd_DspAscII8x16(140,15,BLACK,"SUDOKU");
	Lcd_DspAscII8x16(10,70,BLACK,"PULSE UN BOTON PARA JUGAR");
	Lcd_DspAscII8x16(0,90,BLACK,"Instrucciones: ");
	Lcd_DspAscII8x16(0,100,BLACK," - Primero debera elegir la fila");
	Lcd_DspAscII8x16(0,110,BLACK," - Segundo debera elegir la columna");
	Lcd_DspAscII8x16(0,120,BLACK," - Finalmente debera elegir el valor");
	Lcd_DspAscII8x16(0,130,BLACK," - Borre el valor de la casilla con 0");
	Lcd_DspAscII8x16(0,140,BLACK," - Boton izquierdo para cambiar un valor");
	Lcd_DspAscII8x16(0,150,BLACK," - Boton derecho para confirmar el valor");
	Lcd_DspAscII8x16(0,160,BLACK," - Finalice el juego eligiendo la fila A");
	Lcd_DspAscII8x16(0,170,BLACK," - El juego acabara con sudoku completo");
	Lcd_DspAscII8x16(0,180,BLACK," - Las pistas se marcan con un recuadro");
	Lcd_DspAscII8x16(0,190,BLACK," - Los errores se marcan en negro");
	Lcd_DspAscII8x16(40,225,BLACK,"Alberto Revel y Ruben Quilez");
	Lcd_Dma_Trans();
}


void* reverse_string(char *str)
{
char temp;
size_t len = strlen(str) - 1;
size_t i;
size_t k = len;

for(i = 0; i < len; i++)
{
	temp = str[k];
	str[k] = str[i];
	str[i] = temp;
	k--;

	    /* As 2 characters are changing place for each cycle of the loop
	       only traverse half the array of characters */
	    if(k == (len / 2))
	    {
	   	    break;
	    }
    }
}

char* itoa(int num, char* str, int base)
{
    int i = 0;
    int isNegative = 0;

    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = 1;
        num = -num;
    }

    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    // If number is negative, append '-'
    if (isNegative==1)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse_string(str);

    return str;
}
void mostrarTiempo() {
	LcdClrRect(0,0,SCR_XSIZE,initY-1,0);
	char buffer [5];
	char buffer2 [5];
	itoa (tiempoTotal/1000000,buffer, 10);
	itoa (tiempoCalculo/1000,buffer2, 10);
	char str[80];
	strcpy(str, "Tiempo Total:");
	strcat(str, buffer);
	strcat(str, "s. Tiempo Calc:");
	strcat(str, buffer2);
	strcat(str, "ms.");
	Lcd_DspAscII8x16(0,0,BLACK,str);
	Lcd_Dma_Trans();
}

void tablero () {
	// Borramos la pantalla
	Lcd_Clr();
	Lcd_Active_Clr();
	// Pintamos el recuadro exterior en el que ira dibujado el tablero

    mostrarTiempo();
	Lcd_Draw_Box(initX,initY,limitX,limitY,14);
	// Pintamos las lineas (Horizontales y verticales) que conformaran el tablero e informaci髇
	int i = 1;
	for ( i = 1; i < 9; i++) {
		if( i % 3 == 0)
		{
			Lcd_Draw_HLine(initX,limitX,initY + (i*spawnY),14,3);
			Lcd_Draw_VLine(initY,limitY,initX + (i*spawnX),14,3);
		}
		else
		{
		Lcd_Draw_HLine(initX,limitX,initY + (i*spawnY),14,1);
		Lcd_Draw_VLine(initY,limitY,initX + (i*spawnX),14,1);
		}
	}
	Lcd_DspAscII8x16(0,SCR_YSIZE-15,BLACK,"INTRODUZCA FILA A PARA ACABAR LA PARTIDA");
	Lcd_Dma_Trans();
}

void putData(char * valor, int x, int y) {
	int x0 = initX + spawnX * y;
	int y0 = initY + spawnY * x;
	Lcd_DspAscII8x16(x0 + 15,y0 + 7,BLACK,valor);
}
void putError(char * valor, int x, int y, int pista) {
	int x0 = initX + spawnX * y;
	int y0 = initY + spawnY * x;
	int x1 = x0 + spawnX;
	int y1 = y0 + spawnY;
	LcdClrRect(x0 + 1,y0 + 1,x1 - 1,y1 - 1,14);
	if (pista==1) {
		Lcd_Draw_Box(x0 + 5,y0 + 2,x1 - 5,y1 - 3,5);
	}
	Lcd_DspAscII8x16(x0 + 15,y0 + 7,WHITE,valor);

}
void putPista(int x, int y) {
	int x0 = initX + spawnX * y;
	int y0 = initY + spawnY * x;
	int x1 = x0 + spawnX;
	int y1 = y0 + spawnY;
	Lcd_Draw_Box(x0 + 5,y0 + 2,x1 - 5,y1 - 3,14);
}

void putCandidates(int candidatos, int x, int y) {
	int x0 = initX + spawnX * y + 6; //Add dots margin (in-cell)
	int y0 = initY + spawnY * x - 4;
	int xcell = 9;
	int ycell = 7;

	int i = 1;
	for ( i = 1; i < 10; i++) {
		// chek if it's in the mask
		if(((1 << (i - 1)) & candidatos) != 0) {
			Lcd_DspAscII8x16(x0,y0,BLACK,"o");
		}

		// move position ptr
		if( i % 3 == 0)
		{
			x0 -= (xcell * 2);
			y0 += ycell;
		}
		else
		{
			x0 += xcell;
		}
	}

}

void putFinal(int celdas_vacias)
{
	
	mostrarTiempo();

	if(celdas_vacias == 0) {
		Lcd_DspAscII8x16(0,40,BLACK," ENHORABUENA, HA COMPLETADO EL SUDOKU");
	} else {
		Lcd_DspAscII8x16(0,40,BLACK,"Juego abandonado.");
	}

	Lcd_Dma_Trans();
}

