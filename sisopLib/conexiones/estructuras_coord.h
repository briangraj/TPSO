/*
 * estructuras_coord.h
 *
 *  Created on: 24 abr. 2018
 *      Author: utnso
 */

#ifndef CONEXIONES_ESTRUCTURAS_COORD_H_
#define CONEXIONES_ESTRUCTURAS_COORD_H_

#include <semaphore.h>
#include <stdbool.h>
#include <commons/collections/queue.h>
#include <unistd.h>

typedef struct solicitud {
	int instruccion;
	char* clave;
	char* valor;
	sem_t solicitud_finalizada;
	int resultado_instancia;
	int respuesta_a_esi;
	int id_esi;
	int socket_esi;
} t_solicitud;

typedef struct instancia {
	int id;
	int socket_instancia;
	pthread_t id_hilo;
	sem_t solicitud_lista;
	bool esta_activa;
	t_queue* solicitudes;
	t_list* claves;//list<char*>
	t_list* claves_a_crear;
	t_list* claves_a_borrar;
	int entradas_disponibles;
} t_instancia;

typedef struct { // esto es lo que nos tienen que mandar cuando reciben el protocolo STATUS :D (y en este orden)
	int tamanio_mensaje; // Puede ser quel el mensaje sea el valor de clave, peeero si la clave no esta, manden el mensaje "CLAVE SIN VALOR"
	char* mensaje; // AGREGENLE EL \0 Y CONTEMPLENLO EN EL TAMANIO
	int id_instancia_actual; // La instancia donde esta la clave que les mandamos, si no esta en ninguna instancia manden -1
	int id_instancia_posible; // Tienen que mirar el enunciado en el comando status.
}t_info_status;

#endif /* CONEXIONES_ESTRUCTURAS_COORD_H_ */
