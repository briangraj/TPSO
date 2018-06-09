#Sistemas Operativos
#1C - 2018
#A-La-Grande-Le-Puse-Jacketing

DIR := ${CURDIR}

lib: so-commons-library readline parsi

so-commons-library:
	$(call mostrarTitulo,$@)
	git clone https://github.com/sisoputnfrba/so-commons-library ../so-commons-library
	cd ../so-commons-library; sudo make install
	
readline:
	$(call mostrarTitulo,$@)
	sudo apt-get install libreadline6 libreadline6-dev

parsi:
	$(call mostrarTitulo,$@)
	git clone https://github.com/sisoputnfrba/parsi.git ../parsi
	cd ../parsi; sudo make install

clean: clean-sisopLib clean-esi clean-instancia clean-planificador clean-coordinador

clean-sisopLib:
	$(call mostrarTitulo,$@)
	cd $(DIR)/sisopLib/Debug; rm -f *.o
	cd $(DIR)/sisopLib/Debug/conexiones ; rm -f *.o
	cd $(DIR)/sisopLib/Debug; rm -f libsisopLib.so
	
clean-esi: 
	$(call mostrarTitulo,$@)
	cd $(DIR)/esi/Debug; rm -f *.o
	cd $(DIR)/esi/Debug/src ; rm -f *.o
	cd $(DIR)/esi/Debug; rm -f esi
	
clean-instancia:
	$(call mostrarTitulo,$@)
	cd $(DIR)/instancia/Debug; rm -f *.o
	cd $(DIR)/instancia/Debug/src ; rm -f *.o
	cd $(DIR)/instancia/Debug; rm -f instancia

clean-planificador:
	$(call mostrarTitulo,$@)
	cd $(DIR)/planificador/Debug; rm -f *.o
	cd $(DIR)/planificador/Debug/src ; rm -f *.o
	cd $(DIR)/planificador/Debug; rm -f planificador
	
clean-coordinador:
	$(call mostrarTitulo,$@)
	cd $(DIR)/coordinador/Debug; rm -f *.o
	cd $(DIR)/coordinador/Debug/src ; rm -f *.o
	cd $(DIR)/coordinador/Debug; rm -f coordinador
	
	
define mostrarTitulo
	@echo
	@echo "########### $(1) ###########################################"
endef
