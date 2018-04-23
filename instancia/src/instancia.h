/*
 * instancia.h
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#ifndef SRC_INSTANCIA_H_
#define SRC_INSTANCIA_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <conexiones/sockets.h>
#include <conexiones/serializacion.h>
#include <conexiones/protocolos.h>

#define PATH_CONFIG "/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/configs/instancia.cfg"

typedef struct {
	int nro_entrada;
	char clave[40];
	int tamanio_clave;
}t_entrada;

char* IP_COORDINADOR;
int PUERTO_COORDINADOR;
int CANTIDAD_ENTRADAS;
int TAMANIO_ENTRADA;
int socket_coordinador;

t_log* log;
t_list* tabla_de_entradas;

void leer_config();
void conectar_con_coordinador();
void configuracion_entradas();
void escuchar_coordinador();
void leer_protocolo(int protocolo);
void configuracion_entradas();
void crear_tabla_de_entradas();

#endif /* SRC_INSTANCIA_H_ */
