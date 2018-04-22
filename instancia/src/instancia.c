/*
 * instancia.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "instancia.h"

int main(int argc, char **argv){
	log_data_node = log_create("DataNode.log", "DataNode", 1, LOG_LEVEL_TRACE);

	leer_config();

	conectar_con_coordinador();

	crear_tabla_de_entradas();

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

void crear_tabla_de_entradas(){

}

void conectar_con_coordinador(){
	socket_coordinador = conectarse_a_server("instancia", INSTANCIA, "coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, log_data_node);
	if(socket_coordinador < 0){
		//rutina_final();
		exit(1);
	}
}

void escuchar_coordinador(){
	int protocolo;

	while(1) {
		protocolo = recibir_protocolo(socket_coordinador);
		printf("leer_protocolo %d\n", protocolo);
		if (protocolo <= 0) {
			log_error(log_data_node, "se desconecto el coordinador");
			//rutina_final();
			break;//exit(1);
		}
		//leer_protocolo_dn(protocolo);
	}

	close(socket_coordinador);

}
