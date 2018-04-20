#include "coordinador.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "conexiones/threads.h"

int main(void) {
	log = log_create("Coordinador.log", "Coordinador", 1, LOG_LEVEL_TRACE);

	int listener = crear_socket();// creo socket para escuchar conexiones entrantes

	int yes=1;// para setsockopt() SO_REUSEADDR, más abajo

	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		log_error(log, "Error en el setsockopt");
		exit(EXIT_FAILURE);
	}

	bindear_socket(listener, MI_IP, MI_PUERTO, log);

	// lo pongo a escuchar conexiones nuevas
	if (listen(listener, 10) == -1) {
		log_error(log, "No se pudo poner el socket a escuchar");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in remoteaddr; // direccion del cliente
	int addrlen;

	while(true){

		addrlen = sizeof(remoteaddr);

		int socket_cliente = accept(listener, (struct sockaddr *) &remoteaddr, &addrlen);

		if (socket_cliente == -1)
			log_error(log, "ERROR: no se pudo aceptar la conexion del socket %d", socket_cliente);
		else
			atender_handshake(socket_cliente);

	}

//						// GESTIONO PETICIONES DE LOS CLIENTES CONOCIDOS
//						protocolo_cliente = recibir_protocolo(socket);
//
//						if(protocolo_cliente < 0)
//							desconectar_cliente(socket);
//
//						else
//							atender_protocolo(protocolo_cliente, socket);

}


void atender_handshake(int socket_cliente){
	int remitente = recibir_handshake(socket_cliente);

	switch(remitente){

	case ESI:
		log_trace(log, "Se realizo el handshake con el ESI en el socket %d", socket_cliente);

		informar_conexion_exitosa_a(socket_cliente);

//		crear_hilo();
	break;
	case PLANIFICADOR:
		log_trace(log, "Se realizo el handshake con el Planificador en el socket %d", socket_cliente);

		informar_conexion_exitosa_a(socket_cliente);

//		crear_hilo();
	break;
	case INSTANCIA:
		log_trace(log, "Se realizo el handshake con la Instancia en el socket %d", socket_cliente);

		informar_conexion_exitosa_a(socket_cliente);

//		crear_hilo();
	break;
	default:
//		errores
	;
}

void atender_protocolo(int protocolo, int socket_cliente){
	log_debug(log, "Llegamos hasta atender protocolo!!! Recibi el protocolo %d", protocolo);
}

void desconectar_cliente(int cliente){
	//error o conexión cerrada por el cliente
	log_trace(log, "Se desconecto el cliente %d", cliente);

	close(cliente); // bye!
}

