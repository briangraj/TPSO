/*
 * serializacion.c
 *
 *  Created on: 1/10/2017
 *      Author: utnso
 */
#include "serializacion.h"

int enviar_paquete(int protocolo, int sockfd, int tamanio_payload, void* payload){ //TODO delegar en una funcion que agarre 2 void* y te los una //TODO chequear send si no manda todos los bytes
	size_t tamanio_mensaje = tamanio_payload + sizeof(int);
	int return_value;

	void* mensaje = malloc(tamanio_mensaje);

	memcpy(mensaje, &protocolo, sizeof(int));
	memcpy(mensaje + sizeof(int), payload, tamanio_payload);

	return_value = send_all(sockfd, mensaje, tamanio_mensaje);
	free(mensaje);

	return return_value;
}

int send_all(int socket, void* mensaje, int tamanio_mensaje){
	ssize_t n;
	char* p = (char*)mensaje;
	while (tamanio_mensaje > 0) {
		n = send(socket, p, tamanio_mensaje, 0);
		if (n <= 0)
			return -1;
		p += n;
		tamanio_mensaje -= n;
	}
	return 1;
}

int recibir_protocolo(int socket_fd){
	int protocolo;

	if(recv(socket_fd, &protocolo, TAM_HEADER, MSG_WAITALL) <= 0) return -1;

	return protocolo;
}

//Podria usarse para informar el exito del handshake
int informar_conexion_exitosa_a(int socket_cliente){
	return enviar_paquete(CONEXION_EXITOSA, socket_cliente, 0, NULL);
}

int recibir_informe(int socket_servidor){// FIXME wtf is this
	int protocolo = recibir_protocolo(socket_servidor);

	if(protocolo == DESCONEXION_INMINENTE) return -1;

	return 0; //Si nada fallo retorna 0

	//Usada por los clientes para saber si se pudo conectar al servidor o si fallo el handshake (podria tener otros usos)
	/*
	 * La idea es que cuando un cliente se quiera conectar a un server, espere una respuesta de este ultimo antes de hacer el handshake,
	 * y de paso reutilizarla para verificar que el handshake no fue rechazado por el server (Sujeto a posibles cambios)
	*/
}

int realizar_handshake(int id_proceso, int socket_servidor){
	int tamanio_payload = sizeof(int);
	int return_value;
	void* payload = malloc(tamanio_payload);

	memcpy(payload, &id_proceso, tamanio_payload);

	return_value = enviar_paquete(HANDSHAKE, socket_servidor,tamanio_payload, payload);
	free(payload);

	return return_value;
}

int recibir_handshake(int socket_fd){//todo esta bien que devuelva -1 siendo 2 errores distintos?
	int remitente;

	if(recibir_protocolo(socket_fd) != HANDSHAKE)
		return -2;

	if(recv(socket_fd, &remitente, TAM_PAQUETE_HANDSHAKE, 0) < 0)
		return -1;

	return remitente;
}

int informar_desconexion(int socket_cliente){//todo se puede agregar close(socket_cliente);?
	return enviar_paquete(DESCONEXION_INMINENTE, socket_cliente, 0, NULL);
}

void* serializar_ruta(char* fs_path){
	int tamanio_ruta = strlen(fs_path) + 1;

	void* payload = malloc(tamanio_ruta + sizeof(int)); //Como el receptor de la ruta no conoce el tamaño del string,
															//este ultimo tiene que estar incluido al principio del paquete
	memcpy(payload, &tamanio_ruta, sizeof(int));

	memcpy(payload + sizeof(int), fs_path, tamanio_ruta);

	return payload;
}

t_ruta deserializar_ruta (int socket_cliente){
	t_ruta ruta_recibida;

	recv(socket_cliente, (void*) &ruta_recibida.tamanio, sizeof(int), MSG_WAITALL);

	ruta_recibida.ruta = malloc(ruta_recibida.tamanio);

	recv(socket_cliente, (void*) ruta_recibida.ruta, ruta_recibida.tamanio, MSG_WAITALL);

	return ruta_recibida;//Acordarse de hacer un free despues de usarse
}


int informar_resultado_de_operacion(int socket, int resultado_operacion){
	return enviar_paquete(resultado_operacion, socket, 0, NULL);

	// Retorna int porque si send retorna < 0 podes logear el error que quieras en tu proceso en tu proceso...

	/*
	 * EJEMPLO :
	 * 		informar_resultado_de_operacion(cliente, TRANSFORMACION_REALIZADA);
	 * 		informar_resultado_de_operacion(cliente, ERROR_EN_TRANSFORMACION);
	 * */

}

int informar_fallo_recv(t_log* logger, char* string){

	log_error(logger, "ERROR: Fallo el recv. %s", string);

	return -1;

}



