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

	if((SOCKET_PLANIFICADOR = conectarse_a_server("ESI", ESI, "Planificador", IP_PLANIFICADOR, PUERTO_PLANIFICADOR, log_esi))  == -1){
		log_destroy(log_esi);
		config_destroy(archivo_config);
		exit(1);
	}

	if(recibir_protocolo(SOCKET_PLANIFICADOR)!= ENVIO_ID){
		log_error(log_esi, "No se recibio el ID del Planificador");
		close(SOCKET_PLANIFICADOR);
		log_destroy(log_esi);
		config_destroy(archivo_config);
		exit(1);
	}

	if(recv(SOCKET_PLANIFICADOR, &MI_ID, sizeof(int), MSG_WAITALL) < 0){
		log_error(log_esi, "No se recibio el ID del Planificador");
		close(SOCKET_PLANIFICADOR);
		log_destroy(log_esi);
		config_destroy(archivo_config);
		exit(1);
	}

	if((SOCKET_COORDINADOR = conectarse_a_server("ESI", ESI, "Coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, log_esi)) == -1){
		close(SOCKET_PLANIFICADOR);
		log_destroy(log_esi);
		config_destroy(archivo_config);
		exit(1);
	}

	if(enviar_paquete(ENVIO_ID, SOCKET_COORDINADOR, sizeof(int), &MI_ID) == -1) {
		log_error(log_esi, "Se perdio la conexion con el coordinador a la hora de enviarle el ID");
		close(SOCKET_PLANIFICADOR);
		finalizar();
	}

	log_info(log_esi, "ESI %d iniciado con exito", MI_ID);
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
	t_resultado_ejecucion resultado; // TODO = Asigna en null ??????

	char* linea = string_new();

	int tamanio_de_lo_leido = 1;

	getline(&linea, &tamanio_de_lo_leido, script);

	if(linea == NULL){
		resultado.informe_coordinador = ERROR_LECTURA_SCRIPT;
		return resultado;
	}

	resultado.sentencia_ejecutada = strdup(linea);

    t_esi_operacion operacion = parse(linea);

	if(operacion.valido){
		switch(operacion.keyword){
			case GET:
				resultado.informe_coordinador = operacion_get_al_coordinador(operacion.argumentos.GET.clave);

				break;
			case SET:
				resultado.informe_coordinador = operacion_set_al_coordinador(operacion.argumentos.SET.clave, operacion.argumentos.SET.valor);

				break;
			case STORE:
				resultado.informe_coordinador = operacion_store_al_coordinador(operacion.argumentos.STORE.clave);

				break;
			default:
				resultado.informe_coordinador = ERROR_INTERPRETACION_SENTENCIA;
		}
	}

	destruir_operacion(operacion);

	free(linea);

	return resultado;
}

int informar_resultado_al_usuario(t_resultado_ejecucion informe_ejecucion){

	if(informe_ejecucion.sentencia_ejecutada != NULL){
		log_debug(log_esi, "Se intento ejecutar la sentencia %s", informe_ejecucion.sentencia_ejecutada);
		free(informe_ejecucion.sentencia_ejecutada);
	}

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
			 log_error(log_esi, "El coordinador tuvo un problema de comunicacion con un proceso crucial");

			 return FALLO_EN_EJECUCION;

		case ERROR_CLAVE_INEXISTENTE:
			 log_error(log_esi, "Se intento acceder a una clave inexistente en el sistema");//FIXME: ver diferencia con CLAVE_NO_IDENTIFICADA

			 return FALLO_EN_EJECUCION;

		case ERROR_LECTURA_SCRIPT:
			log_error(log_esi, "No se pudo leer la ultima linea solicitada del script.");

			 return FALLO_EN_EJECUCION;

		case ERROR_INTERPRETACION_SENTENCIA:
			log_error(log_esi, "No se pudo interpretar la ultima linea solicitada del script, no corresponde a una operacion valida.");

			 return FALLO_EN_EJECUCION;

		default:
			 log_error(log_esi, "Se perdio la conexion con el coordinador");

			 return FALLO_EN_EJECUCION;

	}

}

int operacion_get_al_coordinador(char * clave){
	int tamanio_clave = strlen(clave) + 1;
	int tamanio_paquete = sizeof(int) + tamanio_clave;

	void* paquete = malloc(tamanio_paquete);

	memcpy(paquete, &tamanio_clave, sizeof(int));

	int offset = sizeof(int);

	memcpy(paquete + offset, clave, tamanio_clave);

	if(enviar_paquete(OPERACION_GET, SOCKET_COORDINADOR, tamanio_paquete, paquete) == -1){
		free(paquete);
		return -1;
	}

	free(paquete);

	return recibir_protocolo(SOCKET_COORDINADOR);
}

int operacion_set_al_coordinador(char * clave, char* valor){
	int tamanio_clave = strlen(clave) + 1;

	int tamanio_valor = strlen(valor) + 1;

	int tamanio_paquete = tamanio_clave + tamanio_valor + 2*sizeof(int);

	void* paquete = malloc(tamanio_paquete);

	memcpy(paquete, &tamanio_clave, sizeof(int));

	int offset = sizeof(int);

	memcpy(paquete + offset, clave, tamanio_clave);

	offset += tamanio_clave;

	memcpy(paquete + offset, &tamanio_valor, sizeof(int));

	offset += sizeof(int);

	memcpy(paquete + offset, valor, tamanio_valor);

	if(enviar_paquete(OPERACION_SET, SOCKET_COORDINADOR, tamanio_paquete, paquete) == -1){
		free(paquete);
		return -1;
	}

	free(paquete);

	return recibir_protocolo(SOCKET_COORDINADOR);
}


int operacion_store_al_coordinador(char * clave){
	int tamanio_clave = strlen(clave) + 1;
	int tamanio_paquete = sizeof(int) + tamanio_clave;

	void* paquete = malloc(tamanio_paquete);

	memcpy(paquete, &tamanio_clave, sizeof(int));

	int offset = sizeof(int);

	memcpy(paquete + offset, clave, tamanio_clave);

	if(enviar_paquete(OPERACION_STORE, SOCKET_COORDINADOR, tamanio_paquete, paquete) == -1){
		free(paquete);
		return -1;
	}

	free(paquete);

	return recibir_protocolo(SOCKET_COORDINADOR);
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






