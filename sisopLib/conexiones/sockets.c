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

int conectarse_a_server(char* nombre_cliente, int id_cliente, char* nombre_server, char* ip_server, int puerto_server, int socket_server, t_log* log_cliente){
	socket_server = crear_socket();

	if(socket_server < 0) {
		log_error(log_cliente, "No se pudo crear el socket para conectarse al proceso %s, se finaliza el proceso %s.", nombre_server, nombre_cliente);
		return -1;
	}

	if (conectar_socket_a(ip_server, puerto_server, socket_server) == -1) {
		log_error(log_cliente, "El proceso %s no se pudo conectar al proceso %s por el socket %d",nombre_cliente, nombre_server, socket_server);
		close(socket_server);
		return -1;
	}

	log_trace(log_cliente, "El proceso %s se conecto al proceso %s con exito", nombre_cliente, nombre_server);

	if(recibir_protocolo(socket_server) != CONEXION_EXITOSA){
		log_error(log_cliente, "El proceso %s denego la conexion", nombre_server);
		close(socket_server);
		return -1;
	}

	log_trace(log_cliente, "El proceso %s confirmo la conexion con el proceso %s", nombre_server, nombre_cliente);

	if(realizar_handshake(id_cliente, socket_server) <= 0){
		log_error(log_cliente, "No se pudo iniciar el handshake con el proceso %s", nombre_server);
		close(socket_server);
		return -1;
	}

	log_trace(log_cliente, "Se inicio el handshake con el proceso %s", nombre_server);

	if(recibir_protocolo(socket_server) != CONEXION_EXITOSA){
		log_error(log_cliente, "No se pudo completar el handshake con el proceso %s", nombre_server);
		close(socket_server);
		return -1;
	}


	log_trace(log_cliente, "Se completo el handshake con el proceso %s", nombre_server);

	return socket_server;
}






