/*
 * threads.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "threads.h"


int crear_hilo(void* funcion_del_hilo(void*), void* parametro_hilo) {
		pthread_attr_t attr;
		pthread_t hilo;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		int resultado_creacion = pthread_create(&hilo, &attr, funcion_del_hilo, parametro_hilo);

		pthread_attr_destroy(&attr);

		return resultado_creacion;
}
