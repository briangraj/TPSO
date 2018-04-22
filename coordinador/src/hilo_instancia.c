/*
 * hilo_instancia.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_instancia.h"

void crear_hilo_instancia(int socket_instancia){
	crear_hilo(atender_instancia, (void*) socket_instancia);
}

void* atender_instancia(void* socket_instancia){

}
