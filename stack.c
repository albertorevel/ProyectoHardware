#include "44b.h"
#include "44blib.h"

extern int timer2_leer();

#define DS_SIZE 0x100					// Definimos tamano de la pila
#define DS_END 0xc7ff600
#define DS_START (DS_END + DS_SIZE)		// Definimos limite de la pila

volatile static unsigned int * dstackPt;

/*--- declaracion de funciones ---*/
void init_stack(void);
void push_debug(int ID_evento, int auxData);


void init_stack(void)
{
	dstackPt = (unsigned int *) DS_START;
	int * ipt;
	// Ponemos toda la pila a 0
	for ( ipt = dstackPt - 1; ipt > (unsigned int *) DS_END; ipt-- )
	{
		(ipt[0]) = 0;
	}

}

void push_debug(int ID_evento, int auxData)
{
	dstackPt -= 4;
	if (dstackPt < (unsigned int *) DS_END)
	{
		dstackPt = (unsigned int *) DS_START;
		dstackPt -= 4;
	}
	(dstackPt[0]) = ID_evento;
	(dstackPt[1]) = timer2_leer();
	(dstackPt[2]) = auxData;

}
