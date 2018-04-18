/*
 * funciones_esi.c
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */

#include <funciones_esi.h>

void iniciar_esi(){
	crear_log();

	leer_archivo_config();

	if(conectarse_a_server("ESI", "Coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, SOCKET_COORDINADOR, log_esi) == -1){
		log_destroy(log_esi);
		config_destroy(archivo_config);
		exit(1);
	}

	if(conectarse_a_server("ESI", "Planificador", IP_PLANIFICADOR, PUERTO_PLANIFICADOR, SOCKET_PLANIFICADOR, log_esi)  == -1){
		close(SOCKET_COORDINADOR);
		log_destroy(log_esi);
		config_destroy(archivo_config);
		exit(1);
	}


}

void crear_log(){
	log_esi = log_create("ESI.log", "Esi", 1, LOG_LEVEL_TRACE);

	if(log_esi == NULL){
		printf("No se pudo crear el archivo de log del esi. Se finaliza el mismo.");
		exit(1);
	}


	log_trace(log_esi, "El archivo de log se creo correctamente.");

}

void leer_archivo_config(){

	archivo_config = config_create("../../configs/esi.cfg"); // Ante problema con eclipse/consola, cambiarla por ruta absoluta

	IP_COORDINADOR = config_get_string_value(archivo_config, "IP_COORDINADOR");
	IP_PLANIFICADOR = config_get_string_value(archivo_config, "IP_PLANIFICADOR");
	PUERTO_PLANIFICADOR = config_get_int_value(archivo_config, "PUERTO_PLANIFICADOR");
	PUERTO_COORDINADOR = config_get_int_value(archivo_config, "PUERTO_COORDINADOR");
}













