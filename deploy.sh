#!/bin/sh

DIR=/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing
WORKSPACE=/home/utnso/workspace
NOSOTROS=A-la-grande-le-puse-Jacketing
SEP='----------------------'

LOG_ERROR='\033[0;31m'
LOG_TRACE='\033[0;36m'
LOG_OK='\033[0;32m'
LOG_INFO='\033[0;33m'
LOG_TITLE='\033[0;35m'
NC='\033[0m' # No Color

mostrar (){
  echo -e "$2 $SEP $1 $SEP $NC"
  echo ''
}

mostrar "$NOSOTROS - EMPEZANDO DEPLOY" $LOG_TITLE

mostrar "Instalando dependencias" $LOG_INFO
(cd $DIR && make restart && make lib)

mostrar "Comenzando compilacion" $LOG_INFO
mostrar sisopLib $LOG_TRACE
(cd $DIR/sisopLib/Debug && make all)
mostrar "sisopLib - LISTO!" $LOG_OK

mostrar planificador $LOG_TRACE
(cd $DIR/planificador/Debug && make all)
mostrar "planificador - LISTO!" $LOG_OK

mostrar esi $LOG_TRACE
(cd $DIR/esi/Debug && make all)
mostrar "esi - LISTO!" $LOG_OK

mostrar instancia $LOG_TRACE
(cd $DIR/instancia/Debug && make all)
mostrar "instancia - LISTO!" $LOG_OK

mostrar coordinador $LOG_TRACE
(cd $DIR/coordinador/Debug && make all)
mostrar "coordinador - LISTO!" $LOG_OK

mostrar coordinador-mock $LOG_TRACE
(cd $DIR/coordinador-mock/Debug && make all)
mostrar "coordinador-mock - LISTO!" $LOG_OK

echo export LD_LIBRARY_PATH=/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/sisopLib/Debug/ >> /home/utnso/.bashrc
mostrar "LD_LIBRARY_PATH quedo seteado como $LD_LIBRARY_PATH" $LOG_INFO

mostrar "$NOSOTROS - DEPLOY TERMINADO" $LOG_TITLE
