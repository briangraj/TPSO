#Sistemas Operativos
#1C - 2018
#A-La-Grande-Le-Puse-Jacketing

DIR := ${CURDIR}
SEP := ----------------------

LOG_ERROR = 1
LOG_OK = 2
LOG_INFO = 3
LOG_TRACE = 6

# NO ANDA ! : all: lib tp

lib: commons readline parsi

restart: clean clean-dependencies

########## Instalar dependencias
commons:
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	git clone https://github.com/sisoputnfrba/so-commons-library ../so-commons-library
	cd ../so-commons-library; make; sudo make install
	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	

readline:
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	sudo apt-get install libreadline6 libreadline6-dev
	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	

parsi:
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	git clone https://github.com/sisoputnfrba/parsi.git ../parsi
	cd ../parsi; sudo make install
	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	

########## Compilar TP
## TODAVIA NO ANDA Y NO ENTIENDO POR QUE
#tp: sisopLib planificador esi coordinador instancia coordinador-mock

#sisopLib: 
#	$(call mostrarTitulo,s@)
#	cd $(DIR)/sisopLib/Debug; $(MAKE) all

########## Borrar los compilados del TP  
clean: clean-sisopLib clean-esi clean-instancia clean-planificador clean-coordinador

clean-sisopLib:
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	cd $(DIR)/sisopLib/Debug; rm -f *.o
	cd $(DIR)/sisopLib/Debug/conexiones ; rm -f *.o
	cd $(DIR)/sisopLib/Debug; rm -f libsisopLib.so
	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	
	
clean-esi: 
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	cd $(DIR)/esi/Debug; rm -f *.o
	cd $(DIR)/esi/Debug/src ; rm -f *.o
	cd $(DIR)/esi/Debug; rm -f esi
	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	

clean-instancia:
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	cd $(DIR)/instancia/Debug; rm -f *.o
	cd $(DIR)/instancia/Debug/src ; rm -f *.o
	cd $(DIR)/instancia/Debug; rm -f instancia
	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	

clean-planificador:
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	cd $(DIR)/planificador/Debug; rm -f *.o
	cd $(DIR)/planificador/Debug/src ; rm -f *.o
	cd $(DIR)/planificador/Debug; rm -f planificador
	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	
	
clean-coordinador:
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	cd $(DIR)/coordinador/Debug; rm -f *.o
	cd $(DIR)/coordinador/Debug/src ; rm -f *.o
	cd $(DIR)/coordinador/Debug; rm -f coordinador
	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	
	
########## Desinstalar y borrar dependencias
clean-dependencies: clean-commons clean-parsi

clean-commons:
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	if [ -d "$(DIR)/../so-commons-library" ]; then \
		cd $(DIR)/../so-commons-library/ make clean && cd ./src && sudo make uninstall; \
		sudo rm -rf $(DIR)/../so-commons-library; \
    fi

	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	
		
clean-parsi:
	$(call print,$(SEP) $@ $(SEP), $(LOG_TRACE))
	if [ -d "$(DIR)/../parsi"]; then \
		cd $(DIR)/../parsi && make clean && cd ./src/ && sudo make uninstall; \
		sudo rm -rf $(DIR)/../parsi; \
	fi

	$(call print,$(SEP) $@ - LISTO! $(SEP), $(LOG_OK))	

## clean-readline no hace falta porque si esta instalado no pasa naranja

## TEST
test:
	$(call print, "ROJO", 1)
	$(call print, "VERDE", 2)
	$(call print, "CIAN", 6)
	$(call print, "AMARILLO", 3)
	$(call print, $(SEP) $@ $(SEP), $(LOG_TRACE))
## Funciones

define print
      @tput setaf $2
      @echo $1
      @tput sgr0
      @echo 
endef
