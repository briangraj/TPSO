/*
 * consola.h
 *
 *  Created on: 10 jul. 2018
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "coordinador.h"

typedef struct {
	int tamanio_clave;
	char* clave;
} t_status;

void			crear_hilo_consola				();
void*			atender_consola					(void* _);
t_status		recibir_status					();
t_solicitud*	crear_status					(t_status status);
int				enviar_status					(t_info_status info_status);
t_info_status	armar_status					(t_status status);
t_info_status	enviar_status_a_instancia		(t_instancia* instancia, t_solicitud* solicitud);
t_info_status	info_status_clave_existente		(t_solicitud* solicitud, t_instancia* instancia);
t_info_status	info_status_clave_inaccesible	();
t_info_status	info_status_clave_a_crear		(t_instancia* instancia);
t_info_status	info_status_clave_inexistente	();

#endif /* CONSOLA_H_ */
