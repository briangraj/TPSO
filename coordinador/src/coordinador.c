#include "coordinador.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "conexiones/threads.h"

int main(void) {
	setup_coord();

	int listener = crear_socket();// creo socket para escuchar conexiones entrantes

	bindear_socket_server(listener);

	// lo pongo a escuchar conexiones nuevas
	if (listen(listener, 10) == -1) {
		log_error(LOG_COORD, "No se pudo poner el socket a escuchar");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in remoteaddr; // direccion del cliente
	int addrlen;

	while(true){

		addrlen = sizeof(remoteaddr);

		int socket_cliente = accept(listener, (struct sockaddr *) &remoteaddr, &addrlen);

		if (socket_cliente == -1)
			log_error(LOG_COORD, "ERROR: no se pudo aceptar la conexion del socket %d", socket_cliente);
		else {
			informar_conexion_exitosa_a(socket_cliente);//FIXME que onda con esto? le avisa que se conecto bien 2 veces?
			atender_handshake(socket_cliente);
		}

	}

}

void setup_coord(){
	LOG_COORD = log_create("Coordinador.log", "Coordinador", 1, LOG_LEVEL_TRACE);

	leer_config();

	INSTANCIAS = list_create();//t_instancia
}

void leer_config(){//FIXME hardcodeado
	IP_COORD = "127.0.0.1";
	PUERTO_COORD = 5051;
	CANTIDAD_ENTRADAS_TOTALES = 20;
	TAMANIO_ENTRADA = 100;
}

void bindear_socket_server(int listener) {
	int yes = 1;

	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1){
		log_error(LOG_COORD, "Error en el setsockopt");
		exit(EXIT_FAILURE);
	}

	bindear_socket(listener, IP_COORD, PUERTO_COORD, LOG_COORD);
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

		log_trace(LOG_COORD, "Se realizo el handshake con el ESI en el socket %d", socket_cliente);

		crear_hilo_esi(socket_cliente);
	break;
	case PLANIFICADOR:
		/**
		 * TODO ¿El planificador puede conectarse al coordinador si no hay instancias conectadas?
		 * ¿Cuantas instancias?
		 * Deberia haber 1 solo planificador
		 */
		informar_conexion_exitosa_a(socket_cliente);

		log_trace(LOG_COORD, "Se realizo el handshake con el Planificador en el socket %d", socket_cliente);

		pthread_mutex_init(&SEM_SOCKET_PLANIF, NULL);

		SOCKET_PLANIF = socket_cliente;
	break;
	case INSTANCIA: {
		/**
		 * TODO acordarse de pedirle el id a la instancia
		 * y agregarla a la lista de instancias
		 */

		informar_conexion_exitosa_a(socket_cliente);

		int id = recibir_id(socket_cliente);

		if(id <= 0){
			log_error(LOG_COORD, "No se pudo recibir el id de la instancia conectada en el socket %d", socket_cliente);
			return;
		}

		t_instancia* instancia = crear_instancia(id, socket_cliente);

		list_add(INSTANCIAS, (void*) instancia);

		log_trace(LOG_COORD, "Se realizo el handshake con la Instancia en el socket %d", socket_cliente);

		crear_hilo_instancia(instancia);
	}
	break;
	default:
//		errores
	;
	}
}

void desconectar_cliente(int cliente){
	log_trace(LOG_COORD, "Se desconecto el cliente %d", cliente);
	close(cliente);
}
