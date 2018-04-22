/*
 * hilo_esi.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_esi.h"

void crear_hilo_esi(int socket_cliente){

	crear_hilo(atender_esi, (void*) socket_cliente);

}

void* atender_esi(void* socket_esi){
	while()
	t_solicitud solicitud = recibir_solicitud_esi((int) socket_esi);


	return 0;

}


t_solicitud recibir_solicitud_esi(int socket){
	t_solicitud solicitud;

	int protocolo = recibir_protocolo(socket);

	if(protocolo == -1){
		log_error(log, "El esi %d envio una solicitud invalida", socket);
		EXIT(exit_failu);
	}
	solicitud.clave = recibir_string(socket);

	switch(protocolo){

	case GET:
		solicitud.instruccion = GET;

	case SET:
		solicitud.instruccion = SET;

	case STORE:
		solicitud.instruccion = STORE;
	}
	return solicitud;
}

char* recibir_string(int socket){
	int tam_clave;

	recv(socket, &tam_clave, sizeof(int), MSG_WAITALL);

	char* clave = malloc(tam_clave);

	recv(socket, clave, tam_clave, MSG_WAITALL);

	return clave;
}





























