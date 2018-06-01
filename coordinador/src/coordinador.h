

#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conexiones/protocolos.h>
#include <conexiones/serializacion.h>
#include <conexiones/sockets.h>
#include <conexiones/threads.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include "hilo_planificador.h"
#include "hilo_esi.h"
#include <conexiones/estructuras_coord.h>
#include "hilo_instancia.h"

int CANTIDAD_ENTRADAS_TOTALES;
int TAMANIO_ENTRADA;
char* IP_COORD;
int PUERTO_COORD;
t_log* LOG_COORD;

t_list* INSTANCIAS;//t_instancia
int SOCKET_PLANIF;
pthread_mutex_t SEM_SOCKET_PLANIF;

// Funciones
void leer_config();
void aniadir_cliente(fd_set* master, int cliente, int* fdmax);
void atender_handshake(int socket_cliente);
void atender_protocolo(int protocolo, int socket_cliente);
void desconectar_cliente(int cliente);

#endif /* COORDINADOR_H_ */
