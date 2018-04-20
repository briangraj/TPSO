/*
 ============================================================================
 Name        : esi.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "funciones_esi.h"

int main(int argc, char** argv) { // falta que el esi reciba la ruta del archivo del script como parametro

	if(argc != 2){
		fflush(stdout);
		printf("Cantidad incorrecta de parametros recibida");
		exit(1);
	}

	iniciar_esi();

	FILE* script = fopen(argv[1], "r");

	if(script == NULL){
		log_error(log_esi, "No se pudo abrir el archivo de la ruta %s", argv[1]);
		finalizar();
	}

	bool quedan_sentencias_por_ejecutar = true;

	while(quedan_sentencias_por_ejecutar){

		int orden = recibir_protocolo(SOCKET_PLANIFICADOR);

		switch(orden){
			case FINALIZAR_PROCESO:

				fclose(script);
				finalizar();

				break;

			case EJECUTAR_SENTENCIA:{
				resultado_ejecucion informe_ejecucion = ejecutar_proxima_sentencia(script);

				int resultado = informar_resultado_al_usuario(informe_ejecucion);

				enviar_paquete(resultado, SOCKET_PLANIFICADOR, 0, NULL);

				if(resultado == FALLO_EN_EJECUCION){
					fclose(script);
					finalizar();
				}

				quedan_sentencias_por_ejecutar = verificar_sentencias_restantes(script);

				break;
			}

		}


	}


}











