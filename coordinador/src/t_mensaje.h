/*
 * t_mensaje.h
 *
 *  Created on: 4 jun. 2018
 *      Author: utnso
 */

#ifndef T_MENSAJE_H_
#define T_MENSAJE_H_

#include "conexiones/serializacion.h"

typedef struct mensaje {
	int header;
	int tam_payload;
	void* payload;
} t_mensaje;

t_mensaje	crear_mensaje		(int header, int tam_payload);
int			enviar_mensaje		(t_mensaje mensaje, int socket);
int 		enviar_mensaje_v2	(int socket, t_mensaje (*serializer)(void*), void* target);
void 		destruir_mensaje	(t_mensaje mensaje);

#endif /* T_MENSAJE_H_ */
