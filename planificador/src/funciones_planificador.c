#include "funciones_planificador.h"

void signal_handler(int sig_num){
	if(sig_num == SIGUSR1){
		log_warning(log_planif, "Se recibió la señal para finalizar el planificador");
		finalizar();
	}
}

void iniciar_planificador(int loggear){
	log_planif = log_create("Planificador.log", "Planificador", loggear, LOG_LEVEL_TRACE);

	leer_archivo_config();

	if((SOCKET_COORDINADOR = conectarse_a_coordinador(PLANIFICADOR)) == -1)
		exit(1);

	cola_de_listos = list_create();
	cola_finalizados = list_create();
	colas_de_bloqueados = list_create();
	colas_de_asignaciones = list_create();

	pthread_mutex_init(&semaforo_pausa, NULL);
	pthread_mutex_init(&semaforo_cola_bloqueados, NULL);
	pthread_mutex_init(&semaforo_cola_listos, NULL);
	pthread_mutex_init(&semaforo_asignaciones, NULL);
	pthread_mutex_init(&semaforo_flag_bloqueo, NULL);

	flag_bloqueo.clave_bloqueo = string_new();
	flag_bloqueo.hay_que_bloquear_esi_activo = false;
	flag_bloqueo.fue_bloqueado_consola = false;

	PLANIFICADOR_PID = getpid();
}

void leer_archivo_config(){
	archivo_config = config_create("../../configs/planificador.cfg"); // Ante problema con eclipse/consola, cambiarla por ruta absoluta

//	archivo_config = config_create("/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/configs/planificador.cfg");

	IP_PLANIFICADOR = config_get_string_value(archivo_config, "IP_PLANIFICADOR");
	PUERTO_PLANIFICADOR = config_get_int_value(archivo_config, "PUERTO_PLANIFICADOR");
	IP_COORDINADOR = config_get_string_value(archivo_config, "IP_COORDINADOR");
	PUERTO_COORDINADOR = config_get_int_value(archivo_config, "PUERTO_COORDINADOR");
	ESTIMACION_INICIAL = config_get_int_value(archivo_config, "ESTIMACION_INICIAL");
	ALFA_PLANIFICACION = config_get_int_value(archivo_config, "ALFA_PLANIFICACION") / 100;

	char* algoritmo = config_get_string_value(archivo_config, "ALGORITMO_PLANIFICACION");

	if(!strcmp(algoritmo, "SJF_SD"))
		ALGORITMO_PLANIFICACION = SJF_SD;

	else if(!strcmp(algoritmo, "SJF_CD"))
		ALGORITMO_PLANIFICACION = SJF_CD;

	else if(!strcmp(algoritmo, "HRRN"))
		ALGORITMO_PLANIFICACION = HRRN;

	else {
		log_error(log_planif, "No se reconoce el algoritmo de planificacion");
		log_destroy(log_planif);
		config_destroy(archivo_config);
		exit(1);
	}
}

void aniadir_cliente(fd_set* master, int cliente, int* fdmax){
	FD_SET(cliente, master);//añadir al conjunto maestro

	log_trace(log_planif, "Se conecto el cliente %d", cliente);

	if (cliente > *fdmax)//actualizar el máximo
		*fdmax = cliente;

	if(informar_conexion_exitosa_a(cliente) <= 0)
		log_error(log_planif, "ERROR: no se pudo informar al cliente %d", cliente);

	atender_handshake(cliente);
}

void atender_handshake(int socket_cliente){
	int remitente = recibir_handshake(socket_cliente);

	if(remitente != ESI){

		if(remitente < 0){ //Error en recv o me llego un protocolo que no era handshake

			log_trace(log_planif, "Error en el handshake");

		}else { //El cliente no es ESI ni un error en recv

			log_trace(log_planif, "Cliente desconocido detectado y rechazado");
			informar_desconexion(socket_cliente);	//No me interesa catchear el error del send

		}

		desconectar_cliente(socket_cliente);

	} else { //El cliente es un ESI

		log_trace(log_planif, "Nuevo ESI detectado y aceptado");
		informar_conexion_exitosa_a(socket_cliente);

		enviar_paquete(ENVIO_ID, socket_cliente, sizeof(int), &proximo_id);

//		// SOLO PARA PROBAR QUE ESTE ANDANDO EL ESI
//
//		ejecutar_mock(socket_cliente);

	}


}

