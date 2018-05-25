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
	log_instancia = log_create("DataNode.log", "DataNode", 1, LOG_LEVEL_TRACE);

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
	socket_coordinador = conectarse_a_server("instancia", INSTANCIA, "coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, log_instancia);
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
			log_error(log_instancia, "se desconecto el coordinador");
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
	recv(socket_coordinador, &CANTIDAD_ENTRADAS_TOTALES, sizeof(CANTIDAD_ENTRADAS_TOTALES), MSG_WAITALL);
	recv(socket_coordinador, &TAMANIO_ENTRADA, sizeof(TAMANIO_ENTRADA), MSG_WAITALL);

	crear_tabla_de_entradas();
}

void crear_tabla_de_entradas(){
	tabla_de_entradas = list_create();
	t_entrada* entrada;

	DIR* d = opendir(PUNTO_MONTAGE);
	struct dirent* dir;

	while((dir = readdir(d)) != NULL){
		if(strcmp(dir->d_name, ".")!=0 && strcmp(dir->d_name, "..")!=0 ){
			entrada = levantar_entrada(dir->d_name);
			list_add(tabla_de_entradas, entrada);
		}
	}

	closedir(d);

	log_trace(log_instancia, "cree tabla de entradas");
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

	//no lo chequeo por que se usa solo en la inicializacion (no va a pasar que tenga menos espacio que un estado anterior)
	entrada_nueva->nro_entrada = entrada_para(entrada_nueva->tamanio_entradas_clave);

	setear_bitarray(entrada_nueva);

	return entrada_nueva;
}

int abrir_entrada(char* nombre){
	char* aux = string_from_format("%s/%s", PUNTO_MONTAGE, nombre);

	int fd = open(aux, O_RDWR);
	if (fd == -1) {
		log_error(log_instancia, "ERROR: no se puedo abrir la entrada: %s", nombre);
		close(fd);
		//rutina_final();
		exit(1);
	}
	return fd;
}

struct stat crear_stat(int fd){
	struct stat stat;
	if(fstat(fd, &stat) < 0){
		log_error(log_instancia, "ERROR: no se pudo cargar los atributos de una entrada");
		close(fd);
		//rutina_final();
		exit(1);
	}
	return stat;
}

void* mi_mmap(int fd, struct stat stat){
	void* data = mmap(0, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(data == MAP_FAILED){
		log_error(log_instancia, "ERROR: no se pudo mapear uan entrada");
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

int entrada_para(int cant_entradas){
	int i = 0, entradas_libres;

	while(i < CANTIDAD_ENTRADAS_TOTALES){
		if(!bitarray_test_bit(bitarray_entradas, i)){
			entradas_libres = entradas_libres_desde(i, cant_entradas);
			if(entradas_libres == cant_entradas)
				return i;

			i += entradas_libres;
		}

		i++;
	}

	return -1;
}

int entradas_libres_desde(int nro_entrada, int entradas_necesarias){
	int i;
	for(i = 0; i < entradas_necesarias; i++){
		if(bitarray_test_bit(bitarray_entradas, nro_entrada + i)){
			break;
		}
	}

	return i;
}

void setear_bitarray(t_entrada* entrada){
	int i;
	for(i = entrada->nro_entrada; i < entrada->tamanio_entradas_clave; i++)
		bitarray_set_bit(bitarray_entradas, i);
}

void recibir_set(){
	char* clave = recibir_string(socket_coordinador);
	char* valor = recibir_string(socket_coordinador);

	int resultado = modificar_entrada(clave, valor);
	enviar_paquete(resultado, socket_coordinador, 0, NULL);
}

int modificar_entrada(char* clave, char* valor){
	t_entrada* entrada = buscar_entrada(clave);
	int entradas_nuevo_valor = entradas_ocupadas(string_length(valor));
	int entradas_faltantes = entradas_nuevo_valor - entrada->tamanio_entradas_clave;

	if(entradas_nuevo_valor == entrada->tamanio_entradas_clave){
		entrada->tamanio_bytes_clave = string_length(valor);
		//todo primero habria que borrar el valor anterior
		memcpy(entrada->valor, valor, string_length(valor));
	} else {
		if(entradas_nuevo_valor > entrada->tamanio_bytes_clave){
			int entradas_libres = entradas_libres_desde(entrada->nro_entrada + entrada->tamanio_entradas_clave, entradas_faltantes);
			if(entradas_libres != entradas_faltantes)
				return -1;
		}

		actualizar_tamanio_entrada(entrada, valor);
	}

	return SET_EXITOSO;
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

	int tamanio_valor = string_length(valor);
	entrada_a_reemplazar->tamanio_bytes_clave = tamanio_valor;
	memcpy(entrada_a_reemplazar, valor, tamanio_valor);

	entrada_a_reemplazar->tamanio_entradas_clave = entradas_ocupadas(tamanio_valor);

	//entrada_a_reemplazar = list_get(tabla_de_entradas, (entrada_a_reemplazar->nro_entrada) + entradas_ocupadas);
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
