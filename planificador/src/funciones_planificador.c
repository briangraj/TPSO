#include "funciones_planificador.h"

void iniciar_planificador(){
	log_planif = log_create("Planificador.log", "Planificador", 1, LOG_LEVEL_TRACE);

	leer_archivo_config();

	conectarse_a_coordinador(SOCKET_COORDINADOR);
}

void leer_archivo_config(){
	//archivo_config = config_create("../../configs/planificador.cfg"); // Ante problema con eclipse/consola, cambiarla por ruta absoluta

	archivo_config = config_create("/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/configs/planificador.cfg");

	IP_PLANIFICADOR = config_get_string_value(archivo_config, "IP_PLANIFICADOR");
	PUERTO_PLANIFICADOR = config_get_int_value(archivo_config, "PUERTO_PLANIFICADOR");
	IP_COORDINADOR = config_get_string_value(archivo_config, "IP_COORDINADOR");
	PUERTO_COORDINADOR = config_get_int_value(archivo_config, "PUERTO_COORDINADOR");
}


void aniadir_cliente(fd_set* master, int cliente, int* fdmax){
	FD_SET(cliente, master);//añadir al conjunto maestro

	log_trace(log_planif, "Se conecto el cliente %d", cliente);

	if (cliente > *fdmax)//actualizar el máximo
		*fdmax = cliente;

	if(informar_conexion_exitosa_a(cliente) <= 0)
		log_error(log_planif, "ERROR: no se pudo informar al cliente %d", cliente);

	atender_handshake(cliente);
}

void atender_handshake(int socket_cliente){
	int remitente = recibir_handshake(socket_cliente);

	if(remitente != ESI){

		if(remitente < 0){ //Error en recv o me llego un protocolo que no era handshake

			log_trace(log_planif, "Error en el handshake");

		}else { //El cliente no es ESI ni un error en recv

			log_trace(log_planif, "Cliente desconocido detectado y rechazado");
			informar_desconexion(socket_cliente);	//No me interesa catchear el error del send

		}

		desconectar_cliente(socket_cliente);

	} else { //El cliente es un ESI

		log_trace(log_planif, "Nuevo ESI detectado y aceptado");
		informar_conexion_exitosa_a(socket_cliente);

	}
}

void atender_protocolo(int protocolo, int socket_cliente){
	log_trace(log_planif, "Llegamos hasta atender protocolo!!! Recibi el protocolo %d", protocolo);
}

void desconectar_cliente(int cliente){
	//error o conexión cerrada por el cliente
	log_trace(log_planif, "Se desconecto el cliente %d", cliente);

	close(cliente); // bye!
	FD_CLR(cliente, &master);
}

void conectarse_a_coordinador(int socket){
	if(conectarse_a_server("Planificador", PLANIFICADOR, "Coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, socket, log_planif) == -1){
		log_destroy(log_planif);
		config_destroy(archivo_config);
		exit(1);
	}
}
