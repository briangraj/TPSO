/*
 * instancia.h
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#ifndef SRC_INSTANCIA_H_
#define SRC_INSTANCIA_H_

#include <dirent.h>
#include <sys/mman.h>
#include <commons/bitarray.h>
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
	int tamanio_bytes_clave;
	int tamanio_entradas_clave;
	//void* valor;//se deberia levantar con mmap
}t_entrada; //TODO mejor nombre?

char* PUNTO_MONTAGE;
char* IP_COORDINADOR;
char* ALGORITMO_REEMPLAZO;
int PUERTO_COORDINADOR;
int CANTIDAD_ENTRADAS_TOTALES;
int TAMANIO_ENTRADA;
int MI_ID;
int socket_coordinador;
void* memoria;

t_entrada* (*algoritmo_reemplazo)(char*, char*);

//para reemplazo circular
int entrada_a_reemplazar;

t_log* log_instancia;
t_list* tabla_de_entradas;
t_bitarray* bitarray_entradas;

void inicializar();
void leer_config();
void setup_algoritmo_reemplazo();
void conectar_con_coordinador();
void configuracion_entradas();
void escuchar_coordinador();
void leer_protocolo(int protocolo);
void configuracion_entradas();
void crear_tabla_de_entradas();
t_entrada* levantar_entrada(char* nombre);
int tamanio_entrada(char* nombre);
void levantar_entrada_a_memoria(t_entrada* entrada);
void copiar_a_memoria(t_entrada* entrada, void* valor);
int abrir_entrada(char* nombre);
struct stat crear_stat(int fd);
void* mi_mmap(int fd, struct stat stat);
int entradas_ocupadas(int tamanio);
int entrada_para(int cant_entradas);
int entradas_libres_desde(int nro_entrada, int entradas_necesarias);
void setear_bitarray(t_entrada* entrada);
void atender_set();
int modificar_entrada(char* clave, char* valor);
void aumentar_tamanio_entrada(t_entrada* entrada, char* valor);
void actualizar_tamanio_entrada(t_entrada* entrada, char* valor);
void liberar_entradas_desde(int desde_entrada, int cantidad);
void atender_store();
void persistir(void* entrada_void);
t_entrada* reemplazo_circular(char* clave, char* valor);
t_entrada* reemplazo_bsu(char* clave, char* valor);
bool es_entrada_atomica(void* entrada_void);
bool bsu_entrada(void* entrada1, void* entrada2);
t_entrada* buscar_entrada(char* clave);
int entradas_disponibles();

#endif /* SRC_INSTANCIA_H_ */
