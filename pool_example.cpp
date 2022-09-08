#include <iostream>
#include <chrono>
#include <syncstream>

#include "FunctionThreadPool.hpp"

// Creamos nuestra clase que servirá para pasar todos los parámetros que necesite nuestra función
struct Parameters {
	int x;
	int y;
};

// Esta es la función que queremos usar es importante que sea void y que por lo tanto todo su output vaya por pantalla o a ficheros
// además ha de recibir un solo input que sea una referencia constante a un objeto de la clase que hayamos creado
void funcion(const Parameters& p) {
	std::this_thread::sleep_for(std::chrono::seconds(1)); // Running, calculating, thinking...
	std::osyncstream out{std::cout};
	out << p.x*p.x + p.y*p.y << '\n';
}

int main() {
	// Creamos el objeto en este caso solo le hemos pasado la función así que el número de hilos que tomará vendrá dado por
	// std::thread::hardware_concurrency()
	function_thread_pool<Parameters> Pool(funcion);

	// Enviamos todos los parámetros con los que queremos que la función se ejecute
	for (unsigned i = 1; i < 21; i++) Pool.submit(Parameters(i, i+1));

	// Esperamos hasta que la cola de parámetros con los que se tiene que ejecutar la función se vacíe
	Pool.wait();

	return 0;
}
