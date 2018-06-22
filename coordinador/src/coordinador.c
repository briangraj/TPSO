#include "coordinador.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "conexiones/threads.h"

int main(void) {
	setup_coord();
	setup_listener();

	struct sockaddr_in remoteaddr; // direccion del cliente
	socklen_t addrlen;

	while(true){

		addrlen = sizeof(remoteaddr);

		int socket_cliente = accept(LISTENER, (struct sockaddr *) &remoteaddr, &addrlen);

		if (socket_cliente == -1){
			log_error(LOG_COORD, "No se pudo aceptar la conexion del socket %d", socket_cliente);
			continue;
		}

		log_info(LOG_COORD, "Se acepto a un cliente en el socket %d", socket_cliente);

		if(informar_conexion_exitosa_a(socket_cliente) < 0){
			log_error(LOG_COORD,
					"No se pudo informar la conexion exitosa al cliente %d, se lo desconectara",
					socket_cliente);

			desconectar_cliente(socket_cliente);
			continue;
		}

		log_info(LOG_COORD,
				"Se informo al cliente %d de la conexion exitosa. Inicia el handshake",
				socket_cliente);

		atender_handshake(socket_cliente);

	}

}

void setup_coord(){
	LOG_COORD = log_create("Coordinador.log", "Coordinador", 1, LOG_LEVEL_TRACE);

	leer_config();

	INSTANCIAS = list_create();//t_instancia

	PLANIF_CONECTADO = false;
}

void leer_config(){
	t_config* config = config_create(PATH_CONFIG);

	IP_COORD = leer_string(config, "MI_IP");
	PUERTO_COORD = config_get_int_value(config, "MI_PUERTO");
	ALGORITMO_DISTRIBUCION = leer_string(config, "ALGORITMO_DISTRIBUCION");
	CANTIDAD_ENTRADAS_TOTALES = config_get_int_value(config, "CANTIDAD_ENTRADAS");
	TAMANIO_ENTRADA = config_get_int_value(config, "TAMANIO_ENTRADA");
	RETARDO = config_get_int_value(config, "RETARDO");

	config_destroy(config);
}

