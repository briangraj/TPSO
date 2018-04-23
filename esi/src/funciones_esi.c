/*
 * funciones_esi.c
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */

#include "funciones_esi.h"

void iniciar_esi(){
	crear_log();

	leer_archivo_config();

	if((SOCKET_COORDINADOR = conectarse_a_server("ESI", ESI, "Coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, log_esi)) == -1){
		log_destroy(log_esi);
		config_destroy(archivo_config);
		exit(1);
	}

	if((SOCKET_PLANIFICADOR = conectarse_a_server("ESI", ESI, "Planificador", IP_PLANIFICADOR, PUERTO_PLANIFICADOR, log_esi))  == -1){
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

	//archivo_config = config_create("../../configs/esi.cfg"); // Ante problema con eclipse/consola, cambiarla por ruta absoluta

	archivo_config = config_create("/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/configs/esi.cfg");

	IP_COORDINADOR = config_get_string_value(archivo_config, "IP_COORDINADOR");
	IP_PLANIFICADOR = config_get_string_value(archivo_config, "IP_PLANIFICADOR");
	PUERTO_PLANIFICADOR = config_get_int_value(archivo_config, "PUERTO_PLANIFICADOR");
	PUERTO_COORDINADOR = config_get_int_value(archivo_config, "PUERTO_COORDINADOR");
}

t_resultado_ejecucion ejecutar_proxima_sentencia(FILE* script){
	t_resultado_ejecucion resultado = {.sentencia_ejecutada = string_new()};


	return resultado;
}

int informar_resultado_al_usuario(t_resultado_ejecucion informe_ejecucion){

	log_debug(log_esi, "Se intento ejecutar la sentencia %s", informe_ejecucion.sentencia_ejecutada);

	free(informe_ejecucion.sentencia_ejecutada);

	switch (informe_ejecucion.informe_coordinador){

		case EJECUCION_EXITOSA:
			 log_trace(log_esi, "Se ejecuto correctamente la sentencia");

			 return EJECUCION_EXITOSA;

		case ERROR_TAMANIO_CLAVE:
			 log_error(log_esi, "Se excedio el tamanio maximo de 40 caracteres para la clave");

			 return FALLO_EN_EJECUCION;

		case ERROR_CLAVE_NO_IDENTIFICADA:
			 log_error(log_esi, "Se intento acceder a una clave no identificada en el sistema");//FIXME: ver diferencia con CLAVE_INEXISTENTE

			 return FALLO_EN_EJECUCION;

		case ERROR_DE_COMUNICACION: //Este case contempla los casos de desconexion entre coordinador-planificador y coordinador-instancia
			 log_error(log_esi, "El coordinador tuvo un problema de comunicacion con otro proceso crucial");

			 return FALLO_EN_EJECUCION;

		case ERROR_CLAVE_INEXISTENTE:
			 log_error(log_esi, "Se intento acceder a una clave inexistente en el sistema");//FIXME: ver diferencia con CLAVE_NO_IDENTIFICADA

			 return FALLO_EN_EJECUCION;

		default:
			 log_error(log_esi, "Se perdio la conexion con el coordinador");

			 return FALLO_EN_EJECUCION;

	}

}

bool verificar_sentencias_restantes(FILE* script){

	if(getc(script) == EOF)
		return false;

	fseek(script, -1, SEEK_CUR);

	return true;
}

/*
 * SET clave valor
 * SET clave valor
 *
 * */

void finalizar(){
	close(SOCKET_COORDINADOR);
	close(SOCKET_PLANIFICADOR);
	log_destroy(log_esi);
	config_destroy(archivo_config);

	exit(1);
}






