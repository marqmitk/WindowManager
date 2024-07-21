objects = Config.cpp Manager.cpp Utils.cpp Containers.cpp main.cpp
cc = g++
flags = -pthread -lgdi32
output = a.exe

all:
	@echo "Building and Running..."
	$(cc) -o $(output) $(objects) $(flags)

	./a.exe

debug:
	@echo "Building in Debug mode..."
	$(cc) -o $(output) $(objects) $(flags) -g
	

build: 
	@echo "Building..."
	$(cc) -o $(output) $(objects) $(flags) -g

run:
	@echo "Running..."
	./$(output)

clean:
	@echo "Cleaning..."
	rm -f $(output)
