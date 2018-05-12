/*
 * instancia.h
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#ifndef SRC_INSTANCIA_H_
#define SRC_INSTANCIA_H_

#include <dirent.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <conexiones/sockets.h>
#include <conexiones/serializacion.h>
#include <conexiones/strings.h>
#include <conexiones/protocolos.h>

#define PATH_CONFIG "/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/configs/instancia.cfg"

typedef struct {
	int nro_entrada;
	char* clave;
	int tamanio_clave;
	void* valor;//se deberia levantar con mmap
}t_entrada; //TODO mejor nombre?

char* PUNTO_MONTAGE;
char* IP_COORDINADOR;
int PUERTO_COORDINADOR;
int CANTIDAD_ENTRADAS;
int TAMANIO_ENTRADA;
int MI_ID;
int socket_coordinador;

void (*almacenar_clave)(char*, char*);

//para reemplazo circular
t_entrada* entrada_a_reemplazar;

t_log* log;
t_list* tabla_de_entradas;

void inicializar();
void leer_config();
void conectar_con_coordinador();
void configuracion_entradas();
void escuchar_coordinador();
void leer_protocolo(int protocolo);
void configuracion_entradas();
void crear_tabla_de_entradas();
t_entrada* levantar_entrada(char* nombre);
void recibir_set();
void circular(char* clave, char* valor);

#endif /* SRC_INSTANCIA_H_ */
