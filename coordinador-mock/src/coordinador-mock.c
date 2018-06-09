
#include <stdio.h>
#include <stdlib.h>
#include "coordinador-mock.h"

int no_main(void){
	elegir_opcion();

	return 0;
}

int main(void) {
	struct sockaddr_in remoteaddr; // dirección del cliente
	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	int newfd;        // descriptor de socket de nueva conexión aceptada
	int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
	int addrlen;
	int socket;
	int protocolo_cliente;

	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	log = log_create("Coordinador-Mock.log", "Coordinador Mock", 1, LOG_LEVEL_TRACE);

	// CREO SOCKET PARA ESCUCHAR CONEXIONES ENTRANTES

	listener = crear_socket();

	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		log_error(log, "Error en el setsockopt");
		exit(1);
	}

	bindear_socket(listener, MI_IP, MI_PUERTO, log);

	lista_esis = list_create();

	// LO PONGO A ESCUCHAR LAS CONEXIONES NUEVAS Y PETICIONES DE LAS POSTERIORMENTE EXISTENTES
	if (listen(listener, 10) == -1) {
		log_error(log, "No se pudo poner el listener a escuchar");
		exit(1);
	}
	// añadir listener al conjunto maestro
	FD_SET(listener, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste


	//CICLO PRINCIPAL DE EJECUCION

	for(;;) {
		read_fds = master;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {

			log_error(log, "Fallo en el select! Re raro.");
			exit(1);

		}else{
			// explorar conexiones existentes en busca de datos que leer
			for(socket = 0; socket <= fdmax; socket++) {
				if (FD_ISSET(socket, &read_fds)) { // ¡¡tenemos datos!!
					if (socket == listener) {
						// ATIENDO A LOS NUEVOS CLIENTES Y LES DOY LA BIENVENIDA, SOLO SI SON GENTE DE BIEN (?

						addrlen = sizeof(remoteaddr);
						if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen)) == -1) {
							log_error(log, "ERROR: no se pudo aceptar la conexion del socket");
						} else {
							aniadir_cliente(&master, newfd, &fdmax);
						}

					} else {
						// GESTIONO PETICIONES DE LOS CLIENTES CONOCIDOS
						protocolo_cliente = recibir_protocolo(socket);

						if(protocolo_cliente < 0)
							desconectar_cliente(socket);

						else
							atender_protocolo(protocolo_cliente, socket);

					}
				}
			}

		}

	}

}

void aniadir_cliente(fd_set* master, int cliente, int* fdmax){
	FD_SET(cliente, master);//añadir al conjunto maestro

	log_trace(log, "Se conecto el cliente %d", cliente);

	if (cliente > *fdmax)//actualizar el máximo
		*fdmax = cliente;

	if(informar_conexion_exitosa_a(cliente) <= 0)
		log_error(log, "ERROR: no se pudo informar al cliente %d", cliente);

	atender_handshake(cliente);
}

void atender_handshake(int socket_cliente){
	int remitente = recibir_handshake(socket_cliente);

	if(remitente != ESI && remitente != PLANIFICADOR && remitente != CONSOLA_PLANIFICADOR){

		if(remitente < 0){ //Error en recv o me llego un protocolo que no era handshake

			log_trace(log, "Error en el handshake");

		}else { //El cliente no es ninguno de los esperados ni un error en recv

			log_trace(log, "Cliente desconocido detectado y rechazado");
			informar_desconexion(socket_cliente);	//No me interesa catchear el error del send

		}

		desconectar_cliente(socket_cliente);

	} else { //El cliente es un ESI

		if(remitente == ESI)
			log_trace(log, "Se realizo el handshake con el ESI del socket %d", socket_cliente);

		else if(remitente == CONSOLA_PLANIFICADOR){
			log_trace(log, "Se realizo el handshake con la consola del Planificador en el socket %d", socket_cliente);
			SOCKET_CONSOLA_PLANIFICADOR = socket_cliente;
		}
		else {
			log_trace(log, "Se realizo el handshake con el Planificador en el socket %d", socket_cliente);
			SOCKET_PLANIFICADOR = socket_cliente;
		}

		informar_conexion_exitosa_a(socket_cliente);
	}
}

void armar_nuevo_esi(int socket){
	int id;

	if(recv(socket, &id, sizeof(int), MSG_WAITALL) < 0){
		log_error(log, "Fallo la recepcion del ID del esi conectado al socket %d", socket);
		desconectar_cliente(socket);
		return;
	}

	log_info(log, "Se recibio el ID (%d) del ESI conectado en el socket %d", id, socket);

	t_esi* nuevo_esi = (t_esi*)malloc(sizeof(t_esi));

	nuevo_esi->id = id;
	nuevo_esi->socket = socket;

	list_add(lista_esis, nuevo_esi);
}

