/*
 * instancia.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "instancia.h"

int main(int argc, char **argv){
	log = log_create("DataNode.log", "DataNode", 1, LOG_LEVEL_TRACE);

	leer_config();

	conectar_con_coordinador();

	configuracion_entradas();

	escuchar_coordinador();

	return 0;
}

void leer_config(){
	t_config* archivo_config = config_create(PATH_CONFIG);
	IP_COORDINADOR= string_new();
	string_append(&IP_COORDINADOR, config_get_string_value(archivo_config, "IP_COORDINADOR"));
	PUERTO_COORDINADOR = config_get_int_value(archivo_config, "PUERTO_COORDINADOR");

	config_destroy(archivo_config);
}

void conectar_con_coordinador(){
	socket_coordinador = conectarse_a_server("instancia", INSTANCIA, "coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, log);
	if(socket_coordinador < 0){
		//rutina_final();
		exit(1);
	}
}

void configuracion_entradas(){
	recv(socket_coordinador, &CANTIDAD_ENTRADAS, sizeof(CANTIDAD_ENTRADAS), MSG_WAITALL);
	recv(socket_coordinador, &TAMANIO_ENTRADA, sizeof(TAMANIO_ENTRADA), MSG_WAITALL);

	crear_tabla_de_entradas();
}

void crear_tabla_de_entradas(){
	tabla_de_entradas = list_create();
	int nro_entrada;
	t_entrada* entrada;

	for(nro_entrada = 0; nro_entrada < CANTIDAD_ENTRADAS; nro_entrada++){
		entrada = malloc(sizeof(t_entrada));
		entrada->nro_entrada = nro_entrada;
		list_add(tabla_de_entradas, entrada);
	}

	log_trace(log, "cree tabla de entradas");
}

void escuchar_coordinador(){
	int protocolo;

	while(1) {
		protocolo = recibir_protocolo(socket_coordinador);
		printf("leer_protocolo %d\n", protocolo);
		if (protocolo <= 0) {
			log_error(log, "se desconecto el coordinador");
			//rutina_final();
			break;//exit(1);
		}
		leer_protocolo(protocolo);
	}

	close(socket_coordinador);

}

void leer_protocolo(int protocolo){
	switch(protocolo){
	case CONFIGURACION_ENTRADAS:
		configuracion_entradas();
	}
}

