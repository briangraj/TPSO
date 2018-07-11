#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conexiones/protocolos.h>
#include <conexiones/serializacion.h>
#include <conexiones/sockets.h>
#include <conexiones/threads.h>
#include <conexiones/estructuras_coord.h>
#include <conexiones/strings.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include "hilo_esi.h"
#include "hilo_instancia.h"
#include "t_mensaje.h"
#include "t_instancia.h"
#include "consola.h"

#define PATH_CONFIG "/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/configs/coordinador.cfg"

char* IP_COORD;
int PUERTO_COORD;
char* ALGORITMO_DISTRIBUCION;
int CANTIDAD_ENTRADAS_TOTALES;
int TAMANIO_ENTRADA;
int RETARDO;
int LISTENER;

t_log* LOG_COORD;

t_list* INSTANCIAS;//t_instancia
int SOCKET_PLANIF;
bool PLANIF_CONECTADO;
int SOCKET_CONSOLA;

void 			setup_coord						();
void 			setup_listener					();
void 			setup_conexion_con_planif		(int socket);
t_instancia*	setup_conexion_con_instancia	(int socket);
void			setup_conexion_con_consola		(int socket_cliente);
void			leer_config						();
void			bindear_socket_server			();
bool			hay_instancias_conectadas		();
void			atender_handshake				(int socket_cliente);
void			desconectar_cliente				(int cliente);
bool 			se_puede_atender_esi			();
void			desconectar_planif				();
void 			atender_conexion_esi			(int socket_cliente);
void 			atender_conexion_planif			(int socket_cliente);
void 			atender_conexion_instancia		(int socket_cliente);
void			atender_conexion_consola		(int socket_cliente);

#endif /* COORDINADOR_H_ */
