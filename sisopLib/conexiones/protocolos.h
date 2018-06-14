/*
 * protocolos.h
 *
 *  Created on: 28/9/2017
 *      Author: utnso
 */

#ifndef PROTOCOLOS_H_
#define PROTOCOLOS_H_

typedef enum {
	CONEXION_EXITOSA = 1, //Esto tiene el fin de reemplazar el "Todo ok papa..."
	HANDSHAKE,
	DESCONEXION_INMINENTE,

	//ESI - PLANIFICADOR
	EJECUTAR_SENTENCIA,
	FINALIZAR_PROCESO,
	FALLO_EN_EJECUCION,
	FIN_DEL_SCRIPT,

	//ESI - COORDINADOR
	OPERACION_GET, // Envio: [protocolo, tamanio_clave, clave] , clave es char*, incluye el /0 y esta considerado en el tamanio
	OPERACION_SET, // Envio: [protocolo, tamanio_clave, clave, tamanio_valor, valor] , clave y valor son char*, incluye el /0 y esta considerado en los tamanios
	OPERACION_STORE, // Envio: [protocolo, tamanio_clave, clave] , clave es char*, incluye el /0 y esta considerado en el tamanio
	ERROR_TAMANIO_CLAVE,
	ERROR_CLAVE_NO_IDENTIFICADA,
	ERROR_DE_COMUNICACION,
	ERROR_CLAVE_INACCESIBLE,

	//ESI GENERAL
	EJECUCION_EXITOSA,
	ERROR_LECTURA_SCRIPT,
	ERROR_INTERPRETACION_SENTENCIA,
	ENVIO_ID,

	//COORDINADOR - PLANIFICADOR
	GET_CLAVE,
	GET_EXITOSO,
	GET_BLOQUEANTE,
	GET_FALLIDO,
	STORE_CLAVE,
	STORE_INVALIDO,
	STORE_EXITOSO,
	SET_CLAVE,
	SET_EXITOSO,
	SET_INVALIDO,
	STATUS, // Envio: [protocolo, tamanio_clave, clave] , clave es char*, incluye el /0 y esta considerado en el tamanio
	ENVIO_INFO_STATUS, // Envio (desde coordinador a planificador) : [protocolo, tamanio_mensaje, mensaje, instancia_actual, instancia_posible]

	//COORDINADOR - INSTANCIA
	CONFIGURACION_ENTRADAS,
	CREAR_CLAVE,
	COMPACTACION,
	OPERACION_EXITOSA,//se usaria para saber si salio bien el set, store o crear_clave en la instancia
	FS_EI,//fallo set: espacio insuficiente
	FS_NC,//fallo set: necesita compactar
	//TODO FG_EI, FG_NC
	FALLO_REEMPLAZO
} t_protocolo;

typedef enum {
	ESI,
	PLANIFICADOR,
	CONSOLA_PLANIFICADOR,
	COORDINADOR,
	INSTANCIA

} t_proceso;


#endif /* PROTOCOLOS_H_ */