void atender_protocolo(int protocolo, int socket_cliente){
	int id = obtener_id_desde_socket(socket_cliente);

	switch (protocolo){
		case ENVIO_ID:
			armar_nuevo_esi(socket_cliente);
			break;
		case OPERACION_SET:
			atender_set(socket_cliente, id);
			break;

		case OPERACION_GET:
			atender_get(socket_cliente, id);
			break;

		case OPERACION_STORE:
			atender_store(socket_cliente, id);
			break;

		case STATUS:
			atender_status();
			break;
		default:
			desconectar_cliente(socket_cliente);
	}
}

int obtener_id_desde_socket(int socket){
	bool esi_de_socket_buscado(void* elem){
		t_esi* esi = (t_esi*)elem;

		return esi->socket == socket;
	}

	t_esi* esi = list_find(lista_esis, esi_de_socket_buscado);

	if(esi!=NULL){
		return esi->id;
	}

	return -1;
}

void atender_get(int socket, int id_esi){
	int tamanio;

	recv(socket, &tamanio, sizeof(int), MSG_WAITALL);

	log_debug(log, "Recibi el tamanio de clave %d", tamanio);

	char* clave = string_new();

	recv(socket, clave, tamanio, MSG_WAITALL);

	log_debug(log, "Recibi la clave %s", clave);

	log_info(log, "Se recibio la operacion GET sobre la clave %s del esi de id %d", clave, id_esi);

	//MOCK
//	int opcion_elegida = elegir_opcion();
//
//	free(clave);
//
//	enviar_paquete(opcion_elegida, socket, 0, NULL);
	//FIN MOCK

	sleep(10);

	int tam_paquete = 2* sizeof(int) + tamanio;
	void* paquete = malloc(tam_paquete);

	memcpy(paquete, &id_esi, sizeof(int));
	memcpy(paquete + sizeof(int), &tamanio, sizeof(int));
	memcpy(paquete + (2* sizeof(int)), clave, tamanio);

	if(enviar_paquete(GET_CLAVE, SOCKET_PLANIFICADOR, tam_paquete, paquete) == -1){
		log_error(log, "Se perdio la conexion con el planificador");
		enviar_paquete(ERROR_DE_COMUNICACION, socket, 0, NULL);
		close(SOCKET_PLANIFICADOR);

		free(paquete);
		free(clave);

		return;
	}

	free(paquete);
	free(clave);

	int protocolo = recibir_protocolo(SOCKET_PLANIFICADOR);

	if(protocolo == -1){
		log_error(log, "Se perdio la conexion con el planificador");
		enviar_paquete(ERROR_DE_COMUNICACION, socket, 0, NULL);
		close(SOCKET_PLANIFICADOR);
		return;
	}

	enviar_paquete(protocolo, socket, 0, NULL);

}

void atender_store(int socket, int id_esi){
	int tamanio;

	recv(socket, &tamanio, sizeof(int), MSG_WAITALL);

	char* clave = string_new();

	recv(socket, clave, tamanio, MSG_WAITALL);

	log_info(log, "Se recibio la operacion STORE sobre la clave %s del esi de id %d", clave, id_esi);
	//MOCK
//	int opcion_elegida = elegir_opcion();
//
//	free(clave);
//
//	enviar_paquete(opcion_elegida, socket, 0, NULL);
	//FIN MOCK

	int tam_paquete = 2* sizeof(int) + tamanio;
	void* paquete = malloc(tam_paquete);

	memcpy(paquete, &id_esi, sizeof(int));
	memcpy(paquete + sizeof(int), &tamanio, sizeof(int));
	memcpy(paquete + (2* sizeof(int)), clave, tamanio);

	if(enviar_paquete(STORE_CLAVE, SOCKET_PLANIFICADOR, tam_paquete, paquete) == -1){
		log_error(log, "Se perdio la conexion con el planificador");
		enviar_paquete(ERROR_DE_COMUNICACION, socket, 0, NULL);
		close(SOCKET_PLANIFICADOR);

		free(paquete);
		free(clave);

		return;
	}

	free(paquete);
	free(clave);

	int protocolo = recibir_protocolo(SOCKET_PLANIFICADOR);

	if(protocolo == -1){
		log_error(log, "Se perdio la conexion con el planificador");
		enviar_paquete(ERROR_DE_COMUNICACION, socket, 0, NULL);
		close(SOCKET_PLANIFICADOR);
		return;
	}

	enviar_paquete(protocolo, socket, 0, NULL);
}

