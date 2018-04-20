/*
 * funciones_consola.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "funciones_consola.h"

void levantar_consola(void * param){

	  char * linea;

	  while(1) {
	    linea = readline(">");

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
      printf("No se encontró el comando %s", word);
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


//COMANDOS

int	com_pausar(char* parametro){
	printf("El comando pausar recibió como parametro %s", parametro);
	return 0;
}
int	com_continuar(char* parametro){
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








