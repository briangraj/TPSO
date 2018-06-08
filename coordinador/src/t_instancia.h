/*
 * t_instancia.h
 *
 *  Created on: 4 jun. 2018
 *      Author: utnso
 */

#ifndef T_INSTANCIA_H_
#define T_INSTANCIA_H_

#include "coordinador.h"
#include "conexiones/estructuras_coord.h"

void*        crear_instancia	 (int id, int socket);
void         agregar_clave		 (t_instancia* instancia, char* clave);
bool         esta_activa 		 (t_instancia* instancia);
void         agregar_pedido		 (t_instancia* instancia, t_solicitud* solicitud);
t_solicitud* sacar_pedido        (t_instancia* instancia);
int	         recibir_claves		 (t_instancia* instancia);
void         borrar_clave        (t_solicitud* solicitud, t_instancia* instancia);
t_instancia* instancia_con_clave (t_solicitud* solicitud);

#endif /* T_INSTANCIA_H_ */
