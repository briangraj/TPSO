#!/bin/sh

DIR=/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing

mostrar (){
  echo '###########' $1 '###########################################'
}

mostrar sisopLib
(cd $DIR/sisopLib/Debug && make all)
mostrar planificador
(cd $DIR/planificador/Debug && make all)
mostrar esi
(cd $DIR/esi/Debug && make all)
mostrar instancia
(cd $DIR/instancia/Debug && make all)
mostrar coordinador
(cd $DIR/coordinador/Debug && make all)

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/sisopLib/Debug/"
export LC_ALL=$LC_ALL:"C"
