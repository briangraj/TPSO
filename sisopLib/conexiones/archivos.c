/*
 * archivos.c
 *
 *  Created on: 9 jun. 2018
 *      Author: utnso
 */
#include "archivos.h"

void crear_si_no_existe(char* dir, t_log* logger){ // FIXME si ya existen se rompe?...
	int resultado_creacion_dir = mkdir(dir, 0777);

	if(resultado_creacion_dir != 0 && errno != EEXIST){
		log_error(logger, "El directorio '%s' no pudo ser creado", dir);
		exit(EXIT_FAILURE);
	}else if(resultado_creacion_dir == 0){
		log_info(logger, "Fue creado el directorio '%s'", dir);
	}
}
