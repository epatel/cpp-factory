default: run

factory:
	@echo ===== Compile factory ================================================
	g++ -o factory factory.cpp

run: factory
	@echo ===== Run factory ====================================================
	./factory

clean:
	rm -f *~ factory
