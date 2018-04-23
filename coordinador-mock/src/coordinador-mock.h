

#ifndef COORDINADOR_MOCK_H_
#define COORDINADOR_MOCK_H_

#include <conexiones/protocolos.h>
#include <conexiones/serializacion.h>
#include <conexiones/sockets.h>
#include <commons/log.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

char* MI_IP = "127.0.0.1";
int MI_PUERTO = 5051;

t_log* log;

fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
fd_set master;   // conjunto maestro de descriptores de fichero		//Por comodidad lo pongo aca

// Funciones
void aniadir_cliente(fd_set* master, int cliente, int* fdmax);
void atender_handshake(int socket_cliente);
void atender_protocolo(int protocolo, int socket_cliente);
void desconectar_cliente(int cliente);
void atender_operacion(char* operacion, int socket);
int elegir_opcion();
void mostrar_menu();

int no_main(void);

#endif /* COORDINADOR_MOCK_H_ */
