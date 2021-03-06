/*
 * t_instancia.c
 *
 *  Created on: 4 jun. 2018
 *      Author: utnso
 */

#include "t_instancia.h"

void* crear_instancia(int id, int socket){
	t_instancia* instancia = malloc(sizeof(t_instancia));

	instancia->id = id;
	instancia->solicitudes = queue_create();
	sem_init(&instancia->solicitud_lista, 0, 0);
	instancia->socket_instancia = socket;
	instancia->claves = list_create();
	instancia->claves_a_crear = list_create();
	instancia->claves_a_borrar = list_create();
	instancia->entradas_disponibles = CANTIDAD_ENTRADAS_TOTALES;

	return instancia;
}

bool es_clave_a_crear(t_instancia* instancia, t_solicitud* solicitud){
	return contiene_clave(instancia->claves_a_crear, solicitud);
}

bool contiene_clave(t_list* claves, t_solicitud* solicitud){
	bool contiene_clave(char* clave){
		return string_equals(clave, solicitud->clave);
	}

	return list_any_satisfy(claves, (bool (*)(void*)) contiene_clave);
}

t_instancia* instancia_con_clave(t_solicitud* solicitud){

	bool instancia_contiene_clave(t_instancia* instancia){
		return contiene_clave(instancia->claves, solicitud) || contiene_clave(instancia->claves_a_crear, solicitud);
	}

	return (t_instancia*) list_find(INSTANCIAS, (bool (*)(void*)) instancia_contiene_clave);
}

void borrar_clave(t_solicitud* solicitud, t_instancia* instancia){
	bool contiene_clave(char* clave){
		return string_equals(clave, solicitud->clave);
	}

	char* clave = (char*) list_remove_by_condition(instancia->claves, (bool (*)(void*)) contiene_clave);

	free(clave);
}

void borrar_clave_a_crear(t_solicitud* solicitud, t_instancia* instancia){
	bool contiene_clave(char* clave){
		return string_equals(clave, solicitud->clave);
	}

	list_remove_by_condition(instancia->claves_a_crear, (bool (*)(void*)) contiene_clave);
}

void agregar_clave(t_instancia* instancia, char* clave){
	list_add(instancia->claves, clave);
}

void agregar_clave_a_crear(t_instancia* instancia, char* clave){
	list_add(instancia->claves_a_crear, clave);
}

void agregar_clave_a_borrar(t_instancia* instancia, char* clave){
	list_add(instancia->claves_a_borrar, clave);
}

bool esta_activa(t_instancia* instancia){
	return instancia->esta_activa;
}

t_solicitud* sacar_solicitud(t_instancia* instancia) {
	return (t_solicitud*) queue_pop(instancia->solicitudes);
}

void agregar_solicitud(t_instancia* instancia, t_solicitud* solicitud){
	queue_push(instancia->solicitudes, solicitud);

	sem_post(&instancia->solicitud_lista);
}

int recibir_claves_a_instancia(t_instancia* instancia){
	t_list* claves = recibir_claves(instancia);

	log_info(LOG_COORD, "Se agregaron las claves a la instancia %d", instancia->id);

	if(claves == NULL)
		return -1;

	list_add_all(instancia->claves, claves);

	list_destroy(claves);

	return 0;
}

t_list* recibir_claves(t_instancia* instancia){
	int cant_claves;
	t_list* claves = list_create();

	if(recv(instancia->socket_instancia, &cant_claves, sizeof(int), MSG_WAITALL) <= 0){
		log_error(LOG_COORD, "No se pudo recibir la cantidad de claves de la instancia %d", instancia->id);
		return NULL;
	}

	int i;
	for(i = 0; i < cant_claves; i++){
		char* clave = recibir_string(instancia->socket_instancia);
		list_add(claves, clave);
	}

	return claves;
}

void destruir_instancias(){
	list_destroy_and_destroy_elements(INSTANCIAS, (void (*)(void*)) destruir_instancia);
}

void destruir_instancia(t_instancia* instancia){
	sem_close(&instancia->solicitud_lista);
	sem_destroy(&instancia->solicitud_lista);

	if(list_size(instancia->claves) > 0)
		list_destroy_and_destroy_elements(instancia->claves, free);
	else
		list_destroy(instancia->claves);

	if(list_size(instancia->claves_a_crear) > 0)
		list_destroy_and_destroy_elements(instancia->claves_a_crear, free);
	else
		list_destroy(instancia->claves_a_crear);

	if(list_size(instancia->claves_a_borrar) > 0)
		list_destroy_and_destroy_elements(instancia->claves_a_borrar, free);
	else
		list_destroy(instancia->claves_a_borrar);

	queue_destroy_and_destroy_elements(instancia->solicitudes, (void (*)(void*)) destruir_solicitud);

	if(esta_activa(instancia)){
		pthread_cancel(instancia->id_hilo);//FIXME me parece que esto no va aca
		desconectar_instancia(instancia);
	}

	free(instancia);
}

t_instancia* instancia_de_id(int id){
	bool mismo_id(t_instancia* instancia){
		return instancia->id == id;
	}

	return list_find(INSTANCIAS, (bool (*)(void*)) mismo_id);
}

