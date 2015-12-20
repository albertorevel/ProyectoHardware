/*********************************************************************************************
* Fichero:	main.c
* Autor:
* Descrip:	punto de entrada de C
* Version:  <P4-ARM.timer-leds>
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "stdio.h"

/*--- variables globales ---*/
extern int switch_leds;
extern int state;
int leido;
//extern int start;
/*--- funciones externas ---*/
extern void leds_off();
extern void leds_on();
extern void led1_on();
extern void leds_switch();
extern void timer_init();
extern void timer2_inicializar();
extern void timer2_empezar();
extern void timer2_empezar();
extern void Eint4567_init();
extern void D8Led_init();
extern void D8Led_symbol();
extern void parpadeos();
extern void init_stack();
extern int sudoku9x9();
extern void Lcd_Init();
extern void Lcd_Clr();
extern void Lcd_Active_Clr();
extern void pantalla_inicial();
extern void tablero();
//extern void DoUndef();
//extern void DoDabort();

/*--- declaracion de funciones ---*/
void Main(void);

/*--- codigo de funciones ---*/
void Main(void)
{
	int vacias = 0;
	/* Inicializa controladores */
	sys_init();        // Inicializacion de la placa, interrupciones y puertos
	timer_init();	   // Inicializacion del temporizador
	timer2_inicializar(); //Inicializacion del temporizador2
	Eint4567_init();	// inicializamos los pulsadores. Cada vez que se pulse se verá reflejado en el 8led
	D8Led_init(); // inicializamos el 8led
	init_stack();	//inicializamos la pila de debug
	Lcd_Init(); //	initial LCD controller


	/* Valor inicial de los leds */
	leds_off();
	led1_on();
	/* clear screen */
	Lcd_Clr();
	Lcd_Active_Clr();
	timer2_empezar();

//	DoDabort();
	pantalla_inicial();
	while (1) {

		while (state == 0)
		{
			/* Cambia los leds con cada interrupcion del temporizador */
			if (switch_leds == 1)
			{
				leds_switch();
				switch_leds = 0;
			}
		}
		tablero();
		vacias = sudoku9x9();
		pantalla_inicial();
		putFinal(vacias);
	}
}

void parpadeos(void)
{
	int ledvalor = 0x10;
	leds_on();
	int i = 0;
	while (1)
	{
		if (i == 100000)
		{
			if (ledvalor == 0x10) {
				ledvalor = 0xe;
				leds_off();
			} else {
				ledvalor = 0x10;
				leds_on();
			}
			D8Led_symbol(ledvalor);
			i = 0;

		}
		i++;
	}


}
