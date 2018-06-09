/*
 * archivos.h
 *
 *  Created on: 9 jun. 2018
 *      Author: utnso
 */

#ifndef CONEXIONES_ARCHIVOS_H_
#define CONEXIONES_ARCHIVOS_H_

#include <commons/log.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

void crear_si_no_existe(char* dir, t_log* logger);

#endif /* CONEXIONES_ARCHIVOS_H_ */
