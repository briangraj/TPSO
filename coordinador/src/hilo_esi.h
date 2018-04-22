/*
 * hilo_esi.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef HILO_ESI_H_
#define HILO_ESI_H_

typedef enum{
	GET = 0,
	SET,
	STORE,
};

typedef struct solicitud {
	int instruccion;
	char* clave;
} t_solicitud;


void crear_hilo_esi(int socket_cliente);
void* atender_esi(void* socket_esi);

t_solicitud recibir_solicitud_esi(int socket);


#endif /* HILO_ESI_H_ */
