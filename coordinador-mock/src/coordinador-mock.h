

#ifndef COORDINADOR_MOCK_H_
#define COORDINADOR_MOCK_H_

#include <conexiones/protocolos.h>
#include <conexiones/serializacion.h>
#include <conexiones/sockets.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

char* MI_IP = "127.0.0.1";
int MI_PUERTO = 5051;

int SOCKET_PLANIFICADOR;

t_log* log;
t_list* lista_esis;

typedef struct {
	int socket;
	int id;
}t_esi;

fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
fd_set master;   // conjunto maestro de descriptores de fichero		//Por comodidad lo pongo aca

// Funciones
void 				aniadir_cliente					(fd_set* master, int cliente, int* fdmax);
void 				atender_handshake				(int socket_cliente);
void 				atender_protocolo				(int protocolo, int socket_cliente);
void 				desconectar_cliente				(int cliente);
void 				atender_set						(int socket, int id_esi);
void 				atender_store					(int socket, int id_esi);
void 				atender_get						(int socket, int id_esi);
int 				elegir_opcion					();
void 				mostrar_menu					();
void 				armar_nuevo_esi					(int socket);
int 				obtener_id_desde_socket			(int socket);
int 				no_main							(void);

#endif /* COORDINADOR_MOCK_H_ */
