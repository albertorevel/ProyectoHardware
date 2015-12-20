/*********************************************************************************************
* Fichero:		timer2.c
* Autor:		Alberto Revel Jarne, Rubén Quílez Serrano
* Descrip:		funciones de control del timer2 del s3c44b0x
* Version:		1.0
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"

/*--- variables globales ---*/
volatile int timer2_num_int = 0;
volatile static int espera_boton = 0;

/*--- declaracion de funciones ---*/
void timer2_ISR(void) __attribute__((interrupt("IRQ")));
void timer2_inicializar(void);
void timer2_empezar(void);
int timer2_leer(void);
void timer2_stop (void);
void boton_pulsado(void);

/*--- declaracion de funciones externas ---*/
extern int esperaFinalizada(void);


/*--- codigo de las funciones ---*/
void timer2_ISR(void)
{
	timer2_num_int++;
	if(espera_boton && esperaFinalizada()) {
		espera_boton = 0;
	}
	/* borrar bit en I_ISPC para desactivar la solicitud de interrupción*/
	rI_ISPC |= BIT_TIMER2; // BIT_TIMER2 está definido en 44b.h y pone un uno en el bit 11 que correponde al Timer2
}

void timer2_inicializar(void)
{
	/* Configuracion controlador de interrupciones */
    rINTMOD = 0x0; // Configura las linas como de tipo IRQ
    rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK = rINTMSK & (0xFFFFF7FF); // Emascara todas las lineas excepto Timer2 y el bit global (bits 26 y 11, BIT_GLOBAL y BIT_TIMER2 están definidos en 44b.h)
	/* Establece la rutina de servicio para TIMER2 */
	pISR_TIMER2 = (unsigned) timer2_ISR;

	/* Configura el Timer2 */
	rTCFG0 = rTCFG0 | 0x0000; // ajusta el preescalado
	rTCFG1 = 0x0; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCNTB2 = 65535;// valor inicial de cuenta (la cuenta es descendente)
	rTCMPB2 = 0;// valor de comparación
	/* establecer update=manual (bit 1) + inverter=on (¿? será inverter off un cero en el bit 2 pone el inverter en off)*/
	rTCON |= 0x2000;
	/* iniciar timer (bit 0) con auto-reload (bit 3)*/
	rTCON |= 0x8000; //auto-reload, inverter, manual update
}

/*Esta función reinicia la cuenta de tiempo (contador y variable) y comienza a medir*/
void timer2_empezar()
{
	timer2_num_int = 0;
	//rTCNTB2 = 65535; no hace falta (auto-reload)
	rTCON ^= 0x3000; //descativamos manual update y activamos start
	//rTCON = rTCON | 0x1000; //activamos start
}

/*Esta función lee la cuenta actual del temporizador y el número de interrupciones
  generadas y devuelve el tiempo transcurrido en microsegundos*/
int timer2_leer()
{
	unsigned int tiempo_actual,tiempo_total,parcial;
	// Vemos el tiempo transcurrido según los ciclos completos
	tiempo_actual= ((unsigned int)timer2_num_int * 65535) / 33;
	// Vemos el tiempo transcurrido desde la última interrupción,
	// lo sumamos al calculado anteriormente y devolvemos ese valor.
	parcial= (65535 - (unsigned int)rTCNTO2) / 33;
	tiempo_total= tiempo_actual + parcial;

	return tiempo_total;

}

void timer2_stop(void){
	rTCON &= 0xFFFEFFF;//ponemos el bit 12 a 0 y asi detenemos el timer
}

void boton_pulsado(void)
{
	espera_boton = 1;
}

