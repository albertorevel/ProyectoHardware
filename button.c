/*********************************************************************************************
* Fichero:	button.c
* Autor:
* Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
* Version:
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "def.h"

/*--- variables globales ---*/
/* int_count la utilizamos para sacar un número por el 8led.
  Cuando se pulsa un botón sumamos y con el otro restamos. ¡A veces hay rebotes! */
volatile int state = 0;
volatile int fila = 0;
volatile int columna = 0;
volatile int valor = 1;

static volatile int int_count = 0;
static volatile int retardo = 0;
static volatile int soltado = 0;
static volatile int retardo_trd = 0;
static volatile int retardo_trp = 0;
static volatile int tiempo_inicio = 0;
static volatile int waitState = 0;
//unsigned int mantener= -1;
//int temp = 0;

/*--- declaracion de funciones ---*/
void Eint4567_ISR(void) __attribute__((interrupt("IRQ")));
void Eint4567_init(void);
extern void D8Led_symbol(int value); // declaramos la función que escribe en el 8led
extern void leds_off();
extern void boton_pulsado(void);
extern int timer2_leer();
extern void push_debug(int ID_evento, int auxData);
void iniciarEspera(unsigned int trp, unsigned int trd);
int esperaFinalizada(void);

/*--- codigo de funciones ---*/
void Eint4567_init(void)
{
	/* Configuracion del controlador de interrupciones. Estos registros están definidos en 44b.h */
	rI_ISPC    = 0x3ffffff;	// Borra INTPND escribiendo 1s en I_ISPC
	rEXTINTPND = 0xf;       // Borra EXTINTPND escribiendo 1s en el propio registro
	rINTMOD    = 0x0;		// Configura las linas como de tipo IRQ
	rINTCON    = 0x1;	    // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK    = ~(BIT_GLOBAL | BIT_EINT4567 | BIT_TIMER0 | BIT_TIMER2); // Enmascara todas las lineas excepto eint4567, el bit global y el timer0

	/* Establece la rutina de servicio para Eint4567 */
	pISR_EINT4567 = (int)Eint4567_ISR;

	/* Configuracion del puerto G */
	rPCONG  = 0xffff;        		// Establece la funcion de los pines (EINT0-7)
	rPUPG   = 0x0;                  // Habilita el "pull up" del puerto
	rEXTINT = rEXTINT | 0x22222222;   // Configura las lineas de int. como de flanco de bajada

	/* Por precaucion, se vuelven a borrar los bits de INTPND y EXTINTPND */
	rI_ISPC    |= (BIT_EINT4567);
	rEXTINTPND = 0xf;
}

void Eint4567_ISR(void)
{
	rINTMSK ^= BIT_EINT4567;
	push_debug(0x12,0x0);
	/* Identificar la interrupcion (hay dos pulsadores)*/
	int which_int = rEXTINTPND;
	switch (which_int)
	{
		case 4:
			if (state == 0) {
				state = 1;
				leds_off();
				int_count = 0xf;
			} else if (state == 1){
				fila++;
				if (fila > 10) {
					fila = 1;
				}
				int_count = fila;
			} else if (state == 2){
				columna++;
				if (columna > 9) {
					columna = 1;
				}
				int_count = columna;
			} else if (state == 3) {
				valor++;
				if (valor > 9) {
					valor = 0;
				}
				int_count = valor;
			}
			iniciarEspera(100000, 100000);
			break;
		case 8:
			if (state == 0)
			{
				leds_off();
				state = 1;
				int_count = 0xf;
			} else if (state == 1) {
				if (fila == 10) {
					state = 5;
					int_count = 0x10;
				}
				else if(fila != 0) {
					state = 2;
					int_count = 0xc;
				}
			} else if (state == 2) {
				if(columna != 0) {
					state = 3;
					int_count = 0x1;
				}
			} else if (state == 3 || state == 4) {
				state = 4;
			}
			else {
				state = 5;
			}



			iniciarEspera(100000, 100000);
			break;
		default:
			rINTMSK ^= BIT_EINT4567;
			break;
	}


	D8Led_symbol(int_count & 0x000f); // sacamos el valor por pantalla (módulo 16)
	/* Finalizar ISR */
	rEXTINTPND = 0xf;				// borra los bits en EXTINTPND
	rI_ISPC   |= BIT_EINT4567;		// borra el bit pendiente en INTPND

}

int esperaFinalizada(void)
{
	int tiempo_pasado = 0;
	tiempo_pasado = (timer2_leer() - tiempo_inicio);
	if(retardo <= tiempo_pasado) {
		if(soltado) {
			retardo = 0;

			rEXTINTPND = 0xf;				// borra los bits en EXTINTPND
			rI_ISPC |= BIT_EINT4567;		// borra el bit pendiente en INTPND

			rINTMSK ^= BIT_EINT4567;
			return 1;
		}

		else {
			// Si hemos soltado el boton
			if((rPDATG & 0xC0) == 0xC0) {
				retardo = retardo_trd;
				tiempo_inicio = timer2_leer();
				waitState = 0;
				soltado = 1;
			} else {
				if (waitState == 1)
				{
					waitState = 2;
					retardo = 50000;	// Add medio segundo de espera
					tiempo_inicio = timer2_leer();
				}
				else if (waitState == 2)
				{
					waitState = 3;
					retardo = 30000;
					tiempo_inicio = timer2_leer();
				}
				else if (waitState == 3)
				{
					// sumar 1 (mantenido)
					if (state == 1)
					{
						fila++;
						if (fila > 10)
						{
							fila = 1;
						}
						int_count = fila;
					}
					else if (state == 2){
						columna++;
						if (columna > 9) {
							columna = 1;
						}
						int_count = columna;
					} else if (state == 3) {
						valor++;
						if (valor > 9) {
							valor = 0;
						}
						int_count = valor;
					}
					retardo = 30000;
					tiempo_inicio = timer2_leer();
					D8Led_symbol(int_count & 0x000f); // sacamos el valor por pantalla (módulo 16)
				}

				retardo = retardo_trp;
				tiempo_inicio = timer2_leer();
			}
		}
	}

	return 0;
}

void iniciarEspera(unsigned int trp, unsigned int trd)
{
	tiempo_inicio = timer2_leer();
	retardo = trp;
	soltado = 0;
	waitState = 1;
	retardo_trp = trp;
	retardo_trd = trd;
	boton_pulsado();
}

