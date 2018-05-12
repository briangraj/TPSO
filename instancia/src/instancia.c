/*
 * instancia.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "instancia.h"

int main(int argc, char **argv){
	inicializar();

	conectar_con_coordinador();

	//configuracion_entradas();

	escuchar_coordinador();

	return 0;
}

void inicializar(){
	log = log_create("DataNode.log", "DataNode", 1, LOG_LEVEL_TRACE);

	leer_config();
}

void leer_config(){
	t_config* archivo_config = config_create(PATH_CONFIG);

	IP_COORDINADOR = leer_string(archivo_config, "IP_COORDINADOR");

	PUERTO_COORDINADOR = config_get_int_value(archivo_config, "PUERTO_COORDINADOR");

	MI_ID = config_get_int_value(archivo_config, "ID");

	PUNTO_MONTAGE = leer_string(archivo_config, "PUNTO_MONTAGE");

	//TODO esto hay que extraerlo
	almacenar_clave = &circular;
	entrada_a_reemplazar = list_get(tabla_de_entradas, 0);//TODO hay que ver donde va

	config_destroy(archivo_config);
}

void conectar_con_coordinador(){
	socket_coordinador = conectarse_a_server("instancia", INSTANCIA, "coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, log);
	if(socket_coordinador < 0){
		//rutina_final();
		exit(1);
	}

	enviar_paquete(MI_ID, socket_coordinador, 0, NULL);
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
		break;
	case OPERACION_SET:
		recibir_set();
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

//	for(nro_entrada = 0; nro_entrada < CANTIDAD_ENTRADAS; nro_entrada++){
//		entrada = malloc(sizeof(t_entrada));
//		entrada->nro_entrada = nro_entrada;
//		list_add(tabla_de_entradas, entrada);
//	}

	DIR* d = opendir(PUNTO_MONTAGE);
	struct dirent* dir;

	while((dir = readdir(d)) != NULL){
		if(strcmp(dir->d_name, ".")!=0 && strcmp(dir->d_name, "..")!=0 ){
			entrada = levantar_entrada(dir->d_name);
			list_add(tabla_de_entradas, entrada);
		}
	}

	closedir(d);

	log_trace(log, "cree tabla de entradas");
}

t_entrada* levantar_entrada(char* nombre){
	//TODO abrir archivo para leer valor
	t_entrada* entrada_nueva = malloc(sizeof(t_entrada));
	entrada_nueva->clave = string_new();
	string_append(&entrada_nueva->clave, nombre);

	//TODO terminar de setear valores y actualizar bitmap, segun tamanño de entradas (que hay que agregarlo)

	return entrada_nueva;
}

void recibir_set(){
	char* clave = recibir_string(socket_coordinador);
	char* valor = recibir_string(socket_coordinador);
	// TODO decidir como guardar la clave
	almacenar_clave(clave, valor);
}

void circular(char* clave, char* valor){
	//TODO faltan verificaciones: tamaño clave, espacio suficiente para almacenar valor
	entrada_a_reemplazar->clave;
	//TODO ver si el valor quepa en una entrada, sino hay que dividirlo
	int tamanio_valor = string_length(valor);
	entrada_a_reemplazar->tamanio_clave = tamanio_valor;
	memcpy(entrada_a_reemplazar, valor, tamanio_valor);

	int entradas_ocupadas = tamanio_valor / TAMANIO_ENTRADA;
	//TODO falta redondear

	entrada_a_reemplazar = list_get(tabla_de_entradas, entrada_a_reemplazar->nro_entrada + entradas_ocupadas);
}
