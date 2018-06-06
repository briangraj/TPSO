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

int  enviar_mensaje(t_mensaje mensaje, int socket);

#endif /* T_MENSAJE_H_ */