void atender_protocolo(int protocolo, int socket_cliente){
//	// SOLO PARA PROBAR QUE ANDE EL ESI

	switch (protocolo){
		case EJECUCION_EXITOSA:

			pthread_mutex_lock(&semaforo_flag_bloqueo);
			bool hay_que_bloquear = flag_bloqueo.hay_que_bloquear_esi_activo;
			char* clave_bloqueo = strdup(flag_bloqueo.clave_bloqueo);
			bool fue_bloqueado_por_consola = flag_bloqueo.fue_bloqueado_consola;
			pthread_mutex_unlock(&semaforo_flag_bloqueo);

			if(hay_que_bloquear)
				bloquear_esi_activo(clave_bloqueo, fue_bloqueado_por_consola);

			free(clave_bloqueo);

			mandar_a_ejecutar();

			break;
		case FALLO_EN_EJECUCION:
			mover_a_finalizados(esi_activo(), "Fallo en la ejecucion de una instruccion del script");
			mandar_a_ejecutar();
			break;
		case FIN_DEL_SCRIPT:
			mover_a_finalizados(esi_activo(), "Finalizado con exito");
			mandar_a_ejecutar();
			break;
		case GET_CLAVE:
			log_debug(log_planif, "Recibi GET_CLAVE");
			atender_get_clave();
			break;
		case STORE_CLAVE:
			atender_store();
			break;
		case SET_CLAVE:
			atender_set();
			break;
	}
}

void aniadir_a_colas_de_asignaciones(t_ready nuevo_esi){
	t_recursos_por_esi* recursos_por_esi = (t_recursos_por_esi*) malloc(sizeof(t_recursos_por_esi));

	recursos_por_esi->id_esi = nuevo_esi.ID;

	recursos_por_esi->recursos_asignados = list_create();

	pthread_mutex_lock(&semaforo_asignaciones);
	list_add(colas_de_asignaciones, recursos_por_esi);
	pthread_mutex_unlock(&semaforo_asignaciones);

}

int recibir_id_esi(){
	int id_esi;

	if(recv(SOCKET_COORDINADOR, &id_esi, sizeof(int), MSG_WAITALL) < 0){
		log_error(log_planif, "Se perdio la conexion con el coordinador");
		finalizar();
	}

	log_info(log_planif, "Recibi el id %d", id_esi);

	return id_esi;
}

char* recibir_clave(){
	int tamanio_clave;

	if(recv(SOCKET_COORDINADOR, &tamanio_clave, sizeof(int), MSG_WAITALL) < 0){
		log_error(log_planif, "Se perdio la conexion con el coordinador");
		finalizar();
	}

	char* clave = (char*) malloc(tamanio_clave);

	if(recv(SOCKET_COORDINADOR, clave, tamanio_clave, MSG_WAITALL) < 0){
		log_error(log_planif, "Se perdio la conexion con el coordinador");
		finalizar();
	}

	log_info(log_planif, "Recibi la clave %s de tamanio %d", clave, tamanio_clave);

	return clave;
}

void atender_get_clave(){
	int id_esi = recibir_id_esi();

	char* clave = recibir_clave();

	int resultado_get = intentar_asignar(id_esi, clave);

	if(enviar_paquete(resultado_get, SOCKET_COORDINADOR, 0, NULL) < 0){
		log_error(log_planif, "Se perdio la conexion con el coordinador");
		finalizar();
	}
}

int intentar_asignar(int id_esi, char* recurso){

	t_bloqueados_por_clave* bloqueados_por_clave = encontrar_bloqueados_para_la_clave(recurso);

	if(!bloqueados_por_clave){
		crear_entrada_bloqueados_del_recurso(id_esi, recurso);

		asignar_recurso_al_esi(id_esi, recurso);

		return GET_EXITOSO;
	}

	// el de == 0 es el caso en donde estan todos los demas bloqueados por consola
	if(bloqueados_por_clave->id_proximo_esi == id_esi || bloqueados_por_clave->id_proximo_esi == 0){
		asignar_recurso_al_esi(id_esi, recurso);

		return GET_EXITOSO;
	}

	hay_que_bloquear_esi_activo(recurso, false);

	return GET_BLOQUEANTE;
}

