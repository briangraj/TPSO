/*
 * funciones_consola.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "funciones_consola.h"

void levantar_consola(void * param){


	if((SOCKET_COORDINADOR_CONSOLA = conectarse_a_coordinador(CONSOLA_PLANIFICADOR)) == -1){
		imprimir("No se pudo conectar la consola al coordinador");

		kill(PLANIFICADOR_PID, SIGUSR1);

		exit(1);
	}

	nueva_espera = malloc(sizeof(t_espera_circular));


	setear_comandos();

	char * linea;

	while(1) {
		linea = readline("Consola > ");

		linea = stripwhite(linea);

		if(!strncmp(linea, "exit", 4)) {
		   free(linea);
		   free(nueva_espera);
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

	comandos[8].nombre = "check";
	comandos[8].funcion = com_check;
	comandos[8].descripcion = "Permitirá mostrar el estado de la cola de ready";

	comandos[9].nombre = NULL;
	comandos[9].funcion = NULL;
	comandos[9].descripcion = NULL;

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

	char** parametros = controlar_y_obtener_parametros(parametro, 0);

	if(!parametros){
		imprimir("El comando pausar no recibe parametros");
		return 0;
	}

	free(parametros);

	pthread_mutex_lock(&semaforo_pausa);
	PAUSA = true;
	pthread_mutex_unlock(&semaforo_pausa);

	imprimir("El planificador esta en pausa, escriba continuar para reanudar su ejecucion.");

	return 0;
}

int	com_continuar(char* parametro){

	char** parametros = controlar_y_obtener_parametros(parametro, 0);

	if(!parametros){
		imprimir("El comando continuar no recibe parametros");
		return 0;
	}

	free(parametros);

	pthread_mutex_lock(&semaforo_pausa);
	PAUSA = false;
	pthread_mutex_unlock(&semaforo_pausa);

	imprimir("El planificador reanudo su ejecucion.");

	mandar_a_ejecutar();

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

	}else{

		bool es_el_esi_con_el_recurso(void* elem){
			t_recursos_por_esi* rec = (t_recursos_por_esi*)elem;

			return rec->id_esi == esi_bloqueado->info_ejecucion->ID;
		}

		bool es_la_clave(void* elem){
			char* recurso_del_esi = (char*) elem;

			return string_equals_ignore_case(recurso_del_esi, clave);
		}

		pthread_mutex_lock(&semaforo_asignaciones);
		t_recursos_por_esi* recursos_del_esi = list_find(colas_de_asignaciones, es_el_esi_con_el_recurso);
		pthread_mutex_unlock(&semaforo_asignaciones);

		if(list_any_satisfy(recursos_del_esi->recursos_asignados, es_la_clave)){

			imprimir("El ESI ya tenía esta clave asignada, no hacemos nada, issue #1158");

			return 0;
		}

	}


	if(id_esi_activo == id_esi){
		hay_que_bloquear_esi_activo(clave, true);

		free(esi_bloqueado->info_ejecucion);

		free(esi_bloqueado);
	}

	else {

		bloqueados_por_clave = encontrar_bloqueados_para_la_clave(clave);

		pthread_mutex_lock(&semaforo_cola_bloqueados);

		list_add(bloqueados_por_clave->bloqueados, esi_bloqueado);

		pthread_mutex_unlock(&semaforo_cola_bloqueados);

		//Ahora sacamos al ESI de la cola de listos

		pthread_mutex_lock(&semaforo_cola_listos);

		list_remove_and_destroy_by_condition(cola_de_listos, coincide_el_id, free_elem);

		pthread_mutex_unlock(&semaforo_cola_listos);

	}

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

	bool es_el_recurso(void* elem){
		t_bloqueados_por_clave* bloq = (t_bloqueados_por_clave*) elem;

		return string_equals_ignore_case(bloq->clave, clave);
	}

	t_bloqueados_por_clave* bloqueados_por_clave = encontrar_bloqueados_para_la_clave(clave);

	if(!bloqueados_por_clave){
		imprimir("La clave ingresada no se encuentra bloqueada actualmente ni hay ESIs bloqueados por ella");

		free(clave);
		return 0;
	}

	if(bloqueados_por_clave->id_proximo_esi > 0 && list_is_empty(bloqueados_por_clave->bloqueados)){

		// Despojamos al esi de la posesion de la clave, issue #1158

		t_list* recursos_del_esi = asignados_para_el_esi(bloqueados_por_clave->id_proximo_esi);

		bool es_la_clave(void* elem){
			char* recurso = (char*)elem;

			return string_equals_ignore_case(recurso, clave);
		}

		list_remove_and_destroy_by_condition(recursos_del_esi, es_la_clave, free_elem);

		// Borramos la estructura de bloqueados para la clave de las colas de bloqueados

		bool es_el_recurso(void* elem){
			t_bloqueados_por_clave* bloq = (t_bloqueados_por_clave*) elem;

			return string_equals_ignore_case(bloq->clave, clave);
		}

		char* mensaje = string_from_format("La clave fue desalojada del ESI %d quien la poseia, ya que no habia bloqueados para la misma", bloqueados_por_clave->id_proximo_esi);

		imprimir(mensaje);

		free(mensaje);

		pthread_mutex_lock(&semaforo_cola_bloqueados);
		list_remove_and_destroy_by_condition(colas_de_bloqueados, es_el_recurso, clave_destroyer);
		pthread_mutex_unlock(&semaforo_cola_bloqueados);

		free(clave);

		return 0;

	} else {

		if(bloqueados_por_clave->id_proximo_esi == -1 && list_is_empty(bloqueados_por_clave->bloqueados)){

			pthread_mutex_lock(&semaforo_cola_bloqueados);
			list_remove_and_destroy_by_condition(colas_de_bloqueados, es_el_recurso, clave_destroyer);
			pthread_mutex_unlock(&semaforo_cola_bloqueados);

			log_trace(log_planif, "La clave %s, inicialmente bloqueda por config, ahora se encuentra desbloqueada", clave);
			imprimir("La clave solicitada estaba bloqueada por archivo de configuración y ahora se encuentra desbloqueada");

			free(clave);

			return 0;

		}

		bool esta_bloqueado_por_consola(void* elem){
			t_blocked* esi_block = (t_blocked*) elem;

			return esi_block->bloqueado_por_consola;
		}

		pthread_mutex_lock(&semaforo_cola_bloqueados);
		t_blocked* esi_bloqueado = list_find(bloqueados_por_clave->bloqueados, esta_bloqueado_por_consola);//Buscamos al primer bloqueado por consola
		pthread_mutex_unlock(&semaforo_cola_bloqueados);


		//Chequear que se haya encontrado algun ESI bloqueado por consola
		if(!esi_bloqueado){
			imprimir("No hay ningun ESI desloqueable por consola para la clave solicitada. Se liberara la clave para los bloqueados por ejecucion y se desbloqueara al primero de ellos");

			pthread_mutex_lock(&semaforo_cola_bloqueados);
			t_blocked* blocked = proximo_no_bloqueado_por_consola(bloqueados_por_clave->bloqueados);
			pthread_mutex_unlock(&semaforo_cola_bloqueados);

			if(blocked){
				bloqueados_por_clave->id_proximo_esi = blocked->info_ejecucion->ID;

				aniadir_a_listos(*(blocked->info_ejecucion));
				eliminar_de_bloqueados(blocked->info_ejecucion);
			}

			free(clave);

			return 0;
		}

		int id_esi = esi_bloqueado->info_ejecucion->ID;

		char* informe;

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
		return 0;
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

	if(enviar_paquete(STATUS, SOCKET_COORDINADOR_CONSOLA, tamanio_paquete, paquete) < 0){
//		free(parametro); // que es free(linea);

		imprimir("Se perdio la conexion con el coordinador");

		log_error(log_planif, "Se perdio la conexion con el coordinador");

		kill(PLANIFICADOR_PID, SIGUSR1);

		close(SOCKET_COORDINADOR_CONSOLA);

		exit(1);
	}

	t_info_status* info_status = recibir_info_status();

	if(!info_status){
//		free(parametro); // que es free(linea);

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
//	if(parametro){
//		imprimir("El comando deadlock no recibe parametros");
//		return 0;
//	}

	int id_espera = 0;

	esperas_circulares = list_create();

	void buscar_espera_circular(void* elem){
		imprimir("Voy a empezar una nueva busqueda de esperas circulares");

		encontre_un_ciclo = false;

		t_bloqueados_por_clave* bloqueados_por_clave = (t_bloqueados_por_clave*) elem;

		recurso_inicial = bloqueados_por_clave->clave;

		cargar_si_recurso_forma_parte_de_un_deadlock(bloqueados_por_clave);

		if(encontre_un_ciclo){
			imprimir("Encontre un ciclo!!");
			t_espera_circular* espera = malloc(sizeof(t_espera_circular));

			espera->esis_por_recurso = duplicar_lista_involucrados(nueva_espera->esis_por_recurso);
			espera->id_espera = id_espera;

			id_espera++;

			list_add(esperas_circulares, espera);

			list_destroy_and_destroy_elements(nueva_espera->esis_por_recurso, involucrados_destroyer);
		}

	}

	imprimir("Voy a pausar todo para analizar si hay deadlock");

	pthread_mutex_lock(&semaforo_pausa);

	list_iterate(colas_de_bloqueados, buscar_espera_circular);

	imprimir_esperas_circulares();

	list_destroy_and_destroy_elements(esperas_circulares, esperas_destroyer);

	pthread_mutex_unlock(&semaforo_pausa);

	return 0;
}

t_list* duplicar_lista_involucrados(t_list* involucrados){
	t_list* involucrados_copia = list_create();

	void copiar(void* elem){
		t_involucrados* originales = (t_involucrados*) elem;

		t_involucrados* copia = malloc(sizeof(t_involucrados));

		copia->id_bloqueado = originales->id_bloqueado;
		copia->id_esi_duenio = originales->id_esi_duenio;
		copia->recurso = strdup(originales->recurso);

		list_add(involucrados_copia, copia);
	}

	list_iterate(involucrados, copiar);

	return involucrados_copia;
}

void imprimir_esperas_circulares(){

	if(list_is_empty(esperas_circulares))
		imprimir("No se encontraron esperas circulares, no hay deadlock");


	void imprimir_espera(void* elem){
		imprimir("Voy a imprimir una nueva espera");
		t_espera_circular* espera = (t_espera_circular*) elem;

		char* id = string_from_format("Id de la espera : %d", espera->id_espera);

		imprimir(id);

		free(id);

		void imprimir_involucrados(void* elem2){
			t_involucrados* involucrados = (t_involucrados*) elem2;

			char* mensaje = string_from_format("El recurso %s es poseido por el ESI de id %d, y esta siendo esperado por el ESI de id %d", involucrados->recurso, involucrados->id_esi_duenio, involucrados->id_bloqueado);

			imprimir(mensaje);

			free(mensaje);
		}

		list_iterate(espera->esis_por_recurso, imprimir_involucrados);
	}

	list_iterate(esperas_circulares, imprimir_espera);

}

void esperas_destroyer(void* elem){
	t_espera_circular* espera_circular = (t_espera_circular*) elem;

	list_destroy_and_destroy_elements(espera_circular->esis_por_recurso, involucrados_destroyer);
}

void involucrados_destroyer(void* elem){
	t_involucrados* involucrados = (t_involucrados*) elem;

	free(involucrados->recurso);
}

void cargar_si_recurso_forma_parte_de_un_deadlock(t_bloqueados_por_clave* bloqueados_por_clave){
	if(hay_que_descartarla(bloqueados_por_clave)) return;

	int indice = 0;
	t_blocked* esi;

	while(!encontre_un_ciclo){
		imprimir("Voy a analizar un nuevo esi");
		esi = list_get(bloqueados_por_clave->bloqueados, indice);

		if(!esi)
			return;

		if(list_is_empty(asignados_para_el_esi(esi->info_ejecucion->ID))){
			imprimir("La lista de asignados para el esi esta vacia");
			indice++;

			continue;
		}

		int indice_recursos = 0;

		char* recurso;

		while(!encontre_un_ciclo){
			imprimir("Voy a analizar un nuevo recurso para el esi");
			recurso = list_get(asignados_para_el_esi(esi->info_ejecucion->ID), indice_recursos);

			if(!recurso){
				imprimir("El esi se quedo sin recursos");
				break;
			}


			if(string_equals_ignore_case(recurso, recurso_inicial)){
				imprimir("Encontre deadlock en el while !!!!!");

				encontre_un_ciclo = true;

				nueva_espera->esis_por_recurso = list_create();

				break;
			}

			imprimir("Me voy a meter recursivamente");
			cargar_si_recurso_forma_parte_de_un_deadlock(encontrar_bloqueados_para_la_clave(recurso));

			indice_recursos++;
		}

		indice++;
	}

	imprimir("Voy a armar el t_involucrados");
	t_involucrados* involucrados = malloc(sizeof(t_involucrados));

	involucrados->id_bloqueado = esi->info_ejecucion->ID;
	involucrados->id_esi_duenio = bloqueados_por_clave->id_proximo_esi;
	involucrados->recurso = strdup(bloqueados_por_clave->clave);

	list_add(nueva_espera->esis_por_recurso, involucrados);
}

bool hay_que_descartarla(t_bloqueados_por_clave* bloqueados_por_clave){
	bool es_el_esi(void* elem){
		t_ready* esi_ready = (t_ready*) elem;

		return bloqueados_por_clave->id_proximo_esi == esi_ready->ID;
	}

	bool esta_en_la_espera(void* elem){
		t_espera_circular* espera = (t_espera_circular*) elem;

		bool es_la_clave(void* elem){
			t_involucrados* bloq = (t_involucrados*) elem;

			return string_equals_ignore_case(bloq->recurso, bloqueados_por_clave->clave);
		}

		return (espera->esis_por_recurso != NULL) && list_any_satisfy(espera->esis_por_recurso, es_la_clave);
	}

	pthread_mutex_lock(&semaforo_cola_listos);
	bool esta_en_ready = list_any_satisfy(cola_de_listos, es_el_esi);
	pthread_mutex_unlock(&semaforo_cola_listos);

	return bloqueados_por_clave->id_proximo_esi == 0 || list_is_empty(bloqueados_por_clave->bloqueados) || esta_en_ready || list_any_satisfy(esperas_circulares, esta_en_la_espera);
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
	if(recibir_protocolo(SOCKET_COORDINADOR_CONSOLA) != ENVIO_INFO_STATUS)
		return NULL;

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
	pthread_mutex_unlock(&semaforo_cola_bloqueados);

	if(!resultado){
		imprimir("No existe una cola de bloqueados para la clave solicitada");
		return;
	}

	char* mensaje = string_from_format("El ID del ESI que tiene derecho a la clave solicitada es: \n%d", resultado->id_proximo_esi);

	imprimir(mensaje);

	free(mensaje);

	imprimir("El estado de la cola de bloqueados para la clave solicitada es:");

	pthread_mutex_lock(&semaforo_cola_bloqueados);
	if(list_is_empty(resultado->bloqueados))
		imprimir("La cola de bloqueados esta vacia");
	list_iterate(resultado->bloqueados, imprimir_bloqueado);
	pthread_mutex_unlock(&semaforo_cola_bloqueados);

}

int com_check(char* parametro){
	char** parametros = controlar_y_obtener_parametros(parametro, 1);
	// 0 : TODAS LAS SIGUIENTES
	// 1 : COLA DE LISTOS
	// 2 : COLA DE FINALIZADOS
	// 3 : COLAS DE ASIGNACIONES
	// 4 : ESTIMACIONES (SOLO DE LOS QUE ESTÁN EN READY)

	if(!parametros){
		mostrar_todo();

		return 0;
	}


	switch(atoi(parametros[0])){
		case 1:
			mostrar_cola_listos();
			break;
		case 2:
			mostrar_cola_finalizados();
			break;
		case 3:
			mostrar_asignaciones();
			break;
		case 4:
			mostrar_estimaciones();
			break;
		default:
			mostrar_todo();
	}

	liberar_parametros(parametros, 1);

	return 0;
}

void mostrar_cola_listos(){
	char* mensaje = string_from_format("\n------- El estado de la cola de listos es: ------- \n\t[ ");

	void imprimir_listo(void* elem){
		t_ready* esi = (t_ready*) elem;

		string_append_with_format(&mensaje, " ESI %d,", esi->ID);
	}

	pthread_mutex_lock(&semaforo_cola_listos);
	list_iterate(cola_de_listos, imprimir_listo);
	pthread_mutex_unlock(&semaforo_cola_listos);

	char* mensaje_final = string_substring(mensaje, 0, strlen(mensaje) - 1);

	free(mensaje);

	string_append(&mensaje_final, " ]");

	imprimir(mensaje_final);

	free(mensaje_final);
}

void mostrar_cola_finalizados(){
	char* mensaje = string_from_format("\n------- El estado de la cola de finalizados es: ------- \n");

	void imprimir_finalizado(void* elem){
		t_ended* esi = (t_ended*) elem;

		string_append_with_format(&mensaje, "\tESI %d: \n\t exit_text: %s\n\t tiempo_total_espera: %d \n\t tiempo_total_bloqueado: %d \n\t total_instrucciones_ejecutadas: %d\n",
				esi->ID, esi->exit_text, esi->tiempo_total_espera, esi->tiempo_total_bloqueado, esi->total_instrucciones_ejecutadas);
	}

	pthread_mutex_lock(&semaforo_cola_finalizados);
	list_iterate(cola_finalizados, imprimir_finalizado);
	pthread_mutex_unlock(&semaforo_cola_finalizados);

	imprimir(mensaje);

	free(mensaje);
}

void mostrar_asignaciones(){
	char* mensaje = string_from_format("\n------- El estado de la cola de asignaciones es: ------- \n");
//	char* mensaje_final =string_new();

	void imprimir_recursos_del_esi(void* elem){
		char* clave = (char*) elem;

		string_append_with_format(&mensaje, "%s, ", clave);
	}

	void imprimir_asignaciones(void* elem){
		t_recursos_por_esi* recursos_por_esi = (t_recursos_por_esi*) elem;

		string_append_with_format(&mensaje, "\n\tESI %d =>\t[ ", recursos_por_esi->id_esi);

		if(!list_is_empty(recursos_por_esi->recursos_asignados)){

			list_iterate(recursos_por_esi->recursos_asignados, imprimir_recursos_del_esi);

			mensaje = string_substring(mensaje, 0, strlen(mensaje) - 2);
		}

		string_append_with_format(&mensaje, "]");

	}

	pthread_mutex_lock(&semaforo_asignaciones);
	list_iterate(colas_de_asignaciones, imprimir_asignaciones);
	pthread_mutex_unlock(&semaforo_asignaciones);

	imprimir(mensaje);

	free(mensaje);
}

void mostrar_estimaciones(){

	char* mensaje = string_from_format("\n------- Las estimaciones de los ESIs en estado ready son: ------- \n\t[ ");

	void imprimir_estimaciones(void* elem){
		t_ready* esi = (t_ready*) elem;

		string_append_with_format(&mensaje, " ESI %d - %f,", esi->ID, esi->estimacion_actual);
	}

	pthread_mutex_lock(&semaforo_cola_listos);
	list_iterate(cola_de_listos, imprimir_estimaciones);
	pthread_mutex_unlock(&semaforo_cola_listos);

	char* mensaje_final = string_substring(mensaje, 0, strlen(mensaje) - 1);

	free(mensaje);

	string_append(&mensaje_final, " ]");

	imprimir(mensaje_final);

	free(mensaje_final);

}

void mostrar_todo(){

	mostrar_cola_listos();
	mostrar_cola_finalizados();
	mostrar_asignaciones();
	mostrar_estimaciones();

}









