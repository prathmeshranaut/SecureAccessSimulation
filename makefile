CC=g++
CFLAGS=-std=c++17

INCLUDECADMIUM=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../DESTimes/include

#CREATE BIN AND BUILD FOLDERS TO SAVE THE COMPILED FILES DURING RUNTIME
bin_folder := $(shell mkdir -p bin)
build_folder := $(shell mkdir -p build)
results_folder := $(shell mkdir -p simulation_results)

#TARGET TO COMPILE ALL THE TESTS TOGETHER (NOT SIMULATOR)
message.o: data_structures/message.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) data_structures/message.cpp -o build/message.o

main_top.o: top_model/main.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) top_model/main.cpp -o build/main_top.o
	
main_alarmadmin_test.o: test/main_alarmadmin_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_alarmadmin_test.cpp -o build/main_alarmadmin_test.o

main_authentication_test.o: test/main_authentication_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_authentication_test.cpp -o build/main_authentication_test.o

main_display_test.o: test/main_display_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_display_test.cpp -o build/main_display_test.o

tests: main_alarmadmin_test.o main_display_test.o message.o
		$(CC) -g -o bin/ALARMADMIN_TEST build/main_alarmadmin_test.o build/message.o
		#$(CC) -g -o bin/AUTHENTICATION_TEST build/main_authentication_test.o build/message.o
		$(CC) -g -o bin/DISPLAY_TEST build/main_display_test.o build/message.o

#TARGET TO COMPILE ONLY ABP SIMULATOR
simulator: main_top.o message.o
	$(CC) -g -o bin/ABP build/main_top.o build/message.o 
	
#TARGET TO COMPILE EVERYTHING (ABP SIMULATOR + TESTS TOGETHER)
all: simulator tests

#CLEAN COMMANDS
clean: 
	rm -f bin/* build/*