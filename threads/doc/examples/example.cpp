/*
 * example.cpp
 *
 *  Created on: 22/07/2014
 *      Author: rhermoso
 */

//-----------------------------------------------------------------------------
#include <iostream>
#include <cstring>
#include "threads.hpp"
//-----------------------------------------------------------------------------

/** [Thread Function Declaration] */
void threadFunction(nsThread::TThread* Sender, void* vUser)
{
	//-- vUser es el tercer parámetro que se pasó en la función create.
	/** [Counter Recovery] */
	int i_counter = *(int*)vUser;
	/** [Counter Recovery] */

	//-- Mientras no hayamos llegado a 10, se incrementa el contador en uno y
	//   se pinta el resultado por pantalla.
	if(i_counter < 10)
	{
		Sleep(1000);
		std::cout << "thread counter: " << ++i_counter << std::endl;
	}
	else
	{
		//-- Hemos llegado al límite del contador (10). Terminamos el hilo
		/** [Thread Terminate] */
		Sender->terminate();
		/** [Thread Terminate] */
	}
}
//-----------------------------------------------------------------------------
/** [Thread Function Declaration] */

/** [Main Function Declaration] */
int main(int argc, char* argv[])
{
	int i_counter = 0;
	//-- Se crea una instancia de la clase TThread
	/** [Thread Instantiation] */
	nsThread::TThread thrd;
	/** [Thread Instantiation] */

	//-- Se crea la función del hilo. Se le pasa i_counter como parámetro y
	//   comienza la ejecución del hilo inmediatamente.
	/** [Thread Function Creation] */
	thrd.create(threadFunction, (void*)&i_counter);
	/** [Thread Function Creation] */

	//-- Se crea un bucle que cada 300 ms, pinta por pantalla un contador,
	//   durante tres segundos.
	/** [Main Counter Loop] */
	for(int i = 0; i < 10; ++i)
	{
		std::cout << "main counter: " << i << std::endl;
		Sleep(300);
	}
	/** [Main Counter Loop] */
	//-- Se espera a la conclusión de la función del hilo
	/** [Thread Waiting] */
	thrd.waitFor();
	/** [Thread Waiting] */

	return 0;
}
//-----------------------------------------------------------------------------
/** [Main Function Declaration] */
