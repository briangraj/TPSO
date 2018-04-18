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

} t_protocolo;

typedef enum {
	ESI,
	PLANIFICADOR,
	COORDINADOR,
	INSTANCIA

} t_proceso;


#endif /* PROTOCOLOS_H_ */