void bindear_socket_server(){
	int yes = 1;

	if (setsockopt(LISTENER, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1){
		log_error(LOG_COORD, "Error en el setsockopt");
		exit(EXIT_FAILURE);
	}

	bindear_socket(LISTENER, IP_COORD, PUERTO_COORD, LOG_COORD);
}

bool hay_instancias_conectadas(){
	return list_any_satisfy(INSTANCIAS, (bool (*)(void*)) esta_activa);
}

void atender_handshake(int socket_cliente){
	int remitente = recibir_handshake(socket_cliente);

	switch(remitente){

	case ESI:

		if(!se_puede_atender_esi()){
			desconectar_cliente(socket_cliente);
			break;
		}

		log_info(LOG_COORD, "Se recibio una conexion con un esi en el socket %d", socket_cliente);

		if(informar_conexion_exitosa_a(socket_cliente) < 0){
			log_error(LOG_COORD, "No se pudo completar el handshake con el esi en el socket %d, se lo desconectara", socket_cliente);
			desconectar_cliente(socket_cliente);
			break;
		}

		log_debug(LOG_COORD, "Se completo el handshake con el esi en el socket %d", socket_cliente);

		crear_hilo_esi(socket_cliente);

	break;

	case PLANIFICADOR:

		if(PLANIF_CONECTADO){
			log_error(LOG_COORD, "Ya hay un planificador conectado, se desconectara al cliente del socket %d", socket_cliente);
			desconectar_cliente(socket_cliente);
			break;
		}

		log_info(LOG_COORD, "Se recibio una conexion con el planificador en el socket %d", socket_cliente);

		if(informar_conexion_exitosa_a(socket_cliente) < 0){
			log_error(LOG_COORD, "No se pudo completar el handshake con el planificador en el socket %d, se lo desconectara", socket_cliente);
			desconectar_cliente(socket_cliente);
			break;
		}

		log_debug(LOG_COORD, "Se completo el handshake con el planificador en el socket %d", socket_cliente);

		setup_conexion_con_planif(socket_cliente);

	break;

	case INSTANCIA: {
		log_info(LOG_COORD, "Se recibio una conexion con una instancia en el socket %d", socket_cliente);

		if(informar_conexion_exitosa_a(socket_cliente) < 0){
			log_error(LOG_COORD, "No se pudo iniciar el handshake con la instancia en el socket %d, se la desconectara", socket_cliente);
			desconectar_cliente(socket_cliente);
			break;
		}

		t_instancia* instancia = setup_conexion_con_instancia(socket_cliente);

		if(instancia->claves_a_borrar.length() > 0)
			borrar_claves_a_instancia()

		if(instancia == NULL){
			log_error(LOG_COORD,
					"No se pudo completar el handshake con la instancia %d en el socket %d, se la desconectara",
					instancia->id,
					socket_cliente);

			desconectar_cliente(socket_cliente);
			break;
		}

		log_debug(LOG_COORD,
				"Se completo el handshake con la instancia %d en el socket %d",
				instancia->id,
				socket_cliente);

		crear_hilo_instancia(instancia);
	}

	break;

	default:
//		errores
	;
	}
}

t_instancia* setup_conexion_con_instancia(int socket){
	int id = recibir_id(socket);

	if(id <= 0){
		log_error(LOG_COORD,
				"No se pudo recibir el id de la instancia conectada en el socket %d",
				socket);
		return NULL;
	}

	log_trace(LOG_COORD, "Se recibio el id %d", id);

	t_instancia* instancia = instancia_de_id(id);

	if(instancia == NULL){
		instancia = crear_instancia(id, socket);

		if(enviar_config_instancia(socket) == -1){
			log_error(LOG_COORD, "No se pudo enviar la config a la instancia %d", instancia->id);
			destruir_instancia(instancia);
			return NULL;
		}

		if(recibir_claves(instancia) == -1){
			log_error(LOG_COORD, "No se pudieron recibir las claves de la instancia %d", instancia->id);
			destruir_instancia(instancia);
			return NULL;
		}

		log_trace(LOG_COORD, "Se creo la instancia de id %d con exito", id);

		list_add(INSTANCIAS, (void*) instancia);

		log_trace(LOG_COORD, "Se agrego a la instancia de id %d al sistema");
	} else {
		//
	}

	return instancia;
}

void setup_conexion_con_planif(int socket){
	PLANIF_CONECTADO = true;

	SOCKET_PLANIF = socket;
}

void desconectar_cliente(int cliente){
	close(cliente);

	log_trace(LOG_COORD, "Se desconecto al cliente %d", cliente);
}

bool se_puede_atender_esi(){

	if(!PLANIF_CONECTADO){
		log_error(LOG_COORD, "El planificador no se encuentra conectado, se desconectara al esi en el socket");
		return false;
	}

	if(!hay_instancias_conectadas()){
		log_error(LOG_COORD, "No hay instancias conectadas, se desconectara al esi en el socket");
		return false;
	}

	return true;
}

void desconectar_planif(){
	PLANIF_CONECTADO = false;

	desconectar_cliente(SOCKET_PLANIF);
}

void setup_listener(){

	LISTENER = crear_socket();// creo socket para escuchar conexiones entrantes

	bindear_socket_server();

	log_trace(LOG_COORD,
			"Se bindeo el socket %d en la ip %s y puerto %d",
			LISTENER,
			IP_COORD,
			PUERTO_COORD);

	// lo pongo a escuchar conexiones nuevas
	if (listen(LISTENER, 10) == -1) {
		log_error(LOG_COORD, "No se pudo poner el socket a escuchar");
		exit(EXIT_FAILURE);
	}

	log_info(LOG_COORD, "El socket %d esta escuchando conexiones...", LISTENER);

}
