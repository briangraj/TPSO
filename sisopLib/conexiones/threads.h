/*
 * threads.h
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#ifndef CONEXIONES_THREADS_H_
#define CONEXIONES_THREADS_H_


#include <pthread.h>


//FUNCIONES
int 			crear_hilo			(void* funcion_del_hilo(void*), void* parametro_hilo);


#endif /* CONEXIONES_THREADS_H_ */