void hay_que_bloquear_esi_activo(char* clave, bool fue_bloqueado_por_consola){

	pthread_mutex_lock(&semaforo_flag_bloqueo);
	flag_bloqueo.hay_que_bloquear_esi_activo = true;
	free(flag_bloqueo.clave_bloqueo);
	flag_bloqueo.clave_bloqueo = strdup(clave);
	flag_bloqueo.fue_bloqueado_consola = fue_bloqueado_por_consola;
	pthread_mutex_unlock(&semaforo_flag_bloqueo);

}

void bloquear_esi_activo(char* clave, bool fue_bloqueado_por_consola){
	t_ready* esi = esi_activo();

	t_bloqueados_por_clave* bloqueados_por_clave = encontrar_bloqueados_para_la_clave(clave);

	t_blocked* esi_bloqueado = crear_t_bloqueado(esi);

	if(fue_bloqueado_por_consola){
		esi_bloqueado->bloqueado_por_consola = true;
		esi_bloqueado->bloqueado_por_ejecucion = false;
	}

	list_add(bloqueados_por_clave->bloqueados, esi_bloqueado);

	pthread_mutex_lock(&semaforo_cola_listos);
	list_remove_and_destroy_element(cola_de_listos, 0, funcion_al_pedo);
	pthread_mutex_unlock(&semaforo_cola_listos);

	pthread_mutex_lock(&semaforo_flag_bloqueo);
	flag_bloqueo.hay_que_bloquear_esi_activo = false;
	flag_bloqueo.fue_bloqueado_consola = false;
	pthread_mutex_unlock(&semaforo_flag_bloqueo);

}

void crear_entrada_bloqueados_del_recurso(int id_esi, char* recurso){

	char* clave = strdup(recurso);

	t_bloqueados_por_clave* bloqueados_por_clave = (t_bloqueados_por_clave*) malloc(sizeof(t_bloqueados_por_clave));

	bloqueados_por_clave->bloqueados = list_create();
	bloqueados_por_clave->clave = clave;
	bloqueados_por_clave->id_proximo_esi = id_esi;

	pthread_mutex_lock(&semaforo_cola_bloqueados);
	list_add(colas_de_bloqueados, bloqueados_por_clave);
	pthread_mutex_unlock(&semaforo_cola_bloqueados);

}

void asignar_recurso_al_esi(int id_esi, char* recurso){
	bool es_del_esi(void* elem){
		t_recursos_por_esi* rec = (t_recursos_por_esi*) elem;

		return rec->id_esi == id_esi;
	}

	bool es_la_clave(void* elem){
		t_bloqueados_por_clave* bloqueados_por_clave = (t_bloqueados_por_clave*) elem;

		return string_equals_ignore_case(bloqueados_por_clave->clave, recurso);
	}

	t_recursos_por_esi* recursos_por_esi = list_find(colas_de_asignaciones, es_del_esi);

	if(recursos_por_esi == NULL){
		log_error(log_planif, "La lista de asignaciones del esi %d no esta, wtf", id_esi);
		finalizar();
	}

	char* clave = strdup(recurso);

	pthread_mutex_lock(&semaforo_asignaciones);
	list_add(recursos_por_esi->recursos_asignados, clave);
	pthread_mutex_unlock(&semaforo_asignaciones);

	pthread_mutex_lock(&semaforo_cola_bloqueados);
	t_bloqueados_por_clave* bloqueados_de_la_clave = list_find(colas_de_bloqueados, es_la_clave);
	bloqueados_de_la_clave->id_proximo_esi = id_esi;
	pthread_mutex_unlock(&semaforo_cola_bloqueados);
}

