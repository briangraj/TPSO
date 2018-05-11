/*
 * funciones_consola.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "funciones_consola.h"

void levantar_consola(void * param){

	if((SOCKET_COORDINADOR_CONSOLA = conectarse_a_coordinador(SOCKET_COORDINADOR_CONSOLA)) == -1){
		imprimir("No se pudo conectar la consola al coordinador");

		kill(PLANIFICADOR_PID, SIGUSR1);

		exit(1);
	}


	setear_comandos();

	  char * linea;

	  while(1) {
	    linea = readline("Consola > ");

	    linea = stripwhite(linea);

	    if(!strncmp(linea, "exit", 4)) {
	       free(linea);
	       kill(PLANIFICADOR_PID, SIGUSR1);
	       close(SOCKET_COORDINADOR_CONSOLA);
	       break;
	    }

	    if(linea){
			add_history(linea);
			ejecutar_linea(linea);
		}


	    free(linea);
	  }

}

void setear_comandos(){

	comandos[0].nombre = "pausar";
	comandos[0].funcion = com_pausar;
	comandos[0].descripcion = "El Planificador no le dará nuevas órdenes de ejecución a ningún ESI mientras se encuentre pausado";

	comandos[1].nombre = "continuar";
	comandos[1].funcion = com_continuar;
	comandos[1].descripcion = "Permite que el planificador de nuevas ordenes de ejecución a los procesos ESI";

	comandos[2].nombre = "bloquear";
	comandos[2].funcion = com_bloquear;
	comandos[2].descripcion = "Se bloqueará el proceso ESI hasta ser desbloqueado, especificado por dicho <ID> en la cola del recurso <clave>";

	comandos[3].nombre = "desbloquear";
	comandos[3].funcion = com_desbloquear;
	comandos[3].descripcion = "Se desbloqueara el proceso ESI con el ID especificado. Solo se desbloquearán ESIs que fueron bloqueados con la consola";

	comandos[4].nombre = "listar";
	comandos[4].funcion = com_listar;
	comandos[4].descripcion = "Lista los procesos encolados esperando al recurso";

	comandos[5].nombre = "kill";
	comandos[5].funcion = com_kill;
	comandos[5].descripcion = "Finaliza el proceso";

	comandos[6].nombre = "status";
	comandos[6].funcion = com_status;
	comandos[6].descripcion = "Información sobre las instancias del sistema";

	comandos[7].nombre = "deadlock";
	comandos[7].funcion = com_deadlock;
	comandos[7].descripcion = "Permitirá analizar los deadlocks que existan en el sistema y a que ESI están asociados";

	comandos[8].nombre = (char *)NULL;
	comandos[8].funcion = (Function *)NULL;
	comandos[8].descripcion = (char *)NULL;

}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char* stripwhite (char* string){
	register char *s, *t;

	for (s = string; whitespace (*s); s++)
		;
	if (*s == 0)
		return (s);

	t = s + strlen (s) - 1;
	while (t > s && whitespace (*t))
		t--;

	*++t = '\0';

	return s;
}


/* Execute a command line. */
int ejecutar_linea(char* linea){

  register int i;
  t_comando* comando;
  char *word;

  /* Isolate the command word. */
  i = 0;
  while (linea[i] && whitespace (linea[i]))
    i++;
  word = linea + i;

  while (linea[i] && !whitespace (linea[i]))
    i++;

  if (linea[i])
	  linea[i++] = '\0';

  comando = find_command (word);

  if (!comando)
    {
      printf("No se encontró el comando %s \n", word);
      return (-1);
    }

  /* Get argument to command, if any. */
  while (whitespace (linea[i]))
    i++;

  word = linea + i;

  /* Call the function. */
  return ((*(comando->funcion)) (word));
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
t_comando* find_command (char* nombre){

  register int i;

  for (i = 0; comandos[i].nombre; i++)
    if (strcmp (nombre, comandos[i].nombre) == 0)
      return (&comandos[i]);

  return ((t_comando *) NULL);
}

void imprimir(char* cadena){
	printf("%s\n", cadena);
	fflush(stdout);
}

//COMANDOS

int	com_pausar(char* parametro){

	pthread_mutex_lock(&semaforo_pausa);

	imprimir("El planificador esta en pausa, escriba continuar para reanudar su ejecucion.");

	return 0;
}

int	com_continuar(char* parametro){

	pthread_mutex_unlock(&semaforo_pausa);

	imprimir("El planificador reanudo su ejecucion.");

	return 0;
}

int	com_bloquear(char* parametro){

	char** parametros = controlar_y_obtener_parametros(parametro, 2);

	if(!parametros){
		imprimir("Cantidad incorrecta de parámetros para el comando 'bloquear'");
		imprimir("Los parámetros son: 'clave' y 'ID ESI");

		return 0;
	}

	char* clave = strdup(parametros[0]);
	int id_esi = atoi(parametros[1]);

	liberar_parametros(parametros, 2);

	bool coincide_el_id(void* elemento){
		t_ready* esi = (t_ready*) elemento;

		return esi->ID == id_esi;
	}

	pthread_mutex_lock(&semaforo_cola_listos);
	t_ready* info_ejecucion_esi = list_find(cola_de_listos, coincide_el_id);
	pthread_mutex_unlock(&semaforo_cola_listos);

	if(!info_ejecucion_esi){
		imprimir("El ESI que se pidio bloquear no está activo en el sistema");
		free(clave);
		return 0;
	}

	t_blocked* esi_bloqueado = (t_blocked*) malloc(sizeof(t_blocked));

	esi_bloqueado->info_ejecucion = duplicar_esi_ready(*info_ejecucion_esi);
	esi_bloqueado->bloqueado_por_ejecucion = false;
	esi_bloqueado->bloqueado_por_consola = true;

	t_bloqueados_por_clave* bloqueados_por_clave = encontrar_bloqueados_para_la_clave(clave);

	if(!bloqueados_por_clave){//Si no existe la cola de bloqueados, la creamos dejandola libre para que sea tomada por cualquiera (menos el bloqueado por consola)
		crear_entrada_bloqueados_del_recurso(0, clave);
		bloqueados_por_clave = encontrar_bloqueados_para_la_clave(clave);

	}else{
		//TODO: Si el ESI bloqueado por este comando ya tenía esta clave asignada, se la desalojamos dandosela al siguiente ESI en la cola de bloqueados
	}

	pthread_mutex_lock(&semaforo_cola_bloqueados);

	list_add(bloqueados_por_clave->bloqueados, esi_bloqueado);

	pthread_mutex_unlock(&semaforo_cola_bloqueados);

	//Ahora sacamos al ESI de la cola de listos

	pthread_mutex_lock(&semaforo_cola_listos);

	list_remove_and_destroy_by_condition(cola_de_listos, coincide_el_id, funcion_al_pedo);

	pthread_mutex_unlock(&semaforo_cola_listos);

	free(clave);

	imprimir("El ESI se bloqueo exitosamente");

	return 0;
}

int	com_desbloquear(char* parametro){

	char** parametros = controlar_y_obtener_parametros(parametro, 1);

	if(!parametros){
		imprimir("Cantidad incorrecta de parámetros para el comando 'desbloquear'");
		imprimir("El unico parametro del comando es la 'clave'");

		return 0;
	}

	char* clave = strdup(parametros[0]);

	liberar_parametros(parametros, 1);

	t_bloqueados_por_clave* bloqueados_por_clave = encontrar_bloqueados_para_la_clave(clave);

	if(!bloqueados_por_clave)
		imprimir("La clave ingresada no posee ESIs a la espera de su liberacion");

	else{

		bool esta_bloqueado_por_consola(void* elem){
			t_blocked* esi_block = (t_blocked*) elem;

			return esi_block->bloqueado_por_consola;
		}

		pthread_mutex_lock(&semaforo_cola_bloqueados);
		t_blocked* esi_bloqueado = list_find(bloqueados_por_clave->bloqueados, esta_bloqueado_por_consola);//Buscamos al primer bloqueado por consola
		pthread_mutex_unlock(&semaforo_cola_bloqueados);


		//Chequear que se haya encontrado algun ESI bloqueado por consola
		if(!esi_bloqueado){
			imprimir("No hay ningun ESI desloqueable por consola para la clave solicitada");
			free(clave);
			return 0;
		}


		int id_esi = esi_bloqueado->info_ejecucion->ID;
		char* informe = string_new();

		if(esi_bloqueado->bloqueado_por_ejecucion){

			esi_bloqueado->bloqueado_por_consola = false;

			informe = string_from_format("Se removio el flag 'bloqueado por consola' al ESI %d pero no se desbloqueo ya que esta bloqueado por ejecucion propia", id_esi);
			imprimir(informe);

		}else{

			aniadir_a_listos(*(esi_bloqueado->info_ejecucion));

			eliminar_de_bloqueados(esi_bloqueado->info_ejecucion);

			bool es_el_recurso(void* elem){
				t_bloqueados_por_clave* bloq = (t_bloqueados_por_clave*) elem;

				return string_equals_ignore_case(bloq->clave, clave);
			}

			pthread_mutex_lock(&semaforo_cola_bloqueados);

			if(list_is_empty(bloqueados_por_clave->bloqueados) && bloqueados_por_clave->id_proximo_esi == 0)
				list_remove_and_destroy_by_condition(colas_de_bloqueados, es_el_recurso, clave_destroyer);

			pthread_mutex_unlock(&semaforo_cola_bloqueados);

			informe = string_from_format("Se logro desbloquear al ESI %d para la clave solicitada", id_esi);
			imprimir(informe);

		}

		free(informe);
	}

	free(clave);

	return 0;
}

int	com_listar(char* parametro){

	char** parametros = controlar_y_obtener_parametros(parametro, 1);

	if(!parametros){
		imprimir("Cantidad incorrecta de parámetros para el comando 'listar'");
		imprimir("El unico parametro del comando es la 'clave'");

		return 0;
	}

	char* clave = strdup(parametros[0]);

	liberar_parametros(parametros, 1);

	bool es_la_clave(void* elem){
		t_bloqueados_por_clave* bloqueados_por_clave = (t_bloqueados_por_clave*) elem;

		return string_equals_ignore_case(bloqueados_por_clave->clave, clave);
	}

	if(!list_any_satisfy(colas_de_bloqueados, es_la_clave)){
		imprimir("No se encontro la clave solicitada");
		free(clave);
		return 0;
	}

	imprimir_cola_bloqueados(clave);

	free(clave);

	return 0;
}

int	com_kill(char* parametro){

	char** parametros = controlar_y_obtener_parametros(parametro, 1);

	if(!parametros){
		imprimir("Cantidad incorrecta de parámetros para el comando 'kill'");
		imprimir("El unico parametro del comando es el 'id del esi'");

		return 0;
	}

	int id_esi = atoi(parametros[0]);

	liberar_parametros(parametros, 1);

	t_ready* esi_buscado = buscar_en_bloqueados(id_esi);

	if(!esi_buscado)
		esi_buscado = buscar_en_ready(id_esi);

	if(!esi_buscado){
		imprimir("El ESI ingresado no se encuentra activo en el sistema");
	}

	mover_a_finalizados(esi_buscado, "Se envio el comando kill sobre el ESI");

	imprimir("Se finalizo el ESI solicitado");

	return 0;
}

int	com_status(char* parametro){
	char** parametros = controlar_y_obtener_parametros(parametro, 1);

	if(!parametros){
		imprimir("Cantidad incorrecta de parámetros para el comando 'status'");
		imprimir("El unico parametro del comando es la 'clave'");

		return 0;
	}

	char* clave = strdup(parametros[0]);

	liberar_parametros(parametros, 1);

	int tamanio_clave = strlen(clave) + 1;

	int tamanio_paquete = sizeof(int) + tamanio_clave;

	void* paquete = malloc(tamanio_paquete);

	memcpy(paquete, &tamanio_clave, sizeof(int));

	memcpy(paquete + sizeof(int), clave, tamanio_clave);

	if(enviar_paquete(STATUS, SOCKET_COORDINADOR, tamanio_paquete, paquete) < 0){
		free(parametro); // que es free(linea);

		imprimir("Se perdio la conexion con el coordinador");

		log_error(log_planif, "Se perdio la conexion con el coordinador");

		kill(PLANIFICADOR_PID, SIGUSR1);

		close(SOCKET_COORDINADOR_CONSOLA);

		exit(1);
	}

	t_info_status* info_status = recibir_info_status();

	if(!info_status){
		free(parametro); // que es free(linea);

		imprimir("Se perdio la conexion con el coordinador");

		log_error(log_planif, "Se perdio la conexion con el coordinador");

		kill(PLANIFICADOR_PID, SIGUSR1);

		close(SOCKET_COORDINADOR_CONSOLA);

		exit(1);
	}

	mostrar_info_status(info_status);

	imprimir_cola_bloqueados(clave);

	free(clave);

	free(info_status->mensaje);
	free(info_status);

	return 0;
}

int	com_deadlock(char* parametro){
	return 0;
}

char** controlar_y_obtener_parametros(char* parametro, int cantidad_parametros){
	char** parametros = string_split(stripwhite(parametro), " ");

	int indice = 0;
	while(parametros[indice]){
		indice ++;
	}

	if(cantidad_parametros == indice)
		return parametros;

	liberar_parametros(parametros, indice);
	return NULL;
}

void liberar_parametros(char** parametros, int cantidad_parametros){

	int indice;
	for(indice = 0; indice < cantidad_parametros; indice++){
		free(parametros[indice]);
	}

	free(parametros);
}

t_info_status* recibir_info_status(){
	int tamanio_mensaje;

	if(recv(SOCKET_COORDINADOR_CONSOLA, &tamanio_mensaje, sizeof(int), MSG_WAITALL) < 0)
		return NULL;

	char* mensaje = malloc(tamanio_mensaje);

	if(recv(SOCKET_COORDINADOR_CONSOLA, mensaje, tamanio_mensaje, MSG_WAITALL) < 0)
		return NULL;

	int id_instancia_actual;

	if(recv(SOCKET_COORDINADOR_CONSOLA, &id_instancia_actual, sizeof(int), MSG_WAITALL) < 0)
		return NULL;

	int id_instancia_posible;

	if(recv(SOCKET_COORDINADOR_CONSOLA, &id_instancia_posible, sizeof(int), MSG_WAITALL) < 0)
		return NULL;

	t_info_status* info_status = malloc(sizeof(t_info_status));

	info_status->id_instancia_actual = id_instancia_actual;
	info_status->id_instancia_posible = id_instancia_posible;
	info_status->mensaje = mensaje;
	info_status->tamanio_mensaje = tamanio_mensaje;

	return info_status;
}

void mostrar_info_status(t_info_status* info_status){
	char* mensaje = string_from_format("Valor : %s", info_status->mensaje);

	imprimir(mensaje);

	free(mensaje);

	if(info_status->id_instancia_actual == -1)
		mensaje = string_from_format("la clave no se encuentra actualmente en ninguna instancia.");
	else
		mensaje = string_from_format("La instancia actual en la que se encuentra tiene el ID : %d", info_status->id_instancia_actual );

	imprimir(mensaje);

	free(mensaje);

	mensaje = string_from_format("La instancia donde se guardaria tiene el ID : %d", info_status->id_instancia_posible);

	imprimir(mensaje);

	free(mensaje);
}

void imprimir_cola_bloqueados(char* clave){

	imprimir("El estado de la cola de bloqueados para la clave solicitada es:");

	void imprimir_bloqueado(void* elem){
		t_blocked* esi = (t_blocked*) elem;

		char* string = string_from_format("ESI %d", esi->info_ejecucion->ID);

		imprimir(string);

		free(string);
	}

	bool es_la_clave(void* elem){
		t_bloqueados_por_clave* bloqueados_por_clave = (t_bloqueados_por_clave*) elem;

		return string_equals_ignore_case(bloqueados_por_clave->clave, clave);
	}

	pthread_mutex_lock(&semaforo_cola_bloqueados);

	t_bloqueados_por_clave* resultado = list_find(colas_de_bloqueados, es_la_clave);

	list_iterate(resultado->bloqueados, imprimir_bloqueado);

	pthread_mutex_unlock(&semaforo_cola_bloqueados);

}








