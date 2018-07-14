/*
 * t_solicitud.h
 *
 *  Created on: 6 jun. 2018
 *      Author: utnso
 */

#ifndef T_SOLICITUD_H_
#define T_SOLICITUD_H_

#include "coordinador.h"

t_solicitud*	crear_solicitud						(int instruccion, int id, int socket);
void			destruir_solicitud					(t_solicitud* solicitud);
void 			liberar_solicitud					(t_solicitud* solicitud);
const char* 	solicitud_to_string					(t_solicitud* solicitud);
void 			set_resultado_instancia				(t_solicitud* solicitud, int resultado);
void			set_respuesta_a_esi					(t_solicitud* solicitud, int resultado);
void			setear_operacion_exitosa_instancia	(t_solicitud* solicitud);
void 			log_error_envio_planif				(t_solicitud* solicitud);
void 			log_error_resultado_planif			(t_solicitud* solicitud);
void 			log_error_comunicacion_instancia	(t_solicitud* solicitud);
int 			protocolo_planif					(t_solicitud* solicitud);
bool 			valor_ya_esta_en_memoria			(t_solicitud* solicitud);

#endif /* T_SOLICITUD_H_ */
