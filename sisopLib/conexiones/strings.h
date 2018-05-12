/*
 * strings.h
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#ifndef CONEXIONES_STRINGS_H_
#define CONEXIONES_STRINGS_H_

#include <commons/config.h>
#include <commons/string.h>

char* armar_path(char* path_izq, char* path_der);
char* leer_string(t_config* config, char* clave);

#endif /* CONEXIONES_STRINGS_H_ */