t_blocked* crear_t_bloqueado(t_ready* esi_ready){
	t_blocked* bloqueado = (t_blocked*) malloc (sizeof(t_blocked));

	bloqueado->bloqueado_por_consola = false;
	bloqueado->bloqueado_por_ejecucion = true;
	bloqueado->info_ejecucion = malloc(sizeof(t_ready));
	memcpy(bloqueado->info_ejecucion, esi_ready, sizeof(t_ready));

	return bloqueado;
}

t_ready* esi_activo(){
	bool es_el_que_esta_ejecutando(void* elem) {
		t_ready* esi = (t_ready*)elem;

		return esi->ID == id_esi_activo;
	}

	pthread_mutex_lock(&semaforo_cola_listos);
	t_ready* esi_resultado = list_find(cola_de_listos, es_el_que_esta_ejecutando);
	pthread_mutex_unlock(&semaforo_cola_listos);

	return esi_resultado;
}

void atender_store(){
	int id_esi = recibir_id_esi();
	char* clave = recibir_clave();

	int resultado_store = actualizar_cola_de_bloqueados_para(id_esi, clave);

	free(clave);

	if(enviar_paquete(resultado_store, SOCKET_COORDINADOR, 0, NULL) < 0){
		log_error(log_planif, "Se perdio la conexion con el Coordinador");
		finalizar();
	}

}

void atender_set(){
	int id_esi = recibir_id_esi();
	char* clave = recibir_clave();
	int resultado_set;

	if(verificar_tenencia_de_la_clave(id_esi, clave))
		resultado_set = SET_EXITOSO;
	else
		resultado_set = SET_INVALIDO;

	free(clave);

	if(enviar_paquete(resultado_set, SOCKET_COORDINADOR, 0, NULL) < 0){
		log_error(log_planif, "Se perdio la conexion con el Coordinador");
		finalizar();
	}

}

void desconectar_cliente(int cliente){
	//error o conexión cerrada por el cliente
	log_trace(log_planif, "Se desconecto el cliente %d", cliente);

	close(cliente); // bye!
	FD_CLR(cliente, &master);
}

