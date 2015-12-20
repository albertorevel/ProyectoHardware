#include<inttypes.h>
#include <stdio.h>

// Tamaños de la cuadricula
// Se utilizan 16 columnas para facilitar la visualización
enum {NUM_FILAS = 9, NUM_COLUMNAS = 16};

enum {FALSE = 0, TRUE = 1};

extern int state;
extern int fila;
extern int columna;
extern int valor;

unsigned int tiempoCalculo = 0;
unsigned int tiempoTotal = 0;
//static const char *num_ascii[] = {"0","1","2","3","4","5","6","7","8","9"};


typedef uint16_t CELDA;
// La información de cada celda está agrupada en 16 bits con el siguiente formato (empezando en el bit más significativo):
// 4 MSB VALOR
// 1 bit PISTA
// 1 bit ERROR
// 1 bit no usado
// 9 LSB CANDIDATOS


inline void celda_poner_valor(CELDA *celdaptr, uint8_t val){
	*celdaptr = (*celdaptr & 0x0FFF) | ((val & 0x000F) << 12);
}
inline uint8_t celda_leer_valor(CELDA celda){
	return celda >> 12;
}
inline uint16_t celda_leer_candidatos(CELDA celda){
	return celda & 0x01FF;
}
inline int es_pista(CELDA celda){
	int res = celda & 0x800;
	return res;
}

extern void D8Led_symbol();
//extern void led1_on();
//extern void leds_off();
extern void putData();
extern void putError();
extern void putCandidates();
extern void putPista();
//extern void putFinal();
extern void tablero();
extern int timer2_leer();
extern void Lcd_Dma_Trans();
extern void mostrarTiempo();
extern char* itoa();
extern int sudoku_candidatos_arm();
extern Lcd_Clr();
extern Lcd_Active_Clr();


////////////////////////////////////////////////////////////////////////////////
// Este metodo recorre el sudoku celda por celda, almacenando en una variable las celdas vacias
// dependiendo del valor que devuelve la ejecucion del metodo candidatos
// Parametros:
// 	cuadricula: dirección en memoria del sudoku
// Devuelve:
//	numero de celdas vacias, en negativo si hay error
int sudoku_recalcular_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]) {

	//Recorre todas las celdas almacenando las celdas vacias
	int vacias = 0,x,y;
	int resultado = 0;
	int candidatos = 0;
	unsigned int tiempoTemp = timer2_leer();
	int error = 1;
//	char *valor;
	error = 1;
	for (x = 0; x < NUM_FILAS; x++) {
		for (y = 0; y < NUM_FILAS; y++) {

			// Lanzamos el metodo candidatos escrito en ARM
			resultado = sudoku_candidatos_arm(cuadricula,x,y);
			char buffer [1];
			itoa(celda_leer_valor(cuadricula[x][y]),buffer,10);
			int pista = 0;
			if(es_pista(cuadricula[x][y])) {
							putPista(x,y);
							pista=1;
			}

			switch(resultado)
			{
				case -1 :
					error = -1;
					//TODO error en celda?
					// Pintamos el valor indicando que hay error
					putError(buffer,x,y,pista);
					break;
				case FALSE :
					vacias++;
					candidatos = celda_leer_candidatos(cuadricula[x][y]);
					putCandidates(candidatos,x,y);
					break;
				case TRUE :
					// Pintamos el valor
					putData(buffer,x,y);
					break;
			}



		}
	}
	tiempoCalculo += (timer2_leer() - tiempoTemp);
	if (vacias != 0)
	{
		Lcd_Dma_Trans();
	} else {
		Lcd_Clr();
		Lcd_Active_Clr();
	}
	return error * vacias;


}

