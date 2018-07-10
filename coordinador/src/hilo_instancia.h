/*
 * hilo_instancia.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef HILO_INSTANCIA_H_
#define HILO_INSTANCIA_H_

#include "coordinador.h"
#include "t_instancia.h"
#include "get.h"
#include "set.h"
#include "store.h"
#include "compactacion.h"

void		crear_hilo_instancia		(t_instancia* instancia);
void*		atender_instancia			(void* socket_instancia);
t_mensaje	serializar_config_instancia	();
int			enviar_config_instancia		(t_instancia* instancia);
int			enviar_solicitud			(t_solicitud* pedido, int socket);
void		evaluar_resultado_instr		(t_solicitud* solicitud, t_instancia* instancia);

#endif /* HILO_INSTANCIA_H_ */
