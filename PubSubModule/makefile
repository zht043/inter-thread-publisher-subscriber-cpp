

default: module_runner.exe

compiler = clang++
#compiler = g++

boostlib = -lboost_system -lboost_thread -lpthread -lboost_chrono

std = -std=c++17

cppflags = $(std) $(boostlib)
 
%.o: %.cpp
	$(compiler) -c $< 

%.exe: module_runner.o ModuleA.o ModuleB.o ModuleC.o  
	$(compiler) -g -o  $@ module_runner.o ModuleA.o ModuleB.o ModuleC.o  $(cppflags) 
	@rm *.o
	@echo compilation completed

clean:
	@rm -f *.exe
	@rm -f *.o
	@rm -f *.txt