int conectarse_a_coordinador(int remitente){

	int socket;

	if((socket = conectarse_a_server("Planificador", remitente, "Coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, log_planif)) == -1){
		log_destroy(log_planif);
		config_destroy(archivo_config);
		return -1;
	}

	return socket;
}

void aniadir_a_listos(t_ready esi){
	t_ready* esi_ready = duplicar_esi_ready(esi);

	planificar(esi_ready);
	//Cuando finaliza esta funcion.... que pasa con esi_ready? se pierde? porque se quedaría basura en la lista
}

void imprimir_estado_cola_listos(){

	log_info(log_planif, "El estado de la cola de listos es:");

	void imprimir_listo(void* elem){
		t_ready* esi = (t_ready*) elem;

		log_debug(log_planif, "ESI %d", esi->ID);
	}

	pthread_mutex_lock(&semaforo_cola_listos);
	list_iterate(cola_de_listos, imprimir_listo);
	pthread_mutex_unlock(&semaforo_cola_listos);

}

t_ready* duplicar_esi_ready(t_ready esi){
	t_ready* resultado = (t_ready*) malloc(sizeof(t_ready));

	resultado->ID = esi.ID;
	resultado->estimacion_actual = esi.estimacion_actual;
	resultado->socket = esi.socket;
	resultado->tiempo_espera = esi.tiempo_espera;
	resultado->ultima_estimacion = esi.ultima_estimacion;
	resultado->ultima_rafaga_real = esi.ultima_rafaga_real;

	return resultado;
}

void planificar(t_ready* esi_ready){

	pthread_mutex_lock(&semaforo_cola_listos);
	int cant_listos = cola_de_listos->elements_count;
	pthread_mutex_unlock(&semaforo_cola_listos);

	if(cant_listos == 0){

		pthread_mutex_lock(&semaforo_cola_listos);
		list_add(cola_de_listos, esi_ready);
		pthread_mutex_unlock(&semaforo_cola_listos);

		imprimir_estado_cola_listos();
		mandar_a_ejecutar();

	} else
		insertar_ordenado(esi_ready);

//	imprimir_estado_cola_listos();
}

void mandar_a_ejecutar(){//Enviar orden de ejecucion al primer ESI de la cola de listos


	pthread_mutex_lock(&semaforo_pausa);
	pthread_mutex_unlock(&semaforo_pausa);

	pthread_mutex_lock(&semaforo_cola_listos);
	if(list_is_empty(cola_de_listos)){
		log_debug(log_planif, "La cola de listos esta vacia");
		id_esi_activo = 0;
		pthread_mutex_unlock(&semaforo_cola_listos);
		return;
	}

	t_ready* esi_ejecucion = list_get(cola_de_listos, 0);
	pthread_mutex_unlock(&semaforo_cola_listos);

	log_trace(log_planif, "Se le va a dar la orden de ejecucion al ESI de ID %d", esi_ejecucion->ID);

	if(enviar_paquete(EJECUTAR_SENTENCIA, esi_ejecucion->socket, 0, NULL) == -1){
		mover_a_finalizados(esi_ejecucion, "Error en la comunicacion ESI-Planificador");
		mandar_a_ejecutar();
	} else {
		id_esi_activo = esi_ejecucion->ID;
		actualizar_esperas();
	}

}

void mover_a_finalizados(t_ready* esi_ejecucion, char* exit_text){

	t_ended* esi_finalizado = (t_ended*) malloc(sizeof(t_ended));

	esi_finalizado->ID = esi_ejecucion->ID;
	esi_finalizado->exit_text = strdup(exit_text);

	bool coincide_el_id(void* elemento){
		t_ready* esi = (t_ready*) elemento;

		return esi->ID == esi_finalizado->ID;
	}

	// Si no pudo enviar es porque el ESI del otro lado ya se murio (esto pasa cuando finaliza correctamente), no nos interesa controlar este caso.
	enviar_paquete(FINALIZAR_PROCESO, esi_ejecucion->socket, 0, NULL);

	desconectar_cliente(esi_ejecucion->socket);

	// si esta en la cola de ready pasa esto
	pthread_mutex_lock(&semaforo_cola_listos);
	list_remove_and_destroy_by_condition(cola_de_listos, coincide_el_id, funcion_al_pedo);
	pthread_mutex_unlock(&semaforo_cola_listos);

	// si esta en la cola de bloqueados pasa esto
	eliminar_de_bloqueados(esi_ejecucion);

	actualizar_disponibilidad_recursos(esi_finalizado->ID);

	pthread_mutex_lock(&semaforo_asignaciones);
	list_remove_and_destroy_by_condition(colas_de_asignaciones, coincide_el_id, asignacion_destroyer);
	pthread_mutex_unlock(&semaforo_asignaciones);

}

void eliminar_de_bloqueados(t_ready* esi){
	bool es_el_esi(void* elem){
		t_blocked* esi_bloqueado = (t_blocked*) elem;

		return esi_bloqueado->info_ejecucion->ID == esi->ID;
	}

	bool es_el_recurso_que_lo_tiene(void* elem){
		t_bloqueados_por_clave* bloqueados_por_clave = (t_bloqueados_por_clave*) elem;

		bool resultado = list_any_satisfy(bloqueados_por_clave->bloqueados, es_el_esi);

		return resultado;

	}

	pthread_mutex_lock(&semaforo_cola_bloqueados);
	t_bloqueados_por_clave* bloqueados_por_clave = list_find(colas_de_bloqueados, es_el_recurso_que_lo_tiene);
	pthread_mutex_unlock(&semaforo_cola_bloqueados);

	if(bloqueados_por_clave == NULL) return;

	pthread_mutex_lock(&semaforo_cola_bloqueados);
	list_remove_and_destroy_by_condition(bloqueados_por_clave->bloqueados, es_el_esi, blocked_destroyer);
	pthread_mutex_unlock(&semaforo_cola_bloqueados);

}

void actualizar_disponibilidad_recursos(int id_esi){

	bool es_esi_buscado(void* elem){
		t_recursos_por_esi* recursos_por_esi = (t_recursos_por_esi*) elem;

		return recursos_por_esi->id_esi == id_esi;
	}

	pthread_mutex_lock(&semaforo_asignaciones);
	t_recursos_por_esi* recursos_por_esi = list_find(colas_de_asignaciones, es_esi_buscado);
	pthread_mutex_unlock(&semaforo_asignaciones);

	if(recursos_por_esi == NULL){
		log_error(log_planif, "No se encontro al esi %d en la cola de asignaciones, wtf", id_esi);
		finalizar();
	}

	void actualizar_proximo_con_derecho(void* elem){
		char* recurso = (char*) elem;

		actualizar_cola_de_bloqueados_para(id_esi, recurso);
	}

	pthread_mutex_lock(&semaforo_asignaciones);
	list_iterate(recursos_por_esi->recursos_asignados, actualizar_proximo_con_derecho);
	pthread_mutex_unlock(&semaforo_asignaciones);

}

t_bloqueados_por_clave* encontrar_bloqueados_para_la_clave(char* recurso){

	bool es_el_recurso(void* elem){
		t_bloqueados_por_clave* bloq = (t_bloqueados_por_clave*) elem;

		return string_equals_ignore_case(bloq->clave, recurso);
	}

	pthread_mutex_lock(&semaforo_cola_bloqueados);
	t_bloqueados_por_clave* resultado = list_find(colas_de_bloqueados, es_el_recurso);
	pthread_mutex_unlock(&semaforo_cola_bloqueados);

	return resultado;
}

int actualizar_cola_de_bloqueados_para(int id_esi_que_lo_libero, char* recurso){


	t_bloqueados_por_clave* bloqueados_de_la_clave = encontrar_bloqueados_para_la_clave(recurso);

	if(bloqueados_de_la_clave == NULL){
		log_error(log_planif, "No se encontro la lista del recurso %s, wtf", recurso);
		finalizar();
	}

	// el tipo que hizo store no tiene en verdad la clave!
	// solo se llega a este if haciendo store, sino siempre va a coincidir el id
	if(!(verificar_tenencia_de_la_clave(id_esi_que_lo_libero, recurso)))
		return STORE_INVALIDO;

	bool es_el_esi_con_el_recurso(void* elem){
		t_recursos_por_esi* rec = (t_recursos_por_esi*)elem;

		return rec->id_esi == id_esi_que_lo_libero;
	}

	pthread_mutex_lock(&semaforo_asignaciones);
	t_recursos_por_esi* recursos_del_esi = list_find(colas_de_asignaciones, es_el_esi_con_el_recurso);
	pthread_mutex_unlock(&semaforo_asignaciones);

	bool es_el_recurso_asignado(void* elem){
		char* clave = (char*)elem;

		return string_equals_ignore_case(clave, recurso);
	}

	pthread_mutex_lock(&semaforo_asignaciones);
	list_remove_and_destroy_by_condition(recursos_del_esi->recursos_asignados, es_el_recurso_asignado, funcion_al_pedo);
	pthread_mutex_unlock(&semaforo_asignaciones);

	bool es_el_recurso(void* elem){
		t_bloqueados_por_clave* bloq = (t_bloqueados_por_clave*) elem;

		return string_equals_ignore_case(bloq->clave, recurso);
	}

	if(id_esi_que_lo_libero == bloqueados_de_la_clave->id_proximo_esi
			&& list_is_empty(bloqueados_de_la_clave->bloqueados)){

		pthread_mutex_lock(&semaforo_cola_bloqueados);
		list_remove_and_destroy_by_condition(colas_de_bloqueados, es_el_recurso, clave_destroyer);
		pthread_mutex_unlock(&semaforo_cola_bloqueados);

	}else{
		actualizar_privilegiado(bloqueados_de_la_clave);
	}

	return STORE_EXITOSO;
}

bool verificar_tenencia_de_la_clave(int id_esi, char* clave){

	bool es_el_recurso_buscado(void* elem){
		char* recurso = (char*) elem;

		return string_equals_ignore_case(recurso, clave);
	}

	bool es_el_esi(void* elem){
		t_recursos_por_esi* recursos_del_esi = (t_recursos_por_esi*) elem;

		return recursos_del_esi->id_esi == id_esi;
	}

	pthread_mutex_lock(&semaforo_asignaciones);
	t_recursos_por_esi* recursos_del_esi = list_find(colas_de_asignaciones, es_el_esi);

	bool resultado = list_any_satisfy(recursos_del_esi->recursos_asignados, es_el_recurso_buscado);
	pthread_mutex_unlock(&semaforo_asignaciones);

	return resultado;
}


void actualizar_privilegiado(t_bloqueados_por_clave* bloqueados_de_la_clave){

	t_blocked* proximo_con_derecho = proximo_no_bloqueado_por_consola(bloqueados_de_la_clave->bloqueados);

	if(proximo_con_derecho == NULL){
		bloqueados_de_la_clave->id_proximo_esi = 0; // estan todos bloqueados por consola

		return;
	}

	bloqueados_de_la_clave->id_proximo_esi = proximo_con_derecho->info_ejecucion->ID;

	aniadir_a_listos(*(proximo_con_derecho->info_ejecucion));

	blocked_destroyer(proximo_con_derecho);

}

t_blocked* proximo_no_bloqueado_por_consola(t_list* bloqueados){

	bool proximo(void* elem){
		t_blocked* proximo = (t_blocked*) elem;

		return !proximo->bloqueado_por_consola;
	}

	return list_find(bloqueados, proximo);

}

void blocked_destroyer(void* elem){
	t_blocked* bloqueado = (t_blocked*) elem;
	free(bloqueado->info_ejecucion);
}

void actualizar_esperas(){

	pthread_mutex_lock(&semaforo_cola_listos);
	t_ready* primer_esi = list_remove(cola_de_listos, 0);
	pthread_mutex_unlock(&semaforo_cola_listos);

	primer_esi->tiempo_espera = 0.0;
	primer_esi->ultima_rafaga_real += 1.0;

	void aumentar_espera(void* elemento){
		t_ready* esi = (t_ready*) elemento;

		esi->tiempo_espera += 1.0;
	}

	pthread_mutex_lock(&semaforo_cola_listos);
	list_iterate(cola_de_listos, aumentar_espera);

	list_add_in_index(cola_de_listos, 0, primer_esi);//TODO: REVISAR SI ESTO HACE LO QUE QUEREMOS
	pthread_mutex_unlock(&semaforo_cola_listos);

}

t_ready* buscar_en_bloqueados(int id_esi){
	bool es_el_esi(void* elem){
		t_blocked* esi_bloqueado = (t_blocked*) elem;

		return esi_bloqueado->info_ejecucion->ID == id_esi;
	}

	bool es_el_recurso_que_lo_tiene(void* elem){
		t_bloqueados_por_clave* bloqueados_por_clave = (t_bloqueados_por_clave*) elem;

		bool resultado = list_any_satisfy(bloqueados_por_clave->bloqueados, es_el_esi);

		return resultado;

	}

	pthread_mutex_lock(&semaforo_cola_bloqueados);
	t_bloqueados_por_clave* bloqueados_por_clave = list_find(colas_de_bloqueados, es_el_recurso_que_lo_tiene);
	pthread_mutex_unlock(&semaforo_cola_bloqueados);

	if(!bloqueados_por_clave) return NULL;

	pthread_mutex_lock(&semaforo_cola_bloqueados);
	t_blocked* esi_bloqueado = list_find(bloqueados_por_clave->bloqueados, es_el_esi);
	pthread_mutex_unlock(&semaforo_cola_bloqueados);

	if(!esi_bloqueado) return NULL;

	return esi_bloqueado->info_ejecucion;
}

t_ready* buscar_en_ready(int id_esi){
	bool es_el_esi(void* elem){
		t_ready* esi_ready = (t_ready*) elem;

		return esi_ready->ID == id_esi;
	}

	pthread_mutex_lock(&semaforo_cola_listos);
	t_ready* esi_ready = list_find(cola_de_listos, es_el_esi);
	pthread_mutex_unlock(&semaforo_cola_listos);

	return esi_ready;
}

void insertar_ordenado(t_ready* esi_ready){

	esi_ready->estimacion_actual = estimacion(esi_ready);

	switch(ALGORITMO_PLANIFICACION){
		case SJF_CD:

			comparar_desde(0, comparar_sjf, esi_ready);

			break;

		case SJF_SD:

			comparar_desde(1, comparar_sjf, esi_ready);

			break;

		case HRRN:

			comparar_desde(1, comparar_hrrn, esi_ready);

			break;
	}
}

float estimacion(t_ready* esi_ready){

	//Estimacion actual = estimacion anterior * alfa + ( 1 - alfa) * real anterior

	return esi_ready->ultima_estimacion * ALFA_PLANIFICACION + (1 - ALFA_PLANIFICACION) * esi_ready->ultima_rafaga_real;
}

void comparar_desde(int indice_comparacion, bool (*funcion_comparacion)(void*, void*), t_ready* esi_ready){

	pthread_mutex_lock(&semaforo_cola_listos);

	list_add(cola_de_listos, esi_ready);

	if(indice_comparacion == 0)
		list_sort(cola_de_listos, funcion_comparacion);

	else {

		t_ready* primer_esi = list_remove(cola_de_listos, 0);
		list_sort(cola_de_listos, funcion_comparacion);
		list_add_in_index(cola_de_listos, 0, primer_esi);//TODO: REVISAR SI ESTO HACE LO QUE QUEREMOS

	}
	pthread_mutex_unlock(&semaforo_cola_listos);

}

bool comparar_sjf(void* un_esi, void* otro_esi){
	t_ready* primer_esi = (t_ready*) un_esi;
	t_ready* segundo_esi = (t_ready*) otro_esi;

	return primer_esi->estimacion_actual < segundo_esi->estimacion_actual;
}

bool comparar_hrrn(void* un_esi, void* otro_esi){
	t_ready* primer_esi = (t_ready*) un_esi;
	t_ready* segundo_esi = (t_ready*) otro_esi;

	return ratio(primer_esi) > ratio(segundo_esi);
}

float ratio(t_ready* esi){
	return (esi->tiempo_espera / esi->estimacion_actual) + 1;
}

void finalizar(){
	close(SOCKET_COORDINADOR);
	log_destroy(log_planif);
	config_destroy(archivo_config);

	pthread_mutex_lock(&semaforo_cola_listos);
	list_destroy(cola_de_listos);
	pthread_mutex_unlock(&semaforo_cola_listos);

	list_destroy(cola_finalizados);

	pthread_mutex_lock(&semaforo_cola_bloqueados);
	list_destroy_and_destroy_elements(colas_de_bloqueados, clave_destroyer);
	pthread_mutex_unlock(&semaforo_cola_bloqueados);

	pthread_mutex_lock(&semaforo_asignaciones);
	list_destroy_and_destroy_elements(colas_de_asignaciones, asignacion_destroyer);
	pthread_mutex_unlock(&semaforo_asignaciones);

	pthread_mutex_destroy(&semaforo_pausa);
	pthread_mutex_destroy(&semaforo_cola_bloqueados);
	pthread_mutex_destroy(&semaforo_cola_listos);
	pthread_mutex_destroy(&semaforo_asignaciones);
	pthread_mutex_destroy(&semaforo_flag_bloqueo);

	free(flag_bloqueo.clave_bloqueo);


	exit(1);
}

void asignacion_destroyer(void* elemento){
	t_recursos_por_esi* recursos_por_esi = (t_recursos_por_esi*) elemento;

	list_destroy_and_destroy_elements(recursos_por_esi->recursos_asignados, funcion_al_pedo);
}

void clave_destroyer(void* elemento){
	t_bloqueados_por_clave* bloqueados_por_clave = (t_bloqueados_por_clave*) elemento;
	free(bloqueados_por_clave->clave);
	list_destroy(bloqueados_por_clave->bloqueados);
}


void funcion_al_pedo(void* esi){}

// MOCKS

void ejecutar_mock(int socket_cliente){

	enviar_paquete(EJECUTAR_SENTENCIA, socket_cliente, 0 , NULL);

}





