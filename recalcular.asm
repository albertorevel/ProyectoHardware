#################################################################################################################
# sudoku_recalcular_arm:
# Este metodo recorre el sudoku celda por celda, almacenando en una variable las celdas vacias
# dependiendo del valor que devuelve la ejecucion del metodo candidatos
# Parametros:
# 	r0: dirección en memoria del sudoku
#	r1: metodo candidatos a ejecutar (0 para C, 1 para ARM y 2 para THUMB)
# Devuelve:
#	r0: numero de celdas vacias
#######################################################################################################
.global sudoku_recalcular_arm
sudoku_recalcular_arm:

		STMFD   sp!, {r4-r8,lr}

		mov r4, #0 //vacias = 0
		mov r5, r0	// r5 = sudoku
		mov r8, r1
		mov r6, #0  // r6 = 0 (fila)
for1:
		cmp r6, #9
		beq sal
		mov r7, #0  // r7 = 0 (columna)
for2:
		cmp r7, #9  //r1 = columna
		addeq r6, r6, #1  //fila = fila + 1
		beq for1
		mov r0, r5		//parametro1
		mov r1, r6		//parametro2
		mov r2, r7		//parametro3
		cmp r8, #0
		//bleq sudoku_candidatos_c	//Metodo candidatos escrito en C
		cmp r8, #1
		bleq sudoku_candidatos_arm  //Metodo candidatos escrito en ARM
		cmp r8, #2
		//bleq sudoku_candidatos_thumb  //Metodo candidatos escrito en THUMB
		cmp r0, #0   //comp si vacia
		addeq r4, r4, #1
		add r7, r7, #1  //columna = columna + 1
		b for2
sal:
		mov r0, r4

		LDMFD   sp!, {r4-r8,lr}
        # return to the instruccion that called the rutine and to arm mode
        BX      lr
