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

	//enviar_entradas_al_coordinador();

	crear_hilo(hilo_dump, NULL);

	escuchar_coordinador();

	return 0;
}

void inicializar(){
	log_instancia = log_create("Instancia.log", "Instancia", 1, LOG_LEVEL_TRACE);

	pthread_mutex_init(&mutex_dump, NULL);

	leer_config();

	crear_si_no_existe(PUNTO_MONTAJE, log_instancia);
}

void leer_config(){
	t_config* archivo_config = config_create(PATH_CONFIG);

	IP_COORDINADOR = leer_string(archivo_config, "IP_COORDINADOR");

	PUERTO_COORDINADOR = config_get_int_value(archivo_config, "PUERTO_COORDINADOR");

	MI_ID = config_get_int_value(archivo_config, "ID");

	INTERVALO_DUMP = config_get_int_value(archivo_config, "INTERVALO_DUMP");

	PUNTO_MONTAJE = leer_string(archivo_config, "PUNTO_MONTAJE");

	ALGORITMO_REEMPLAZO = leer_string(archivo_config, "ALGORITMO_REEMPLAZO");

	config_destroy(archivo_config);
}

void setup_algoritmo_reemplazo(){
	if(string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CIRC")){
		algoritmo_reemplazo = &reemplazo_circular;
		entrada_a_reemplazar = 0;
	} else if(string_equals_ignore_case(ALGORITMO_REEMPLAZO, "LRU")){
		algoritmo_reemplazo = reemplazo_lru;
	} else if(string_equals_ignore_case(ALGORITMO_REEMPLAZO, "BSU")){
		algoritmo_reemplazo = &reemplazo_bsu;
	}

}

