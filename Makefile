all:
	@echo "Building and Running..."
	g++ -o Manager.exe utils.cpp Manager.cpp main.cpp -pthread -lgdi32
	./Manager.exe
;
build: 
	@echo "Building..."
	g++ -o Manager.exe utils.cpp Manager.cpp main.cpp -pthread -lgdi32 

run:
	@echo "Running..."
	./Manager.exe

clean:
	@echo "Cleaning..."
	rm -f Manager.exe
