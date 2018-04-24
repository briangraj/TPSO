#include "funciones_planificador.h"

void iniciar_planificador(int loggear){
	log_planif = log_create("Planificador.log", "Planificador", loggear, LOG_LEVEL_TRACE);

	leer_archivo_config();

	if((SOCKET_COORDINADOR = conectarse_a_coordinador(SOCKET_COORDINADOR)) == -1)
		exit(1);

	cola_de_listos = list_create();
	cola_finalizados = list_create();
	colas_de_bloqueados = list_create();

}

void leer_archivo_config(){
	archivo_config = config_create("../../configs/planificador.cfg"); // Ante problema con eclipse/consola, cambiarla por ruta absoluta

//	archivo_config = config_create("/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/configs/planificador.cfg");

	IP_PLANIFICADOR = config_get_string_value(archivo_config, "IP_PLANIFICADOR");
	PUERTO_PLANIFICADOR = config_get_int_value(archivo_config, "PUERTO_PLANIFICADOR");
	IP_COORDINADOR = config_get_string_value(archivo_config, "IP_COORDINADOR");
	PUERTO_COORDINADOR = config_get_int_value(archivo_config, "PUERTO_COORDINADOR");
	ESTIMACION_INICIAL = config_get_int_value(archivo_config, "ESTIMACION_INICIAL");
	ALFA_PLANIFICACION = config_get_int_value(archivo_config, "ALFA_PLANIFICACION");

	char* algoritmo = config_get_string_value(archivo_config, "ALGORITMO_PLANIFICACION");

	if(!strcmp(algoritmo, "SJF_SD"))
		ALGORITMO_PLANIFICACION = SJF_SD;

	else if(!strcmp(algoritmo, "SJF_CD"))
		ALGORITMO_PLANIFICACION = SJF_CD;

	else if(!strcmp(algoritmo, "HRRN"))
		ALGORITMO_PLANIFICACION = HRRN;

	else {
		log_error(log_planif, "No se reconoce el algoritmo de planificacion");
		log_destroy(log_planif);
		config_destroy(archivo_config);
		exit(1);
	}
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

//		// SOLO PARA PROBAR QUE ESTE ANDANDO EL ESI
//
//		ejecutar_mock(socket_cliente);

	}


}

void atender_protocolo(int protocolo, int socket_cliente){
	log_debug(log_planif, "Llegamos hasta atender protocolo!!! Recibi el protocolo %d, por el socket %d", protocolo, socket_cliente);

//	// SOLO PARA PROBAR QUE ANDE EL ESI
//	if(protocolo == EJECUCION_EXITOSA)
//		ejecutar_mock(socket_cliente);
}

void desconectar_cliente(int cliente){
	//error o conexión cerrada por el cliente
	log_trace(log_planif, "Se desconecto el cliente %d", cliente);

	close(cliente); // bye!
	FD_CLR(cliente, &master);
}

int conectarse_a_coordinador(){

	int socket;

	if((socket = conectarse_a_server("Planificador", PLANIFICADOR, "Coordinador", IP_COORDINADOR, PUERTO_COORDINADOR, log_planif)) == -1){
		log_destroy(log_planif);
		config_destroy(archivo_config);
		return -1;
	}

	return socket;
}

void aniadir_a_listos(t_ready esi){
	t_ready* esi_ready = duplicar_esi_ready(esi);

	planificar(esi_ready);
	//Cuando finaliza esta funcion.... que pasa con esi_ready? se pierde? porque se quedaría basura en la lista
}

t_ready* duplicar_esi_ready(t_ready esi){
	t_ready* resultado = (t_ready*) malloc(sizeof(t_ready));

	resultado->ID = esi.ID;
	resultado->estimacion_actual = esi.estimacion_actual;
	resultado->socket = esi.socket;
	resultado->tiempo_espera = esi.tiempo_espera;
	resultado->ultima_estimacion = esi.ultima_estimacion;
	resultado->ultima_rafaga_real = esi.ultima_rafaga_real;

	return resultado;
}

void planificar(t_ready* esi_ready){

	if(cola_de_listos->elements_count == 0){

		list_add(cola_de_listos, esi_ready);
		mandar_a_ejecutar();

	} else
		insertar_ordenado(esi_ready);

}

void mandar_a_ejecutar(){//Enviar orden de ejecucion al primer ESI de la cola de listos
	//TODO setear en 0 el tiempo de espera del que va a ejecutar y sumar 1 al resto
}

void insertar_ordenado(t_ready* esi_ready){

	esi_ready->estimacion_actual = estimacion(esi_ready);

	switch(ALGORITMO_PLANIFICACION){
		case SJF_CD:

			comparar_desde(0, comparar_sjf, esi_ready);

			break;

		case SJF_SD:

			comparar_desde(1, comparar_sjf, esi_ready);

			break;

		case HRRN:

			comparar_desde(1, comparar_hrrn, esi_ready);

			break;
	}
}

float estimacion(t_ready* esi_ready){
	return 0.0;//FIXME
}

void comparar_desde(int indice_comparacion, bool (*funcion_comparacion)(void*, void*), t_ready* esi_ready){

	list_add(cola_de_listos, esi_ready);

	if(indice_comparacion == 0)
		list_sort(cola_de_listos, funcion_comparacion);

	else {

		t_ready* primer_esi = list_remove(cola_de_listos, 0);
		list_sort(cola_de_listos, funcion_comparacion);
		list_add_in_index(cola_de_listos, 0, primer_esi);//TODO: REVISAR SI ESTO HACE LO QUE QUEREMOS

	}

}

bool comparar_sjf(void* un_esi, void* otro_esi){
	t_ready* primer_esi = (t_ready*) un_esi;
	t_ready* segundo_esi = (t_ready*) otro_esi;

	return primer_esi->estimacion_actual < segundo_esi->estimacion_actual;
}

bool comparar_hrrn(void* un_esi, void* otro_esi){
	t_ready* primer_esi = (t_ready*) un_esi;
	t_ready* segundo_esi = (t_ready*) otro_esi;

	return ratio(primer_esi) > ratio(segundo_esi);
}

float ratio(t_ready* esi){
	return (esi->tiempo_espera / esi->estimacion_actual) + 1;
}

void finalizar(){
	close(SOCKET_COORDINADOR);
	log_destroy(log_planif);
	config_destroy(archivo_config);

	list_destroy(cola_de_listos);
	list_destroy(cola_finalizados);
	list_destroy_and_destroy_elements(colas_de_bloqueados, clave_destroyer);

	exit(1);
}

void clave_destroyer(void* elemento){
	t_bloqueados_por_clave* bloqueados_por_clave = (t_bloqueados_por_clave*) elemento;
	free(bloqueados_por_clave->clave);
	list_destroy(bloqueados_por_clave->bloqueados);
}


// MOCKS

void ejecutar_mock(int socket_cliente){

	enviar_paquete(EJECUTAR_SENTENCIA, socket_cliente, 0 , NULL);

}










