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
#include <commons/collections/list.h>
#include <unistd.h>

typedef struct solicitud {
	int instruccion;
	char* clave;
	char* valor;
} t_solicitud;

typedef struct instancia {
	int id;
	int socket;
	sem_t sem;
	bool esta_conectado;
	t_list* pedidos;
} t_instancia;

#endif /* CONEXIONES_ESTRUCTURAS_COORD_H_ */
