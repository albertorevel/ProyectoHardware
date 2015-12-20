.global    sudoku_candidatos_thumb		// Definimos la etiqueta como global para hacerla accesible
sudoku_candidatos_thumb:
		push {lr}
		// Preparamos el cambio de ARM a THUMB
		ADR		r3, candidatos_thumb_interno+1 /* the last address bit is not really used to specify the address but to select between ARM and Thumb code */
		adr		r14,return		/* we store the return address in r14*/
		BX		r3				/* FUNCTION CALL, we jump to th_mul. +1 indicates that we want to switch to thumb */
return:
		pop {lr}
		bx lr


#######################################################################################################
# candidatos_thumb_interno:
# Almacena en una máscara todos los valores presentes en la fila, columna o región de la celda dada y
# almacena esta máscara en los 9 bits menos significativos de la celda.
# Parametros:
# 	r0: dirección en memoria del sudoku
#	r1: fila de la celda
#	r2: columna de la celda
# Devuelve:
#	r0: 1 si la celda esta llena y 0 en caso contrario
#######################################################################################################
.thumb
.global candidatos_thumb_interno
candidatos_thumb_interno:
		push	{r4-r7}			//preservamos registros almacenándolos en la pila

		lsl r3, r1, #5			//actualizamos el puntero de la fila -> fila = fila * 32
		add r3, r3, r0			//guardamos la dirección del primer elemento de la fila de la celda

		lsl r4, r2, #1			//actualizamos el puntero de la columna -> columna = columna * 2
		add r4, r4, r0			//guardamos la dirección del primer elemento de la columna de la celda

		lsl r5, r2, #1
		add r5, r3, r5  		//guardamos la dirección de la celda [r5 = r3 + 2 * columna]

		ldrb r6, [r5,#1]		//vemos si no es pista
		mov r7, #0x08
		and r6, r7
		bne pista				//vamos al final del método si es pista

//calculamos la dirección del primer elemento de la región
		//calculamos la fila del elemento (0,3 o 6) y almacenamos en r1
		mov r6, #6
		cmp r1, r6
		bge segcomp
		mov r6, #3
		cmp r1, r6
		bge segcomp
		mov r6, #0

		//calculamos la columna del elemento (0,3 o 6) y almacenamos en r2
segcomp:
		mov r1, r6		 // guardamos fila calculada

		mov r6, #6
		cmp r2, r6
		bge fincomp
		mov r6, #3
		cmp r2, r6
		bge fincomp
		mov r6, #0

fincomp:
		mov r2, r6		// guardamos la columna calculada

		//calculamos la dirección del elemento [r0 = sudoku + (32 * r1) + (2 * r2)]
		lsl r2, r2, #1
		lsl r1, r1, #5
		add r0, r1
		add r0, r2

		mov r1, #0			//inicializamos la mascara
		mov r7, #1			//1 auxiliar
		mov r6, #9 			//init contador

//Recorremos la fila activando los bits necesarios en la mascara
forfila:
		ldrh r2, [r3]
		add r3, #2
		lsr r2, r2, #12
		sub r2, r2, #1
		lsl r7, r7, r2
		orr r1, r7
		mov r7, #1

		sub r6, #1
		bne forfila


		mov r6, #9			//reiniciamos contador para la columna

//Recorremos la columna activando los bits necesarios en la mascara
forcolumna:
		ldrh r2, [r4]
		add r4, #32
		lsr r2, r2, #12
		sub r2, r2, #1
		lsl r7, r7, r2
		orr r1, r7
		mov r7, #1

		sub r6, #1
		bne forcolumna

//Recorremos la región activando los bits necesarios en la mascara
		mov r6, #3				//contador de filas
		mov r3, #3				//contador de columnas

forregion:
		ldrh r2, [r0]
		add r0, #2
		lsr r2, r2, #12
		sub r2, r2, #1
		lsl r7, r7, r2
		orr r1, r7
		mov r7, #1

		sub r6, #1
		bne forregion
		mov r6, #3				// Cambio de fila

		add r0, #26
		sub r3, #1				// Cambio de columna
		bne forregion



//añadimos la máscara a la celda (manteniendo el resto de bits)
		ldrb r2, [r5,#1]		// Cargamos el primer byte de la celda
		mov r4, #0XFE			// Borramos el ultimo bit del valor cargado (pertenece a la mascara)
		and r2, r4
		lsl r2, #8
		lsl r4, #1
		add r4, #3				// Invertimos la mascara y la añadimos a la celda
		eor r1, r4
		orr r2, r1

		strh r2, [r5]			// Almacenamos la celda en su posición


		mov r0, #0				// Indicamos celda vacia
final:
		pop	{r4-r7}

        BX      lr

pista:
		mov r0, #1				// Indicamos celda no vacia
		b final
