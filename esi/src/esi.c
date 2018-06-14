/*
 ============================================================================
 Name        : esi.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "funciones_esi.h"

int main(int argc, char** argv) {
	//TODO: falta definir como y en que momento mandamos el ID del esi con los chicos (inmediatamente despues del handshake)

	if(argc != 2){
		fflush(stdout);
		printf("Cantidad incorrecta de parametros recibida");
		exit(1);
	}

	signal(SIGINT, signal_handler);

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

				log_debug(log_esi, "El planificador decidió abortar la ejecucion de este proceso");

				fclose(script);
				finalizar();

				break;

			case EJECUTAR_SENTENCIA:{
				log_debug(log_esi, "Se recibio la orden de ejecucion de una instruccion.");

				t_resultado_ejecucion informe_ejecucion = ejecutar_proxima_sentencia(script);

				int resultado = informar_resultado_al_usuario(informe_ejecucion, script);

				quedan_sentencias_por_ejecutar = verificar_sentencias_restantes(script);

				if(!quedan_sentencias_por_ejecutar && resultado != FALLO_EN_EJECUCION){
					log_trace(log_esi, "Se ejecutaron todas las sentencias correctamente. Congratulaciones!");

					enviar_paquete(FIN_DEL_SCRIPT, SOCKET_PLANIFICADOR, 0, NULL);

					break;
				}

				enviar_paquete(resultado, SOCKET_PLANIFICADOR, 0, NULL);

				if(resultado == FALLO_EN_EJECUCION){
					fclose(script);
					finalizar();
				}

				break;
			}

			default:
				log_error(log_esi, "Se perdio la conexion con el planificador, el proceso ESI va a finalizar");

				fclose(script);

				finalizar();
		}


	}

	fclose(script);

	finalizar();
}











