ORACLE_HOME = /oracle/orahome
ORA_H = -I$(ORACLE_HOME)/rdbms/public
ORA_LIB_DIR = -L$(ORACLE_HOME)/lib
ORA_LIB = -lclntsh
UTL_DIR = ../../utils/src
UTL_H = -I $(UTL_DIR)

omaotaclient: ./obj/main.o ./obj/dbpool.o ./obj/log.o ./obj/config.o
	g++ -o ../omaotaclient ./obj/main.o ./obj/dbpool.o ./obj/config.o ./obj/log.o $(ORA_LIB_DIR) $(ORA_LIB) -lcurl
./obj/main.o: main.cpp
	g++ -Wall -Wextra $(UTL_H) $(ORA_H) -c main.cpp -o ./obj/main.o
./obj/dbpool.o: $(UTL_DIR)/utils/dbpool/dbpool.cpp
	g++ -c $(UTL_DIR)/utils/dbpool/dbpool.cpp $(UTL_H) $(ORA_H) -o ./obj/dbpool.o
./obj/config.o: $(UTL_DIR)/utils/config/config.cpp
	g++ -c $(UTL_DIR)/utils/config/config.cpp -o ./obj/config.o
./obj/log.o: $(UTL_DIR)/utils/log/log.cpp
	g++ -c $(UTL_DIR)/utils/log/log.cpp -o ./obj/log.o

install:
	cp -f ../omaotaclient /usr/local/bin

clean:
	rm -f ./obj/* ../*.so