////////////////////////////////////////////////////////////////////////////////
// Proceso principal del juego que recibe el tablero,
// y la señal de ready que indica que se han actualizado fila y columna
int sudoku9x9() {

	int celdas_vacias = 0;
	unsigned int tiempoInit = 0;
	CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS] __attribute__((aligned(8)))=
	{
			0x9800,0x6800,0x0000,0x0000,0x0000,0x0000,0x7800,0x0000,0x8800,0,0,0,0,0,0,0,
			0x8800,0x0000,0x0000,0x0000,0x0000,0x4800,0x3800,0x0000,0x0000,0,0,0,0,0,0,0,
			0x1800,0x0000,0x0000,0x5800,0x0000,0x0000,0x0000,0x0000,0x0000,0,0,0,0,0,0,0,
			0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1800,0x7800,0x6800,0,0,0,0,0,0,0,
			0x2800,0x0000,0x0000,0x0000,0x9800,0x3800,0x0000,0x0000,0x5800,0,0,0,0,0,0,0,
			0x7800,0x0000,0x8800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0,0,0,0,0,0,0,
			0x0000,0x0000,0x7800,0x0000,0x3800,0x2800,0x0000,0x4800,0x0000,0,0,0,0,0,0,0,
			0x3800,0x8800,0x2800,0x1800,0x0000,0x5800,0x6800,0x0000,0x0000,0,0,0,0,0,0,0,
			0x0000,0x4800,0x1800,0x0000,0x0000,0x9800,0x5800,0x2800,0x0000,0,0,0,0,0,0,0
	};

//	CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS] __attribute__((aligned(8)))=
//	{
//			0x9800,0x6800,0x4800,0x3800,0x2800,0x1800,0x7800,0x5800,0x8800,0,0,0,0,0,0,0,
//			0x8800,0x7800,0x5800,0x9800,0x6800,0x4800,0x3800,0x1800,0x2800,0,0,0,0,0,0,0,
//			0x1800,0x2800,0x0000,0x5800,0x8800,0x7800,0x9800,0x6800,0x4800,0,0,0,0,0,0,0,
//			0x4800,0x3800,0x9800,0x2800,0x5800,0x8800,0x1800,0x7800,0x6800,0,0,0,0,0,0,0,
//			0x2800,0x1800,0x6800,0x7800,0x9800,0x3800,0x4800,0x8800,0x5800,0,0,0,0,0,0,0,
//			0x7800,0x5800,0x8800,0x4800,0x1800,0x6800,0x2800,0x3800,0x9800,0,0,0,0,0,0,0,
//			0x5800,0x9800,0x7800,0x6800,0x3800,0x2800,0x8800,0x4800,0x1800,0,0,0,0,0,0,0,
//			0x3800,0x8800,0x2800,0x1800,0x4800,0x5800,0x6800,0x9800,0x7800,0,0,0,0,0,0,0,
//			0x6800,0x4800,0x1800,0x8800,0x7800,0x9800,0x5800,0x2800,0x3800,0,0,0,0,0,0,0
//	};

	// Iniciamos contadores
	tiempoInit = timer2_leer();
	tiempoCalculo = 0;
	tiempoTotal = 0;
	// Recalculamos el tablero por primera vez
	celdas_vacias = sudoku_recalcular_c(cuadricula);
	tiempoTotal = timer2_leer() - tiempoInit;
	mostrarTiempo();

	// Mientras no haya acabado el juego.
	while (state < 5)
	{
		// Entramos si se ha introducido un cambio
		if (state == 4)
		{
			if(es_pista(cuadricula[fila - 1][columna - 1]) == 0) {
				celda_poner_valor(&cuadricula[fila - 1][columna - 1], valor);
				tablero();
				celdas_vacias = sudoku_recalcular_c(cuadricula);
			}

			fila = 0;
			columna = 0;
			valor = 1;


			//TODO mirar si cambiar
			tiempoTotal = timer2_leer() - tiempoInit;
			mostrarTiempo();
			state = 1;
			if(celdas_vacias == 0) {
				D8Led_symbol(0x000a);
				state = 5;
			} else {
				D8Led_symbol(0x000f);
				state = 1;

			}

		}
	}

	// Juego acabado.
	state = 0;
	return celdas_vacias;


}
