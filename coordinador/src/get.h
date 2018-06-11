/*
 * get.h
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#ifndef GET_H_
#define GET_H_

#include "coordinador.h"
#include "conexiones/estructuras_coord.h"
#include "distribucion.h"
#include "t_mensaje.h"
#include "t_instancia.h"
#include "t_solicitud.h"

t_solicitud* crear_get			   		      (int socket, int id);
int 		 realizar_get		   			  (t_solicitud* solicitud);
int 		 validar_existencia_clave		  (t_solicitud* solicitud);
t_mensaje 	 serializar_get_a_instancia		  (char* clave);
t_mensaje 	 serializar_get_a_planif		  (t_solicitud* solicitud);
void 		 setear_respuesta_a_esi			  (t_solicitud* solicitud, int resultado_planif);
bool         existe_clave_en_instancia_activa (t_solicitud* solicitud);

#endif /* GET_H_ */
