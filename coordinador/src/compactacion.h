/*
 * compactacion.h
 *
 *  Created on: 9 jul. 2018
 *      Author: utnso
 */

#ifndef COMPACTACION_H_
#define COMPACTACION_H_

#include <commons/collections/list.h>
#include <stdbool.h>
#include <conexiones/estructuras_coord.h>
#include <stdio.h>
#include <stdlib.h>
#include "t_mensaje.h"
#include "t_instancia.h"
#include "t_solicitud.h"

t_list* instancias_activas();
void compactar_instancias();
void compactar(t_instancia* instancia);
t_mensaje serializar_compactacion(t_solicitud* solicitud);

#endif /* COMPACTACION_H_ */
