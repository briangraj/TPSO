/*
 ============================================================================
 Name        : planificador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "funciones_planificador.h"
#include "funciones_consola.h"

int main(int argc, char* argv[]) {
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

	proximo_id = 1;
	id_esi_activo = 0;

	iniciar_planificador(argc > 1);

	if(argc < 2){
		if(crear_hilo((void*)levantar_consola, NULL) != 0){
			log_error(log_planif, "No se pudo levantar el hilo de la consola");
			finalizar();
		}

	}

	signal(SIGUSR1, signal_handler);
	// CREO SOCKET PARA ESCUCHAR CONEXIONES ENTRANTES

	listener = crear_socket();

	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		log_error(log_planif, "Error en el setsockopt");
		exit(1);
	}

	bindear_socket(listener, IP_PLANIFICADOR, PUERTO_PLANIFICADOR, log_planif);


	// LO PONGO A ESCUCHAR LAS CONEXIONES NUEVAS Y PETICIONES DE LAS POSTERIORMENTE EXISTENTES
	if (listen(listener, 10) == -1) {
		log_error(log_planif, "No se pudo poner el listener a escuchar");
		exit(1);
	}
	// añadir listener al conjunto maestro
	FD_SET(listener, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste

	//AGREGO EL SOCKET DEL COORDINADOR AL SET MASTER PARA ESCUCHAR SI ME MANDA ALGÚN MENSAJE

	FD_SET(SOCKET_COORDINADOR, &master);

	//CICLO PRINCIPAL DE EJECUCION


	for(;;) {
		read_fds = master;


		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error(log_planif, "Fallo en el select! Re raro.");
			exit(1);

		}else{

			// explorar conexiones existentes en busca de datos que leer
			for(socket = 0; socket <= fdmax; socket++) {
				if (FD_ISSET(socket, &read_fds)) { // ¡¡tenemos datos!!
					if (socket == listener) {
						// ATIENDO A LOS NUEVOS CLIENTES Y LES DOY LA BIENVENIDA, SOLO SI SON GENTE DE BIEN (?

						addrlen = sizeof(remoteaddr);
						if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen)) == -1) {
							log_error(log_planif, "ERROR: no se pudo aceptar la conexion del socket");
						} else {


							aniadir_cliente(&master, newfd, &fdmax);

							t_ready nuevo_esi = {
									.ID = proximo_id,
									.socket = newfd,
									.tiempo_espera = 0.0,
									.ultima_estimacion = ESTIMACION_INICIAL,
									.ultima_rafaga_real = 0.0,
									.estimacion_actual = 0.0
							};


							proximo_id ++;

							aniadir_a_colas_de_asignaciones(nuevo_esi);

							aniadir_a_listos(nuevo_esi);
						}

					} else {
						// GESTIONO PETICIONES DE LOS CLIENTES CONOCIDOS
						protocolo_cliente = recibir_protocolo(socket);

						atender_protocolo(protocolo_cliente, socket);

					}
				}
			}

		}

	}
}
