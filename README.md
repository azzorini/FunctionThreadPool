# FunctionThreadPool

FunctionThreadPool es una biblioteca hecha en C++ que te permite ejecutar concurrentemente una función con una cola de parámetros.

## ¿Qué incluye?

La biblioteca consta solamente de un header (*FunctionThreadPool.hpp*) y un archivo en el que se han implementado algunos de los métodos de las clases que se crean en dicho header (*FunctionThreadPool.tpp*).

Además en el repositorio se incluye el archivo *pool_example.cpp* para hacer una demostración del uso de dicha biblioteca.

En dicha biblioteca se definen tres clases: threadsafe_queue, join_threads y function_thread_pool.

1. threadsafe_queue: Esta es una cola a la cual es seguro que accedan varios hilos de forma simultánea. Esto será muy importante a la hora de crear la pool.
2. join_threads: Esta clase recibe una referencia a un vector de hilos y lo único que hace es asegurarse de que cuando se vaya a destruir se espere primero a que todos los hilos hayan acabado su tarea.
3. function_thread_pool: Es la plantilla que se espere que use el usuario y por lo tanto en la que nos vamos a centrar. Crea una piscina de hilos que ejecutan una cierta función con varios parámetros.

## Breve ejemplo

Explicaremos aquí el ejemplo expuesto en *pool_example.cpp*:

```cpp
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
```

Vayamos parte por parte. En primer lugar tenemos las bibliotecas que hemos incluido:

```cpp
#include <iostream>
#include <chrono>
#include <syncstream>

#include "FunctionThreadPool.hpp"
```

Incluimos iostream porque queremos hacer uso de la salida por pantalla, incluímos chrono para tener un control del tiempo ya que para simular que se está realizando una tarea pesada nuestra función de prueba dormirá durante un tiempo y syncstream para sincronizar la salida por pantalla y que cada hilo escriba su resultado ordenadamente. Esta biblioteca está solo disponible a partir de C++20 y por lo tanto el ejemplo se tiene que compilar con este estándar.

Por último incluímos nuestra biblioteca *FunctionThreadPool.hpp* que se tiene que encontrar en el mismo directorio que nuestro código.

Pasamos ahora a la definición de la clase que contiene todos los parámetros:

```cpp
struct Parametres {
  int x;
  int y;
};
