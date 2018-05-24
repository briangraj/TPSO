/*
 * serializacion.h
 *
 *  Created on: 1/10/2017
 *      Author: utnso
 */

#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include "protocolos.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <commons/log.h>
#include <stdio.h>
#include <errno.h>
#include <commons/string.h>
#include "estructuras_coord.h"

//------------- ESTRUCTURAS -------------
typedef struct {
	int protocolo;
	int remitente;
} t_handshake;

typedef struct {
	int protocolo;
	void* payload;
} t_paquete;

typedef struct {
	int tamanio;
	char* ruta;
}t_ruta;

typedef struct{
	int tamanio_ip;
	char* ip;
	int puerto;
}t_ip_puerto;
//------------- CONSTANTES -------------

#define TAM_HEADER 4

#define TAM_PAQUETE_HANDSHAKE 4


//------------- FUNCIONES -------------

int 	enviar_paquete						(int protocolo, int sockfd, int tamanio_payload, void* payload);
int 	send_all							(int socket, void* mensaje, int tamanio_mensaje);
int 	recibir_protocolo					(int socket_fd);
int 	informar_conexion_exitosa_a			(int socket_cliente);
int 	recibir_informe						(int socket_servidor);
int 	realizar_handshake					(int id_proceso, int socket_servidor);
int 	recibir_handshake					(int socket_fd);
int 	informar_desconexion				(int socket_cliente);
void* 	serializar_ruta						(char* fs_path);
t_ruta	deserializar_ruta					(int socket_cliente);
int 	informar_resultado_de_operacion		(int socket, int resultado_operacion);
void 	serializar_string					(void* payload, char* string);
char* 	recibir_string		   				(int socket);
int 	informar_fallo_recv					(t_log* logger, char* string);
void* 	serializar_info_status				(t_info_status* info_status, int* tamanio_paquete);

#endif /* SERIALIZACION_H_ */
