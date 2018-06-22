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
#include <string.h>
#include <stdbool.h>

char* armar_path(char* path_izq, char* path_der);
char* leer_string(t_config* config, char* clave);
bool  string_equals(char* un_string, char* otro_string);
int   string_size(char* string);

#endif /* CONEXIONES_STRINGS_H_ */
