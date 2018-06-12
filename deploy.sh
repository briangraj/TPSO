#!/bin/sh

DIR=/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing
WORKSPACE=/home/utnso/workspace
NOSOTROS=A-la-grande-le-puse-Jacketing
CLONAR = 0 #false
MANDO_FRUTA = 0 #false

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

opciones_repo(){
 read -p "El repo ya existe, queres borrarlo y empezar de cero (b), dejarlo asi como esta y compilar (c), hacer pull y compilar los combios (p) o salir del script (s) ? : " resp
 case $resp in
	[Bb]* ) MANDO_FRUTA = 0 && CLONAR = 1 && sudo rm -rf $DIR && (mostrar "El repo fue borrado" $LOG_INFO) ;;
	[Cc]* ) MANDO_FRUTA = 0;;
	[Pp]* ) MANDO_FRUTA = 0 && cd $DIR && git pull;;
	[Ss]* ) return;;
	* ) MANDO_FRUTA = 1 && mostrar "Dale, no mandes fruta." $LOG_ERROR;;
}

mostrar "$NOSOTROS - EMPEZANDO DEPLOY" $LOG_TITLE

mostrar "Preparando el repo" $LOG_INFO
if [ ! -d $WORKSPACE ]
  then (cd /home/utnso && mkdir workspace) && mostrar "Workspace creado" LOG_INFO && CLONAR = 1

elif [ -d "$DIR" ]
  then 
    opciones_repo
    while [ MANDO_FRUTA ]; do
	   opciones_repo
    esac
fi

if [ $CLONAR ]
  then (cd $WORKSPACE && git clone https://github.com/sisoputnfrba/tp-2018-1c-A-la-grande-le-puse-Jacketing.git)
fi

if [ -d $DIR ]; 
  then
    if [ ! $CLONAR ]
      then
	 mostrar "Repo clonado con exito" $LOG_OK
    fi  
  else
    mostrar "Error clonando el repo" $LOG_ERROR
    return
fi

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

export LD_LIBRARY_PATH=/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/sisopLib/Debug/
mostrar "LD_LIBRARY_PATH quedo seteado como $LD_LIBRARY_PATH" $LOG_INFO
export LC_ALL=C

mostrar "$NOSOTROS - DEPLOY TERMINADO" $LOG_TITLE
