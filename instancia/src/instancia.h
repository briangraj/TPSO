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
#include <conexiones/sockets.h>
#include <conexiones/serializacion.h>
#include <conexiones/protocolos.h>


#define PATH_CONFIG "/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/configs/instancia.cfg"

//t_config* archivo_config;// = config_create(PATH_CONFIG);
char* IP_COORDINADOR;
int PUERTO_COORDINADOR;

t_log* log_data_node = log_create("DataNode.log", "DataNode", 1, LOG_LEVEL_TRACE);

int socket_coordinador;

void leer_config();
void conectar_con_coordinador();
void escuchar_coordinador();

#endif /* SRC_INSTANCIA_H_ */