int conectar_instancia_nueva(t_instancia* instancia){
	if(enviar_config_instancia(instancia) == -1){
		log_error(LOG_COORD, "No se pudo enviar la config a la instancia %d", instancia->id);
		return -1;
	}

	if(recibir_claves_a_instancia(instancia) == -1){
		log_error(LOG_COORD, "No se pudieron recibir las claves de la instancia %d", instancia->id);
		return -1;
	}

	list_add(INSTANCIAS, (void*) instancia);

	instancia->esta_activa = true;

	log_trace(LOG_COORD, "Se agrego la instancia de id %d al sistema", instancia->id);

	return 0;
}

bool hay_claves_a_borrar(t_instancia* instancia){
	return !list_is_empty(instancia->claves_a_borrar);
}

int	reconectar_instancia(t_instancia* instancia, int socket){
	instancia->socket_instancia = socket;

	if(enviar_config_instancia(instancia) == -1){
		log_error(LOG_COORD, "No se pudo enviar la config a la instancia %d", instancia->id);
		return -1;
	}

	t_list* claves = recibir_claves(instancia);

	list_destroy_and_destroy_elements(claves, free);

	if(hay_claves_a_borrar(instancia)){
		if(enviar_claves_a_borrar(instancia) <= 0){
			log_error(LOG_COORD, "No se pudieron enviar las claves a borrar a la instancia %d", instancia->id);
			return -1;
		}

		instancia->entradas_disponibles = recibir_entradas(instancia);

		log_debug(LOG_COORD, "claves de instancia %d antes de borrar", instancia->id);

		mostrar_claves(instancia->claves);

		log_debug(LOG_COORD, "claves a borrar de instancia %d", instancia->id);

		mostrar_claves(instancia->claves_a_borrar);

		borrar_claves_a_borrar(instancia);

		log_debug(LOG_COORD, "claves de instancia %d despues de borrar", instancia->id);

		mostrar_claves(instancia->claves);
	}

	instancia->esta_activa = true;


	return 0;
}

void borrar_claves_a_borrar(t_instancia* instancia){
	borrar_claves(instancia, instancia->claves_a_borrar);

	list_destroy_and_destroy_elements(instancia->claves_a_borrar, free);
}

void mostrar_claves(t_list* claves){
	void mostrar_clave(char* clave){
		log_debug(LOG_COORD, "%s", clave);
	}

	list_iterate(claves, (void (*)(void*)) mostrar_clave);
}

int list_sum(t_list* numeros){
	int acum = 0;

	void acumular(int numero){
		acum += numero;
	}

	list_iterate(numeros, (void (*)(void*)) acumular);

	return acum;
}

t_mensaje serializar_claves_a_borrar(t_instancia* instancia){//TODO testear
//	CLAVES_A_BORRAR | cant_claves + (tam_clave + clave)+
	int cant_claves = list_size(instancia->claves_a_borrar);
	int cant_letras = list_sum(list_map(instancia->claves_a_borrar, (void* (*)(void*)) string_size));

	t_mensaje mensaje = crear_mensaje(CLAVES_A_BORRAR,
			sizeof(int) + sizeof(int) * cant_claves + cant_letras);

	char* aux = mensaje.payload;

	memcpy(aux, &cant_claves, sizeof(int));

	aux += sizeof(int);

	void copiar_clave(char* clave){
		serializar_string(aux, clave);
		aux += sizeof(int) + string_size(clave);
	}

	list_iterate(instancia->claves_a_borrar, (void (*)(void*)) copiar_clave);

	return mensaje;
}

int enviar_claves_a_borrar(t_instancia* instancia){
	t_mensaje claves_a_borrar = serializar_claves_a_borrar(instancia);

	return enviar_mensaje(claves_a_borrar, instancia->socket_instancia);
}

int recibir_entradas(t_instancia* instancia){
	int cant_entradas;
	recv(instancia->socket_instancia, &cant_entradas, sizeof(int), MSG_WAITALL);

	return cant_entradas;
}

char* get_clave(t_instancia* instancia, char* una_clave){
	bool misma_clave(char* otra_clave){
		return string_equals(una_clave, otra_clave);
	}

	char* clave_a_crear = (char*) list_find(instancia->claves_a_crear, (bool (*)(void*)) misma_clave);

	char* clave = (char*) list_find(instancia->claves, (bool (*)(void*)) misma_clave);

	if(clave == NULL){
		if(clave_a_crear == NULL)
			return NULL;

		return clave_a_crear;
	} else
		return clave;

//	return (clave == NULL)? (clave_a_crear == NULL)? NULL : clave;
}

void eliminar_instancia(t_instancia* una_instancia){
	bool misma_instancia(t_instancia* otra_instancia){
		return una_instancia->id == otra_instancia->id;
	}

	list_remove_by_condition(INSTANCIAS, (bool (*)(void*)) misma_instancia);

	destruir_instancia(una_instancia);
}

void desconectar_instancia(t_instancia* instancia){
	instancia->esta_activa = false;

	desconectar_cliente(instancia->socket_instancia);

	log_trace(LOG_COORD, "Se desconecto la instancia %d", instancia->id);
}
