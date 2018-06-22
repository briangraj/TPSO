/*
 * strings.c
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#include "strings.h"

char* armar_path(char* path_izq, char* path_der){
	char* path_aux = string_new();
	string_append(&path_aux, path_izq);
	string_append(&path_aux, "/");
	string_append(&path_aux, path_der);

	return path_aux;
}

char* leer_string(t_config* config, char* clave){
	char* aux = string_new();
	string_append(&aux, config_get_string_value(config, clave));
	return aux;
}

bool string_equals(char* un_string, char* otro_string){
	return !strcmp(un_string, otro_string);
}

int string_size(char* string){
	return strlen(string) + 1;
}
