#######################################################################################################
# candidatos_arm:
# Almacena en una máscara todos los valores presentes en la fila, columna o región de la celda dada y
# almacena esta máscara en los 9 bits menos significativos de la celda.
# Parametros:
# 	r0: dirección en memoria del sudoku
#	r1: fila de la celda
#	r2: columna de la celda
# Devuelve:
#	r0: 1 si la celda esta llena y 0 en caso contrario
#######################################################################################################
.global    sudoku_candidatos_arm		// Definimos la etiqueta como global para hacerla accesible
sudoku_candidatos_arm:

		STMFD   sp!, {r4-r6,lr}

		add r3, r0, r1, lsl #5  // guardamos la dirección del primer elemento de la fila de la celda [r3 = @sudoku + 32 * fila]
		add r4, r0, r2, lsl #1  // guardamos la dirección del primer elemento de la columna de la celda [r4 = @sudoku + 2 * columna]
		add r5, r3, r2, lsl #1  // guardamos la dirección de la celda [r5 = r3 + 2 * columna]

//		ldrh r6, [r5]			// ver si no es pista
//		ands r6, r6, #0x0800
//		movne r0, #1			// si es pista, pongo r0 a 1, que será la salida, indicando que está ocupada
//		bne fin					// si es pista, salto al final

//calculamos dirección del primer elemento de la región
		//calculamos la fila del elemento (0,3 o 6) y almacenamos en r1
		cmp r1, #2
		movle r1,#0
		ble segcomp
		cmp r1, #5
		movle r1,#3
		ble segcomp
		mov r1, #6

		//calculamos la columna del elemento (0,3 o 6) y almacenamos en r2
segcomp:
		cmp r2, #2
		movle r2,#0
		ble fincomp
		cmp r2, #5
		movle r2,#3
		ble fincomp
		mov r2, #6

fincomp:
		//calculamos la dirección del elemento [r0 = sudoku + (32 * r1) + (2 * r2)]
		add r1, r2, r1, lsl #4 //r1 = r2 + r0 * 16
		add r0, r0, r1, lsl #1 //r0 = r0 + r1 * 2

		mov r1, #0 	//init mascara
		mov r12, #1  //1 auxiliar
		mov r6, #0	//init contador

//Recorremos la fila activando los bits necesarios en la mascara
forfila:
		cmp r3, r5
		beq minefila

		ldrh r2, [r3] 		//cargamos la celda **
		mov r2, r2, lsr #12		//guardamos solo el valor de la celda
		sub r2, r2, #1			//restamos uno para el desplazamiento
		mov r2, r12, lsl r2      //preparamos el 1 en el lugar que corresponde para la mascara
		orr r1, r1, r2			//actualizamos máscara

minefila:
		add r3, r3, #2
		cmp r6, #8
		beq initcolumna
		add r6, r6, #1
		b forfila

//Recorremos la columna activando los bits necesarios en la mascara
initcolumna:
		mov r6, #0	//init contador bucle

forcolumna:
		cmp r4, r5
		beq minecolumna

		ldrh r2, [r4] 		//cargamos la celda **

		mov r2, r2, lsr #12		//guardamos solo el valor de la celda
		sub r2, r2, #1			//restamos uno para el desplazamiento
		mov r2, r12, lsl r2      //preparamos el 1 en el lugar que corresponde para la mascara
		orr r1, r1, r2			//actualizamos máscara

minecolumna:
		add r4, r4, #32
		cmp r6, #8
		beq initregion
		add r6, r6, #1
		b forcolumna

//Recorremos la región activando los bits necesarios en la mascara
initregion:
		mov r6, #3				//contador de filas
		mov r3, #3				//contador de columnas

forregion:
		cmp r0, r5
		beq mineregion

		ldrh r2, [r0]		//cargamos la celda **

		mov r2, r2, lsr #12		//guardamos solo el valor de la celda
		sub r2, r2, #1			//restamos uno para el desplazamiento
		mov r2, r12, lsl r2     //preparamos el 1 en el lugar que corresponde para la mascara
		orr r1, r1, r2			//actualizamos máscara

mineregion:
		add r0, r0, #2
		subs r6, r6, #1
		bne forregion
		mov r6, #3				//si hemos recorrido la fila, reiniciamos contador y saltamos


		subs r3, r3, #1
		addne r0, r0, #26		//si hemos acabado iteracion, pero no todas, pasamos a la sigte linea y repetimos bucle de columnas
		bne forregion


//Check if error
		ldrh r2, [r5]			//Cargamos el valor de la celda
		mov r2, r2, lsr #12		//guardamos solo el valor de la celda
		sub r2, r2, #1			//restamos uno para el desplazamiento
		mov r2, r12, lsl r2
		ands r2, r2, r1
		moveq r0, #1
		movne r0, #-1

//añadimos la máscara a la celda (manteniendo el resto de bits)
		ldrh r2, [r5]			//Cargamos el valor de la celda
		mov r4, #0xFE00			//Ponemos la mascara a 0
		mvn r1, r1				// Invertimos la mascara y la añadimos a la celda
		and r2, r2, r4
		mvn r4, r4
		and r1, r1, r4
		orr r2, r2, r1
		strh r2, [r5]			// Almacenamos la celda en su posición

		ands r3, r2, #0xF000	// Ponemos a 0 la salida si la celda esta vacia
		moveq r0, #0

		//Comprobamos si está en la máscara

fin:

		LDMFD   sp!, {r4-r6,lr}
        # return to the instruccion that called the rutine and to arm mode
        BX      lr
