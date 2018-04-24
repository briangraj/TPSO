#include "coordinador.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "conexiones/threads.h"

int main(void) {
	log_coord = log_create("Coordinador.log", "Coordinador", 1, LOG_LEVEL_TRACE);

	int listener = crear_socket();// creo socket para escuchar conexiones entrantes

	int yes=1;// para setsockopt() SO_REUSEADDR, más abajo

	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		log_error(log_coord, "Error en el setsockopt");
		exit(EXIT_FAILURE);
	}

	leer_config();

	bindear_socket(listener, ip_coord, puerto_coord, log_coord);

	// lo pongo a escuchar conexiones nuevas
	if (listen(listener, 10) == -1) {
		log_error(log_coord, "No se pudo poner el socket a escuchar");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in remoteaddr; // direccion del cliente
	int addrlen;

	while(true){

		addrlen = sizeof(remoteaddr);

		int socket_cliente = accept(listener, (struct sockaddr *) &remoteaddr, &addrlen);

		if (socket_cliente == -1)
			log_error(log_coord, "ERROR: no se pudo aceptar la conexion del socket %d", socket_cliente);
		else {
			informar_conexion_exitosa_a(socket_cliente);
			atender_handshake(socket_cliente);
		}

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

void leer_config(){
	ip_coord= "127.0.0.1";
	puerto_coord = 5051;
}

void atender_handshake(int socket_cliente){
	int remitente = recibir_handshake(socket_cliente);

	switch(remitente){

	case ESI:
		/**
		 * TODO ¿Hay que validar que haya un planificador conectado?
		 * ¿E instancias conectadas?
		 */
		informar_conexion_exitosa_a(socket_cliente);

		log_trace(log_coord, "Se realizo el handshake con el ESI en el socket %d", socket_cliente);

		crear_hilo_esi(socket_cliente);
	break;
	case PLANIFICADOR:
		/**
		 * TODO ¿El planificador puede conectarse al coordinador si no hay instancias conectadas?
		 * ¿Cuantas instancias?
		 */
		informar_conexion_exitosa_a(socket_cliente);

		log_trace(log_coord, "Se realizo el handshake con el Planificador en el socket %d", socket_cliente);

		crear_hilo_planificador(socket_cliente); //TODO falta hacer
	break;
	case INSTANCIA:
		/**
		 * TODO acordarse de pedirle el id a la instancia
		 * y agregarla a la lista de instancias
		 */
		informar_conexion_exitosa_a(socket_cliente);

		log_trace(log_coord, "Se realizo el handshake con la Instancia en el socket %d", socket_cliente);

		crear_hilo_instancia(socket_cliente);
	break;
	default:
//		errores
	;
	}
}

void desconectar_cliente(int cliente){
	log_trace(log_coord, "Se desconecto el cliente %d", cliente);
	close(cliente);
}
