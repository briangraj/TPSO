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
#include "t_solicitud.h"

void*			crear_instancia				(int id, int socket);
void			desconectar_instancia		(t_instancia* instancia);
void 			eliminar_instancia			(t_instancia* instancia);
void			destruir_instancia			(t_instancia* instancia);
void			destruir_instancias			();
void			agregar_clave				(t_instancia* instancia, char* clave);
bool			esta_activa					(t_instancia* instancia);
void			agregar_solicitud			(t_instancia* instancia, t_solicitud* solicitud);
t_solicitud*	sacar_solicitud				(t_instancia* instancia);
int				recibir_claves				(t_instancia* instancia);
void			borrar_clave				(t_solicitud* solicitud, t_instancia* instancia);
t_instancia*	instancia_con_clave			(t_solicitud* solicitud);
void			agregar_clave_a_crear		(t_instancia* instancia, char* clave);
void 			agregar_clave_a_borrar		(t_instancia* instancia, char* clave);
bool			contiene_clave				(t_list* claves, t_solicitud* solicitud);
t_instancia*	instancia_de_id				(int id);
int				reconectar_instancia		(t_instancia* instancia, int socket);
int 			conectar_instancia_nueva	(t_instancia* instancia);
int 			enviar_claves_a_borrar		(t_instancia* instancia);

#endif /* T_INSTANCIA_H_ */
