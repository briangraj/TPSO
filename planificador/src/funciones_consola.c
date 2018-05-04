/*
 * funciones_consola.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "funciones_consola.h"

void levantar_consola(void * param){

	setear_comandos();

	  char * linea;

	  while(1) {
	    linea = readline("Consola > ");

	    linea = stripwhite(linea);

	    if(!strncmp(linea, "exit", 4)) {
	       free(linea);
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
	return 0;
}
int	com_desbloquear(char* parametro){
	return 0;
}
int	com_listar(char* parametro){
	return 0;
}
int	com_kill(char* parametro){
	return 0;
}
int	com_status(char* parametro){
	return 0;
}
int	com_deadlock(char* parametro){
	return 0;
}








