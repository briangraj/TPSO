/*
 * distribucion.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "t_distribucion.h"


t_instancia* equitative_load(char* clave){
	t_instancia* instancia = list_get(INSTANCIAS, distribucion.proxima_instancia);

	distribucion.proxima_instancia = (distribucion.proxima_instancia + 1) % list_size(INSTANCIAS);

	if(!esta_activa(instancia))
		return equitative_load(clave);

	return instancia;
}

t_instancia* least_space_used(char* clave){
	t_instancia* instancia_con_mas_espacio = list_get(INSTANCIAS, 0);

	void tiene_mas_espacio(t_instancia* instancia){
		if(instancia_con_mas_espacio->entradas_disponibles < instancia->entradas_disponibles)
			instancia_con_mas_espacio = instancia;
	}

	list_iterate(INSTANCIAS, (void(*)(void*))tiene_mas_espacio);

	return instancia_con_mas_espacio;
}

t_instancia* key_explicit(char* clave){
	set_rangos();

	return elegir_instancia_segun_rango(clave);
}

void set_rangos(){
	t_list* instancias_activas = list_filter(INSTANCIAS, (bool(*)(void*))esta_activa);
	int cant_instancias_activas = list_size(instancias_activas);

	int max_rango = ceiling(26/cant_instancias_activas);
	int min_rango = 26 - max_rango*(cant_instancias_activas-1);

	int i;
	char letra_inicio = 'a';

	for(i=0; i < cant_instancias_activas -1 ;i++){

		t_rango* nuevo_rango = malloc(sizeof(t_rango));
		nuevo_rango->id_instancia = ((t_instancia*)list_get(instancias_activas, i))->id;

		nuevo_rango->inicio = letra_inicio;
		nuevo_rango->fin = letra_inicio + max_rango;

		list_add(distribucion.rangos, nuevo_rango);

		letra_inicio = letra_inicio + max_rango;
	}

	t_rango* nuevo_rango = malloc(sizeof(t_rango));
	nuevo_rango->id_instancia = ((t_instancia*)list_get(instancias_activas, i))->id;

	nuevo_rango->inicio = letra_inicio;
	nuevo_rango->fin = letra_inicio + min_rango;

	list_add(distribucion.rangos, nuevo_rango);

}

t_instancia* elegir_instancia_segun_rango(char* clave){
	char* clave_aux = strdup(clave);
	string_to_lower(clave_aux);

	bool esta_en_rango(t_rango* rango){
		return rango->inicio <= *clave_aux && *clave_aux <= rango->fin;
	}

	t_rango* rango = (t_rango*) list_find(distribucion.rangos, (bool(*)(void*))esta_en_rango);

	free(clave_aux);

	return instancia_de_id(rango->id_instancia);
}

int ceiling(double numero){
	return (int) numero < numero ? (int)numero +1 : numero;
}