void conectar_con_coordinador(){
	socket_coordinador = conectarse_a_server("instancia", INSTANCIA, "coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, log_instancia);
	if(socket_coordinador < 0){
		//rutina_final();
		exit(1);
	}

	enviar_paquete(ENVIO_ID, socket_coordinador, sizeof(MI_ID), &MI_ID);
}

//todo probablemente haya que ponerlo despues de recibir la config de la tabla de entredas
void enviar_entradas_al_coordinador(){
	int cant_claves = tabla_de_entradas->elements_count;
	int tamanio_paquete = 0;
	void* paquete = NULL;

	void empaquetar_clave(void* void_entrada){
		t_entrada* entrada = (t_entrada*)void_entrada;
		int offset = tamanio_paquete;
		tamanio_paquete += sizeof(int) + string_length(entrada->clave) + 1;
		paquete = realloc(paquete, tamanio_paquete);
		serializar_string(paquete + offset, entrada->clave);
	}

	list_iterate(tabla_de_entradas, empaquetar_clave);
	enviar_paquete(cant_claves, socket_coordinador, tamanio_paquete, paquete);
	free(paquete);
}

void* hilo_dump(void* _){
	while(true){
		sleep(INTERVALO_DUMP);
		pthread_mutex_lock(&mutex_dump);
		//wait() seria para que la instancia no atienda pedidos
		list_iterate(tabla_de_entradas, persistir);
		//signal()
		pthread_mutex_unlock(&mutex_dump);
	}
	return NULL;
}

void escuchar_coordinador(){
	int protocolo;

	while(1) {
		protocolo = recibir_protocolo(socket_coordinador);
		pthread_mutex_lock(&mutex_dump);
		//printf("leer_protocolo %d\n", protocolo);
		if (protocolo <= 0) {
			log_error(log_instancia, "se desconecto el coordinador");
			//rutina_final();
			break;//exit(1);
		}
		leer_protocolo(protocolo);
		pthread_mutex_unlock(&mutex_dump);
	}

	close(socket_coordinador);

}

void leer_protocolo(int protocolo){
	int resultado;

	switch(protocolo){
	case CONFIGURACION_ENTRADAS:
		configuracion_entradas();
		return;
	case OPERACION_SET:
		resultado = atender_set();
		break;
	case OPERACION_STORE:
		resultado = atender_store();
		break;
	case CREAR_CLAVE:
		resultado = atender_crear_clave();
		break;
//	case BORRAR_CLAVES_REMOVIDAS: //TODO @chakl tens que borrar claves /cantclaves / tam 8 / clave / tam 5/ clave2
//		borrar_claves_removidas();
//		return;

		/*
		int recibir_claves(t_instancia* instancia){
			int cant_claves;

			if(recv(instancia->socket, &cant_claves, sizeof(int), MSG_WAITALL) <= 0){
				log_error(LOG_COORD, "No se pudo recibir la cantidad de claves de la instancia %d", instancia->id);
				return -1;
			}

			log_info(LOG_COORD, "Se recibieron las claves de la instancia");

			int i;
			for(i = 0; i < cant_claves; i++){
				char* clave = recibir_string(instancia->socket);
				agregar_clave(instancia, clave);
			}

			log_info(LOG_COORD, "Se agregaron las claves a la instancia %d", instancia->id);
			return 0;
		}
		 */
	case COMPACTACION:
		atender_compactacion();//todo ver si deberia retornar algo
		return;
	}

	enviar_paquete(resultado, socket_coordinador, 0, NULL);
}

void configuracion_entradas(){
	recv(socket_coordinador, &CANTIDAD_ENTRADAS_TOTALES, sizeof(CANTIDAD_ENTRADAS_TOTALES), MSG_WAITALL);
	recv(socket_coordinador, &TAMANIO_ENTRADA, sizeof(TAMANIO_ENTRADA), MSG_WAITALL);

	crear_tabla_de_entradas();
	setup_algoritmo_reemplazo();
	//todo acomodar
	enviar_entradas_al_coordinador();
}

void crear_tabla_de_entradas(){
	memoria = malloc(TAMANIO_ENTRADA * CANTIDAD_ENTRADAS_TOTALES);

	tabla_de_entradas = list_create();
	crear_bitarray();
	t_entrada* entrada;

	DIR* d = opendir(PUNTO_MONTAJE);
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

void crear_bitarray(){

	int tamanio_en_bytes = CANTIDAD_ENTRADAS_TOTALES/8;
	if(CANTIDAD_ENTRADAS_TOTALES%8 != 0)
		tamanio_en_bytes += 1;

	char* bitarray = malloc(tamanio_en_bytes);

	bitarray_entradas = bitarray_create_with_mode(bitarray, tamanio_en_bytes, LSB_FIRST);
}

t_entrada* levantar_entrada(char* nombre){
	t_entrada* entrada_nueva = crear_entrada(nombre);

	entrada_nueva->tamanio_bytes_clave = tamanio_entrada_en_disco(nombre);

	entrada_nueva->tamanio_entradas_clave = entradas_ocupadas(entrada_nueva->tamanio_bytes_clave);

	//no lo chequeo por que se usa solo en la inicializacion (no va a pasar que tenga menos espacio que un estado anterior)
	entrada_nueva->nro_entrada = entrada_para(entrada_nueva->tamanio_entradas_clave);

	levantar_entrada_a_memoria(entrada_nueva);

	setear_bitarray(entrada_nueva);

	return entrada_nueva;
}

t_entrada* crear_entrada(char* clave){
	t_entrada* entrada = malloc(sizeof(t_entrada));
	entrada->clave = string_new();
	string_append(&entrada->clave, clave);

	return entrada;
}

int tamanio_entrada_en_disco(char* nombre){
	int file_desc = abrir_entrada(nombre);
	struct stat stat_entrada = crear_stat(file_desc);

	int tamanio = stat_entrada.st_size;

	close(file_desc);
	return tamanio;
}

void levantar_entrada_a_memoria(t_entrada* entrada){
	int file_desc = abrir_entrada(entrada->clave);
	struct stat stat_entrada = crear_stat(file_desc);

	void* valor = mi_mmap(file_desc, stat_entrada);

	copiar_a_memoria(entrada, valor);

	munmap(valor, entrada->tamanio_bytes_clave);
	close(file_desc);
}

void copiar_a_memoria(t_entrada* entrada, void* valor){
	int offset = TAMANIO_ENTRADA * entrada->nro_entrada;
	memcpy(memoria + offset, valor, entrada->tamanio_bytes_clave);
}

int abrir_entrada(char* nombre){
	char* aux = ruta_absoluta(nombre);

	int fd = open(aux, O_RDWR | O_CREAT, 0666);
	if (fd == -1) {
		log_error(log_instancia, "no se puedo abrir la entrada: %s", nombre);
		close(fd);
		//rutina_final();
		exit(1);
	}

	free(aux);
	return fd;
}

char* ruta_absoluta(char* nombre){
	return string_from_format("%s/%s", PUNTO_MONTAJE, nombre);
}

struct stat crear_stat(int fd){
	struct stat stat;
	if(fstat(fd, &stat) < 0){
		log_error(log_instancia, "no se pudo cargar los atributos de una entrada");
		close(fd);
		//rutina_final();
		exit(1);
	}
	return stat;
}

void* mi_mmap(int fd, struct stat stat){
	void* data = mmap(0, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(data == MAP_FAILED){
		log_error(log_instancia, "no se pudo mapear uan entrada");
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
		entradas_ocupadas++;

	return entradas_ocupadas;
}

int entrada_para(int cant_entradas){
	int i = 0, entradas_libres;

	while(i < CANTIDAD_ENTRADAS_TOTALES){
		if(!bitarray_test_bit(bitarray_entradas, i)){
			entradas_libres = entradas_libres_desde(i);
			if(entradas_libres >= cant_entradas)
				return i;

			i += entradas_libres;
		}

		i++;
	}

	return -1;
}

int entradas_libres_desde(int nro_entrada){
	int i;
	int entradas_restantes = CANTIDAD_ENTRADAS_TOTALES - nro_entrada;
	for(i = 0; i < entradas_restantes; i++){
		if(bitarray_test_bit(bitarray_entradas, nro_entrada + i)){
			break;
		}
	}

	return i;
}

void setear_bitarray(t_entrada* entrada){
	int i;
	for(i = 0; i < entrada->tamanio_entradas_clave; i++)
		bitarray_set_bit(bitarray_entradas, entrada->nro_entrada + i);
}

int atender_set(){
	char* clave = recibir_string(socket_coordinador);
	char* valor = recibir_string(socket_coordinador);

	//todo tendria que actualizar cosas de lsu
	return modificar_entrada(clave, valor);
}

int modificar_entrada(char* clave, char* valor){
	t_entrada* entrada = buscar_entrada(clave, buscar_entrada_clave);
	int entradas_nuevo_valor = entradas_ocupadas(string_length(valor));
	int entradas_faltantes = entradas_nuevo_valor - entrada->tamanio_entradas_clave;
	int resultado = OPERACION_EXITOSA;

	if(entradas_nuevo_valor == entrada->tamanio_entradas_clave){
		actualizar_valor_entrada(entrada, valor);
	} else {
		int entradas_libres = entradas_libres_desde(entrada->nro_entrada + entrada->tamanio_entradas_clave);
		if(entradas_nuevo_valor > entrada->tamanio_bytes_clave && entradas_libres < entradas_faltantes)
			resultado = entradas_disponibles() > entradas_faltantes ? FS_NC : FS_EI;//todo es necesario el FS_NC? o deberia ver si entra en otra parte?
		else
			actualizar_tamanio_entrada(entrada, valor);
	}

	free(valor);
	free(clave);
	return resultado;
}

int entradas_disponibles(){
	int i, contador = 0;
	for(i = 0; i < CANTIDAD_ENTRADAS_TOTALES; i++)
		if(!bitarray_test_bit(bitarray_entradas, i))
			contador++;

	return contador;
}

void actualizar_tamanio_entrada(t_entrada* entrada, char* valor){
	liberar_entradas_desde(entrada->nro_entrada, entrada->tamanio_entradas_clave);
	actualizar_valor_entrada(entrada, valor);
	setear_bitarray(entrada);
}

void actualizar_valor_entrada(t_entrada* entrada, char* valor){
	entrada->tamanio_bytes_clave = string_length(valor);
	entrada->tamanio_entradas_clave = entradas_ocupadas(string_length(valor));
	copiar_a_memoria(entrada, valor);
}

void liberar_entradas_desde(int desde_entrada, int cantidad){
	int i;
	for(i = 0; i < cantidad; i++)
		bitarray_clean_bit(bitarray_entradas, desde_entrada + i);
}

int atender_store(){
	char* clave = recibir_string(socket_coordinador);
	int resultado = OPERACION_EXITOSA;

	t_entrada* entrada = buscar_entrada(clave, buscar_entrada_clave);

	if(entrada == NULL)
		resultado = ERROR_CLAVE_NO_IDENTIFICADA;
	else
		persistir(entrada);

	free(clave);
	return resultado;
}

void persistir(void* entrada_void){
	t_entrada* entrada = (t_entrada*)entrada_void;
	//puts(entrada->clave);
	int file_desc = abrir_entrada(entrada->clave);
	ftruncate(file_desc, entrada->tamanio_bytes_clave);
	struct stat stat_entrada = crear_stat(file_desc);
	void* archivo_valor = mi_mmap(file_desc, stat_entrada);

	int offset = TAMANIO_ENTRADA * entrada->nro_entrada;
	memcpy(archivo_valor, memoria + offset, entrada->tamanio_bytes_clave);

	munmap(archivo_valor, stat_entrada.st_size);
	close(file_desc);
}

int atender_crear_clave(){
	char* clave = recibir_string(socket_coordinador);
	char* valor = "hola";//TODO recibir_string(socket_coordinador); //TODO SACAR MOCK PT

	//puts(clave);
	int entradas_nuevo_valor = entradas_ocupadas(string_length(valor));
	int nro_entrada = entrada_para(entradas_nuevo_valor);
	int resultado = OPERACION_EXITOSA;

	//todo deberia usar el algoritmo para nuevos valores que ocupen mas de una entrada?
	if(nro_entrada == -1 && entradas_nuevo_valor == 1){
		resultado = buscar_entrada_para_reemplazar(clave, valor);
	} else {
		t_entrada* entrada = crear_entrada(clave);
		entrada->nro_entrada = nro_entrada;
		actualizar_valor_entrada(entrada, valor);
		list_add(tabla_de_entradas, entrada);
	}

	free(clave);
	//free(valor);
	return resultado;
}

int buscar_entrada_para_reemplazar(char* clave, char* valor){
	int entrada_reemplazada = algoritmo_reemplazo(clave, valor);

	//todo creo que solo seria si no hay entradas atomicas
	if(entrada_reemplazada == -1)
		return FALLO_REEMPLAZO;

	reemplazar_entrada(entrada_reemplazada, clave, valor);
	return OPERACION_EXITOSA;
}

void reemplazar_entrada(int nro_entrada, char* clave, char* valor){
	t_entrada* entrada = buscar_entrada(&nro_entrada, buscar_entrada_nro);

	eliminar_entrada(entrada->clave);
	actualizar_valor_entrada(entrada, valor);
	free(entrada->clave);
	entrada->clave = string_new();
	string_append(&entrada->clave, clave);
}

void eliminar_entrada(char* nombre){
	char* aux = ruta_absoluta(nombre);
	remove(aux);
	free(aux);
}

int reemplazo_circular(char* clave, char* valor){
	int entrada_inicial = entrada_a_reemplazar;
	bool hay_entrada = false;
	do {
		if(es_nro_entrada_atomica(entrada_a_reemplazar)){
			hay_entrada = true;
			break;
		}

		entrada_a_reemplazar = siguiente_entrada(entrada_a_reemplazar);
	} while(entrada_a_reemplazar != entrada_inicial);

	if(!hay_entrada)//todo si dio toda la vuelta, deberia avanzar la entrada_a_reemplazar?
		return -1;

	int entrada = entrada_a_reemplazar;
	entrada_a_reemplazar = siguiente_entrada(entrada_a_reemplazar);
	return entrada;
}

bool es_nro_entrada_atomica(int nro_entrada){
	//todo ver si no es necesario pasar el nro_entrada a void*
	t_entrada* entrada = buscar_entrada(&nro_entrada, buscar_entrada_nro);

	return entrada == NULL ? false : entrada->tamanio_entradas_clave == 1;
}

t_entrada* buscar_entrada(void* buscado, bool (*comparador)(void*, void*)){
	//TODO ver que esto funcione
	bool comparar_clave(void* void_clave){
		return comparador(void_clave, buscado);
	}
	t_entrada* entrada = (t_entrada*)list_find(tabla_de_entradas, comparar_clave);

	return entrada;
}

bool buscar_entrada_clave(void* entrada_void, void* clave_void){
	t_entrada* entrada_aux = (t_entrada*)entrada_void;

	return string_equals_ignore_case(entrada_aux->clave, clave_void);
}

bool buscar_entrada_nro(void* entrada_void, void* nro_void){
	t_entrada* entrada_aux = (t_entrada*)entrada_void;
	int* nro_aux = (int*)nro_void;
	return entrada_aux->nro_entrada == *nro_aux;
}

int siguiente_entrada(int nro_entrada){
	return nro_entrada == CANTIDAD_ENTRADAS_TOTALES - 1 ? 0 : nro_entrada + 1;
}

int reemplazo_bsu(char* clave, char* valor){
	t_list* entradas_atomicas = list_filter(tabla_de_entradas, es_entrada_atomica);

	if(list_is_empty(entradas_atomicas))
		return -1;

	list_sort(tabla_de_entradas, mayor_entrada);

	t_entrada* entrada_a_reemplazar = list_get(entradas_atomicas, 0);
	//ToDO habria que ver si mas de una cumple y desempatar
	return entrada_a_reemplazar->nro_entrada;
}

bool es_entrada_atomica(void* entrada_void){
	t_entrada* entrada = (t_entrada*)entrada_void;

	return entrada->tamanio_entradas_clave == 1;
}

bool mayor_entrada(void* entrada1, void* entrada2){
	return ((t_entrada*)entrada1)->tamanio_bytes_clave > ((t_entrada*)entrada2)->tamanio_bytes_clave;
}

int reemplazo_lru(char* clave, char* valor){
	return -1;//buscador_generico()
}

void atender_compactacion(){
	list_sort(tabla_de_entradas, menor_nro_entrada);
	int primer_entrada_libre = entrada_para(1);

	if(primer_entrada_libre == -1)
		return;//quiere decir que no hay espacio libre

	void compactar_entrada(void* void_entrada){
		t_entrada* entrada = (t_entrada*)void_entrada;
		if(entrada->nro_entrada < primer_entrada_libre)
			return;

		char* valor = obtener_valor_de(entrada);
		entrada->nro_entrada = primer_entrada_libre;
		copiar_a_memoria(entrada, valor);
		free(valor);

		primer_entrada_libre = entrada_para(1);
	}

	list_iterate(tabla_de_entradas, compactar_entrada);
}

bool menor_nro_entrada(void* entrada1, void* entrada2){
	return ((t_entrada*)entrada1)->nro_entrada < ((t_entrada*)entrada2)->nro_entrada;
}

char* obtener_valor_de(t_entrada* entrada){
	int offset = TAMANIO_ENTRADA * entrada->nro_entrada;
	int tamanio_valor = entrada->tamanio_bytes_clave + 1;
	char* valor = malloc(tamanio_valor);

	memcpy(valor, memoria + offset, entrada->tamanio_bytes_clave);
	valor[tamanio_valor] = '\0';
	return valor;
}



