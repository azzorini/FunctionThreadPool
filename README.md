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
struct Parameters {
	int x;
	int y;
};
```
El único requisito que ha de cumplir esta clase es que tiene que tener un constructor que no reciba ningún parámetro ya que en un punto los hilos crearán objetos de esta clase sin pasarles argumento alguno.

En este caso se ha hecho todo lo sencilla que podría ser, formada solo por dos enteros. Pero puede ser todo lo complicada que queramos, en un caso más realista tendría varios parámetros de diferentes tipos entre los cuales por ejemplo también se podría incluir la salida a un fichero para no usar la salida por pantalla. Esto no es tan sencillo y se discutirá una forma sencilla en la sección de Consejos. Lo importante es que esta clase tenga **todo** lo que nuestra función necesite para ejecutarse.

Aclarado esto vayamos ahora a la definición de la función:

```cpp
void funcion(const Parameters& p) {
	std::this_thread::sleep_for(std::chrono::seconds(1)); // Running, calculating, thinking...
	std::osyncstream out{std::cout};
	out << p.x*p.x + p.y*p.y << '\n';
}
```

Esta función tiene bastante libertad pero ha de cumplir dos requisitos:

1. La función no puede devolver nada, es decir, tiene que ser `void`.
2. La función tiene que recibir un solo input que ha de ser `const Parameters&`.

Dentro de eso el resto de la función es completamente libre. Aunque al no poder devolver nada lo lógico es que la salida se haga por pantalla o a un fichero. Es importante resaltar que la thread pool no se encargará de sincronizar la salida de las funciones. Por lo tanto si varias de nuestras funciones comparten salida es importante que sea la propia función quien garantice que no haya problemas. En este ejemplo simple esto se consigue utilizando la biblioteca `syncstream` incluída en el estándar de C++20. En caso de no querer usarla o de estar usando un estándar anterior se tendrá que hacer manualmente. La primera línea de la función es algo que en un caso realista no se quiere poner y simplemente pretende simular que la función está trabajando durante un tiempo.

Para acabar romperemos en diferentes partes el `main`. En primer lugar se construye nuestra pool:

```cpp
function_thread_pool<Parameters> Pool(funcion);
```

Esta clase se trata de una plantilla, la clase que tenemos que poner en la plantilla es la que hemos definido más arriba que tiene todos los parámetros necesarios para que se ejecute nuestra función de ahí viene que en el ejemplo se ponga `function_thread_pool<Parameters>`. Respecto a los argumentos que recibe el constructor hay uno obligatorio que es nuestra función y otro opcional que es el tamaño de la pool. Si no se le pasa el tamaño de la pool este será el dado por `std::thread::hardware_concurrency()` que se corresponde con la capacidad que tiene el ordenador de ejecutar tareas en paralelo. Hay que tener cuidado porque esta función no tiene en cuenta otras limitaciones como podría ser la memoria, en caso de que ese sea nuestro factor limitante tendremos que configurar el tamaño de la pool manualmente pasando ese segundo argumento.

Una vez inicializada la pool se usa un bucle para mandar todos los parámetros con los que queremos que nuestra función se ejecute:

```cpp
for (unsigned i = 1; i < 21; i++) Pool.submit(Parameters(i, i+1));
```

En este caso se mandan 20 configuraciones distintas porque se está trabajando con 8 hilos y 20 es una buena prueba pero el número es arbitrario. Para mandar unos parámetros con los que queremos que se ejecute nuestra función simplemente usamos el método `submit` que recibe esta configuración de parámetros y la añade a la cola de trabajos.

Por último se espera hasta que la cola de trabajos se vacíe usando el método `wait`.

```cpp
Pool.wait()
```

Este método espera hasta que la cola de trabajo se vacíe. Es importante notar que el hecho de que la cola de trabajo se vacíe no significa que todas las tareas ya hayan acabado. Puede ser que todavía haya hilos acabando su última ejecución. Sin embargo cuando se llame al destructor de la piscina se garantiza que se esperará a los hilos que todavía están haciendo su trabajo. De hecho si ya la cola de ejecución está vacía es beneficioso llamar al destructor porque así los hilos que vayan acabando dejarán de esperar a nuevos trabajos y podrán finalizar.

## Breve documentación

Se hará en este sección una breve documentación sobre los métodos de la clase `function_thread_pool`.

### Constructor: `function_thread_pool<P>::function_thread_pool(const std::function<void(const P&)>& f_, unsigned pool_size = std::thread::hardware_concurrency())`

Recibe un argumento obligatorio que es la función con la que queremos trabajar y además opcionalmente le podemos indicar cuantos hilos queremos que tenga nuestra pool. De no indicar este segundo argumento se tomará por defecto `std::thread::hardware_concurrency()` que es la máxima capacidad que tiene nuestro hardware de realizar tareas en paralelo. En caso de que el número de hilos sea menor que 2 el constructor tomará un valor de 2 ya que no tendría sentido tener un único hilo o ninguno.

### Método submit: `void function_thread_pool<P>::submit(const P& p)`

Recibe una configuración de parámetros de la clase con la que hayamos creado la plantilla y la añade a la cola de trabajo.

### Método wait: `void function_thread_pool<P>::wait()`

Espera hasta que la cola de trabajos se haya vaciado para continuar la ejecución. Pensado para ejecutarlo después de haber mandado todos los parámetros con los que queremos que la función se ejecute. Cabe destacar que la espera se hace mediante una condition_variable y por lo tanto no se hace un *busy waiting* esta condition_variable manda una notificación cada vez que un hilo empieza a ejecutarse.