void atender_set(int socket, int id_esi){
	int tamanio_clave;

	recv(socket, &tamanio_clave, sizeof(int), MSG_WAITALL);

	char* clave = string_new();

	recv(socket, clave, tamanio_clave, MSG_WAITALL);

	int tamanio_valor;

	recv(socket, &tamanio_valor, sizeof(int), MSG_WAITALL);

	char* valor = string_new();

	recv(socket, valor, tamanio_valor, MSG_WAITALL);

	log_info(log, "Se recibio la operacion SET sobre la clave %s con el valor %s del esi de id %d", clave, valor, id_esi);

	int tam_paquete = 2* sizeof(int) + tamanio_clave;
	void* paquete = malloc(tam_paquete);

	memcpy(paquete, &id_esi, sizeof(int));
	memcpy(paquete + sizeof(int), &tamanio_clave, sizeof(int));
	memcpy(paquete + (2* sizeof(int)), clave, tamanio_clave);

	if(enviar_paquete(SET_CLAVE, SOCKET_PLANIFICADOR, tam_paquete, paquete) == -1){
		log_error(log, "Se perdio la conexion con el planificador");
		enviar_paquete(ERROR_DE_COMUNICACION, socket, 0, NULL);
		close(SOCKET_PLANIFICADOR);

		free(paquete);
		free(clave);

		return;
	}

	free(paquete);
	free(clave);

	int protocolo = recibir_protocolo(SOCKET_PLANIFICADOR);

	if(protocolo == -1){
		log_error(log, "Se perdio la conexion con el planificador");
		enviar_paquete(ERROR_DE_COMUNICACION, socket, 0, NULL);
		close(SOCKET_PLANIFICADOR);
		return;
	}

	enviar_paquete(protocolo, socket, 0, NULL);
}

void atender_status(){

	log_info(log, "Estoy por atender el status");

	int tamanio_clave;

	if(recv(SOCKET_CONSOLA_PLANIFICADOR, &tamanio_clave, sizeof(int), MSG_WAITALL) < 0){
		log_error(log, "Se perdio la conexion con la consola del planificador");

		desconectar_cliente(SOCKET_CONSOLA_PLANIFICADOR);
	}

	log_info(log, "Recibi el tamanio de clave %d ", tamanio_clave);

	char* clave = malloc(sizeof(tamanio_clave));

	if(recv(SOCKET_CONSOLA_PLANIFICADOR, clave, tamanio_clave, MSG_WAITALL ) < 0){
		log_error(log, "Se perdio la conexion con la consola del planificador");

		desconectar_cliente(SOCKET_CONSOLA_PLANIFICADOR);
	}

	log_info(log, "Recibi la clave %s de tamanio %d", clave, tamanio_clave);

	t_info_status* info_status = malloc(sizeof(t_info_status));

	info_status->id_instancia_actual = 1;
	info_status->id_instancia_posible = 1;
	info_status->mensaje = string_from_format("Esto es el valor de la clave");
	info_status->tamanio_mensaje = strlen(info_status->mensaje) + 1;

	log_info(log ,"Info Status: tam_msg = %d | msg = %s | id_actual = %d | id_posible = %d", info_status->tamanio_mensaje, info_status->mensaje, info_status->id_instancia_actual, info_status->id_instancia_posible);

	int tamanio_paquete;

	void* paquete = serializar_info_status(info_status, &tamanio_paquete);

	log_info(log, "Ya serialize");

	if(enviar_paquete(ENVIO_INFO_STATUS, SOCKET_CONSOLA_PLANIFICADOR, tamanio_paquete, paquete) < 0){
		log_error(log, "Se perdio la conexion con la consola del planificador");

		desconectar_cliente(SOCKET_CONSOLA_PLANIFICADOR);
	}
}

int elegir_opcion(){

	mostrar_menu();

	int opcion;

	scanf("%d", &opcion);

	switch (opcion){
		case 1: // EJECUCION_EXITOSA
//			 log_trace(log, "Se va a enviar EJECUCION_EXITOSA");

			 return EJECUCION_EXITOSA;

		case 2: // ERROR_TAMANIO_CLAVE
			 log_trace(log, "Se va a enviar ERROR_TAMANIO_CLAVE");

			 return ERROR_TAMANIO_CLAVE;

		case 3: // ERROR_CLAVE_NO_IDENTIFICADA
			log_trace(log, "Se va a enviar ERROR_CLAVE_NO_IDENTIFICADA");

			return ERROR_CLAVE_NO_IDENTIFICADA;

		case 4: // ERROR_DE_COMUNICACION
			log_trace(log, "Se va a enviar ERROR_DE_COMUNICACION");

			return ERROR_DE_COMUNICACION;

		case 5: // ERROR_CLAVE_INACCESIBLE
			log_trace(log, "Se va a enviar ERROR_CLAVE_INACCESIBLE");

			return ERROR_CLAVE_INACCESIBLE;

		default: return -1;
	}
}


void mostrar_menu(){

	char* opciones[] = {
			"1: EJECUCION_EXITOSA",
			"2: ERROR_TAMANIO_CLAVE",
			"3: ERROR_CLAVE_NO_IDENTIFICADA",
			"4: ERROR_DE_COMUNICACION",
			"5: ERROR_CLAVE_INEXISTENTE",
			NULL
	};

	int i;

	for(i=0; opciones[i]; i++){
		printf("%s\n", opciones[i]);
		fflush(stdout);
	}

}

void desconectar_cliente(int cliente){
	//error o conexión cerrada por el cliente
	log_trace(log, "Se desconecto el cliente %d", cliente);

	void funcion_al_pedo(void* esi){};

	bool es_un_esi(void* elem){
		t_esi* esi = (t_esi*)elem;

		return esi->socket == cliente;
	}

	list_remove_and_destroy_by_condition(lista_esis, es_un_esi, funcion_al_pedo);

	close(cliente); // bye!
	FD_CLR(cliente, &master);
}
