/*
 * t_mensaje.c
 *
 *  Created on: 4 jun. 2018
 *      Author: utnso
 */

#include "t_mensaje.h"

int enviar_mensaje(t_mensaje mensaje, int socket){
	return enviar_paquete(mensaje.header, socket, mensaje.tam_payload, mensaje.payload);
}
