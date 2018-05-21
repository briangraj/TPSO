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

	nro_entrada = 0;

	leer_config();
}

void leer_config(){
	t_config* archivo_config = config_create(PATH_CONFIG);

	IP_COORDINADOR = leer_string(archivo_config, "IP_COORDINADOR");

	PUERTO_COORDINADOR = config_get_int_value(archivo_config, "PUERTO_COORDINADOR");

	MI_ID = config_get_int_value(archivo_config, "ID");

	PUNTO_MONTAGE = leer_string(archivo_config, "PUNTO_MONTAGE");

	//TODO esto hay que extraerlo
	algoritmo_reemplazo = &reemplazo_circular;
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
			entrada = levantar_entrada(dir->d_name);//FIXME decidir como levantar una entrada (si usa almacenar_entrada)
			list_add(tabla_de_entradas, entrada);
		}
	}

	closedir(d);

	log_trace(log, "cree tabla de entradas");
}

t_entrada* levantar_entrada(char* nombre){
	int file_desc = abrir_entrada(nombre);
	struct stat stat_entrada = crear_stat(file_desc);

	t_entrada* entrada_nueva = malloc(sizeof(t_entrada));
	entrada_nueva->clave = string_new();
	string_append(&entrada_nueva->clave, nombre);

	entrada_nueva->tamanio_bytes_clave = stat_entrada.st_size;

	entrada_nueva->valor = mi_mmap(file_desc, stat_entrada);

	entrada_nueva->tamanio_entradas_clave = entradas_ocupadas(entrada_nueva->tamanio_bytes_clave);

	//TODO delegarlo
	entrada_nueva->nro_entrada = nro_entrada;
	nro_entrada += entrada_nueva->tamanio_entradas_clave;

	int i;
	for(i = 0; i < entrada_nueva->tamanio_entradas_clave; i++)
		bitarray_set_bit(bitarray_entradas, entrada_nueva->nro_entrada + i);

	return entrada_nueva;
}

int abrir_entrada(char* nombre){
	char* aux = string_from_format("%s/%s", PUNTO_MONTAGE, nombre);

	int fd = open(aux, O_RDWR);
	if (fd == -1) {
		log_error(log, "ERROR: no se puedo abrir la entrada: %s", nombre);
		close(fd);
		//rutina_final();
		exit(1);
	}
	return fd;
}

struct stat crear_stat(int fd){
	struct stat stat;
	if(fstat(fd, &stat) < 0){
		log_error(log, "ERROR: no se pudo cargar los atributos de una entrada");
		close(fd);
		//rutina_final();
		exit(1);
	}
	return stat;
}

void* mi_mmap(int fd, struct stat stat){
	void* data = mmap(0, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(data == MAP_FAILED){
		log_error(log, "ERROR: no se pudo mapear uan entrada");
		//perror("mmap");
		close(fd);
		//rutina_final();
		exit(1);
	}

	return data;
}

int entradas_ocupadas(int tamanio){
	int entradas_ocupadas = tamanio / TAMANIO_ENTRADA;
	if(tamanio % TAMANIO_ENTRADA != 0)
		tamanio++;

	return entradas_ocupadas;
}

void recibir_set(){
	char* clave = recibir_string(socket_coordinador);
	char* valor = recibir_string(socket_coordinador);
	// TODO tiene que devolver si falla o no
	modificar_entrada(clave, valor);
}

void modificar_entrada(char* clave, char* valor){
	t_entrada* entrada = buscar_entrada(clave);
	if(string_length(valor) > entrada->tamanio_bytes_clave){
		//TODO ver si hay espacio
	}

	//FIXME esto seria el caso ideal: la nueva clave ocupa la misma cantidad de entradas que la anterior
	entrada->tamanio_bytes_clave = string_length(valor);

	//todo primero habria que borrar el valor anterior
	memcpy(entrada->valor, valor, string_length(valor));
}

void reemplazo_circular(char* clave, char* valor){

	//esto era para agregarlo a la tabla
//	if(!hay_espacio_para(entrada)){
//		//TODO habria que compactar y/o reemplazar
//		break;
//	}
//
//	entrada->nro_entrada = nro_entrada;
//	nro_entrada = entrada->tamanio_clave / TAMANIO_ENTRADA;
//	if(entrada->tamanio_clave % TAMANIO_ENTRADA != 0)
//		nro_entrada++;
//	list_add(tabla_de_entradas, entrada);

	//TODO faltan verificaciones: tamaño clave, espacio suficiente para almacenar valor
	entrada_a_reemplazar->clave;

	int tamanio_valor = string_length(valor);
	entrada_a_reemplazar->tamanio_bytes_clave = tamanio_valor;
	memcpy(entrada_a_reemplazar, valor, tamanio_valor);

	entrada_a_reemplazar->tamanio_entradas_clave = entradas_ocupadas(tamanio_valor);

	entrada_a_reemplazar = list_get(tabla_de_entradas, (entrada_a_reemplazar->nro_entrada) + entradas_ocupadas);
}

t_entrada* buscar_entrada(char* clave){
	//clave_a_encontrar = clave;
//TODO ver que esto funcione
	bool comparar_clave(void* void_clave){
		t_entrada* entrada_aux = (t_entrada*)void_clave;
		//char* clave_a_encontrar = clave;

		return string_equals_ignore_case(entrada_aux->clave, clave);
	}
	t_entrada* entrada = (t_entrada*)list_find(tabla_de_entradas, comparar_clave);

	return entrada;

}
