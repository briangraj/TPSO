
#include <stdio.h>
#include <stdlib.h>
#include "coordinador-mock.h"

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
			log_trace(log, "Pase el select");

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

						if(protocolo_cliente < 0){
							FD_CLR(socket, &master);
							log_info(log, "Se desconecto al cliente del socket %d", socket);
							close(socket);
						}

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

	if(remitente != ESI){

		if(remitente < 0){ //Error en recv o me llego un protocolo que no era handshake

			log_trace(log, "Error en el handshake");

		}else { //El cliente no es ESI ni un error en recv

			log_trace(log, "Cliente desconocido detectado y rechazado");
			informar_desconexion(socket_cliente);	//No me interesa catchear el error del send

		}

		desconectar_cliente(socket_cliente);

	} else { //El cliente es un ESI

		log_trace(log, "Nuevo ESI detectado y aceptado");
		informar_conexion_exitosa_a(socket_cliente);

	}
}

void atender_protocolo(int protocolo, int socket_cliente){
	log_trace(log, "Llegamos hasta atender protocolo!!! Recibi el protocolo %d", protocolo);
}

void desconectar_cliente(int cliente){
	//error o conexión cerrada por el cliente
	log_trace(log, "Se desconecto el cliente %d", cliente);

	close(cliente); // bye!
	FD_CLR(cliente, &master);
}

