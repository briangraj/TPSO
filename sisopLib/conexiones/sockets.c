/*
 * cliente.c
 *
 *  Created on: 20/9/2017
 *      Author: utnso
 */
#include "sockets.h"

int crear_socket(){
	int _socket = socket(AF_INET, SOCK_STREAM, 0);

	return _socket; // CONTROLAR POR FUERA SI EL VALOR RETORNADO FUE -1 !!
}

struct sockaddr_in cargar_direccion(char ip[], int puerto){
	struct sockaddr_in address;

	address.sin_family = AF_INET;// Ordenación de bytes de la máquina
	address.sin_addr.s_addr = inet_addr(ip);// FIXME ESTA DEBE SER LA IP DEL NODO, la toma por archivo de config
	address.sin_port = htons(puerto);// short, Ordenación de bytes de la red
	memset(&(address.sin_zero), '\0', 8);// poner a cero el resto de la estructura

	return address;
}

int conectar_socket_a(char ip[], int puerto, int sockfd){
	struct sockaddr_in address = cargar_direccion(ip, puerto); // información de la dirección de destino

	return connect(sockfd, (struct sockaddr *)&address, sizeof(struct sockaddr));
}

void bindear_socket(int listener, char ip[], int puerto, t_log* log){
	struct sockaddr_in address = cargar_direccion(ip, puerto);
	size_t tamanio_dir_server = sizeof(address);
	if(bind(listener, (struct sockaddr *) &address, tamanio_dir_server) == -1){
		log_error(log, "ERROR: no se pudo bindear el socket");
		exit(1);
	};
}

