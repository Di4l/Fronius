/**
 * \file
 *   Define un hilo de manera transparente a la plataforma de compilación. Se
 * definen macros, variables y constantes que se mapean a las correspondientes
 * definiciones de la plataforma en uso.
 *
 *   Se encapsula todas las llamadas a la API correspondiente y la función del
 * hilo en la clase NsThread::TThread.
 *
 *   Se aprovecha para declarar macros y defines de manera multiplataforma,
 * como son el uso de mutex y sus funciones y la prioridad de los hilos.
 *
 * \mainpage Guía del programador
 *
 * \tableofcontents
 *
 * \section sec_intro Introducción
 *
 * La librería que se describe e este documento encapsula el trabajo con hilos
 * en un objeto de fácil acceso. Además, el código se ha hecho multiplataforma,
 * de manera que aquellos proyectos que usen a nsThreads::TThread compilarán
 * sin necesidad de cambiar código tanto en Linux como Windows. Los beneficios
 * son varios. Por un lado la mantenibilidad del código fuente. Por otro, se
 * crea una capa transparente al programador de manera que actualizaciones y
 * bug-fixes no afecten, o minimicen el impacto en código que use la librería.
 *
 *  Para llevar a cabo este propósito, se ha usado el lenguaje de programación
 *  C++. Todas las comunicaciones se encapsulan en la clase TSerial. En este
 *  documento se explica la estructura interna de esta clase, así cómo la
 *  manera de usarla en sus proyectos particulares.
 *
 *  El proyecto produce una librería estática, que debe enlazarse en los
 *  proyectos que la usen.
 *
 *  \subsection sec_scope Alcance
 *
 * La librería compila tanto en Linux como en Windows, por lo que se puede usar
 * en cualquiera de estos sistemas. En ambos casos el compilador empleado es
 * g++. Para Windows se emplea MinGW.
 *
 *  \subsection sec_audience Audiencia
 *
 *  Este manual está indicado a los programadores que deseen usar esta
 *  librería. Aquí se explica cómo y qué incluir en sus archivos fuentes, y
 *  cómo usar la clase TThread.
 *
 *  \subsection sec_credits Créditos
 *
 *  Esta librería fue creada inicialmente para uso personal de su autor. Al
 *  considerarla de utilidad, se ha distribuido bajo licencia GNU. Puede ser
 *  libremente distribuida, modificada o alterada y es completamente gratuita.
 *
 *  \version v1.0
 *  \date 2013
 *  \copyright GNU Public License
 *  \author Raúl Hermoso Sánchez
 *
 *  \subsection sec_whereto ¿Dónde continuar?
 *
 *  Dónde seguir leyendo este manual depende mucho de lo que ande buscando. Si
 *  es nuevo a esta librería, le recomendamos visitar la sección
 *  \subpage pag_quick_start. Si por el contrario prefiere estudiar cómo está
 *  implementada la clase principal, le recomendamos que vaya a la sección de
 *  documentación de las clases.
 *
 *  \page pag_quick_start Comienzo rápido
 *
 *  \tableofcontents
 *
 *  La manera más rápida de ponerse ne marcha es mediante ejemplos. Hemos
 *  preparado un ejemplo sencillo pero que creemos ilustra muy bien la manera
 *  de usar la libreria en cualquier otro proyecto.
 *
 *  El codigo fuente puede verse \ref example.cpp "aquí".
 *
 */
//-----------------------------------------------------------------------------
#ifndef __THREADS_HPP__
#define __THREADS_HPP__
//-----------------------------------------------------------------------------
#ifdef WIN32
	#include <windows.h>

	#define thrd_handle HANDLE
	#define MUTEX		HANDLE
    #define thrd_mutex  void*

	#define INI_CRITICAL_SECTION(x)     InitializeCriticalSection(x)
	#define DEL_CRITICAL_SECTION(x)     DeleteCriticalSection(x)
	#define ENTER_CRITICAL_SECTION(x)   EnterCriticalSection(x)
	#define LEAVE_CRITICAL_SECTION(x)   LeaveCriticalSection(x)

	#define CREATE_MUTEX(x)    x = CreateMutex(NULL, FALSE, NULL)
	#define RELEASE_MUTEX(x)   CloseHandle(x)
	#define LOCK_MUTEX(x)      (WaitForSingleObject(x, INFINITE) == WAIT_OBJECT_0)
	#define UNLOCK_MUTEX(x)    ReleaseMutex(x)
#else
    #include <pthread.h>
    
	/** Alias para una variable de doble palabra.
	 *
	 *  - \c Linux Equivalente a <tt>unsigned int</tt> (en 32 bits)
	 *  - \c Windows Ya viene definido */
	#define DWORD       			unsigned int
	/** Alias para el descriptor de un hilo.
	 *
	 *  - \c Linux Equivalente a <tt>pthread_t</tt>
	 *  - \c Windows Equivalente a <tt>HANDLE</tt> */
    #define thrd_handle				pthread_t
	/** Alias para el descriptor de un mutex.
	 *
	 *  - \c Linux Equivalente a <tt>pthread_mutex_t</tt>
	 *  - \c Windows Equivalente a <tt>HANDLE</tt> */
    #define thrd_mutex  			pthread_mutex_t
	/** Otro alias para el descriptor de un mutex.
	 *
	 *  - \c Linux Equivalente a <tt>thrd_mutex</tt>
	 *  - \c Windows Ya viene definido */
	#define MUTEX					thrd_mutex
	/** Alias para una sección crítica.
	 *
	 *   Una sección crítica en Windows se define como una proción de código
	 * cuya ejecución debe completarse de principio a fin, sin interrupciones
	 * por parte del SO.
	 *
	 *  - \c Linux Se ha hecho equivalente a <tt>thrd_mutex</tt> aunque en
	 *  realidad sean criaturas diferentes
	 *  - \c Windows Ya viene definido */
	#define CRITICAL_SECTION	    thrd_mutex

	/** Alias para la incialización de una sección crítica.
	 *
	 *  - \c Linux Se ha hecho equivalente a <tt>pthread_mutex_init</tt> aunque
	 *  en realidad sean criaturas diferentes
	 *  - \c Windows <tt>InitializeCriticalSection</tt>
	 *
	 *  \sa ::CRITICAL_SECTION */
	#define INI_CRITICAL_SECTION(x)   pthread_mutex_init(x, NULL)
	/** Alias para liberar recursos de de una sección crítica.
	 *
	 *  - \c Linux Se ha hecho equivalente a <tt>pthread_mutex_init</tt> aunque
	 *  en realidad sean criaturas diferentes
	 *  - \c Windows <tt>DeleteCriticalSection</tt>
	 *
	 *  \sa ::CRITICAL_SECTION */
	#define DEL_CRITICAL_SECTION(x)   pthread_mutex_destroy(x)
	/** Alias para definir el comienzo de una sección crítica.
	 *
	 *  - \c Linux Se ha hecho equivalente a <tt>pthread_mutex_lock</tt> aunque
	 *  en realidad sean criaturas diferentes
	 *  - \c Windows <tt>EnterCriticalSection</tt>
	 *
	 *  \sa ::CRITICAL_SECTION */
	#define ENTER_CRITICAL_SECTION(x) (pthread_mutex_lock(x) == 0)
	/** Alias para definir el final de una sección crítica.
	 *
	 *  - \c Linux Se ha hecho equivalente a <tt>pthread_mutex_unlock</tt>
	 *  aunque en realidad sean criaturas diferentes
	 *  - \c Windows <tt>LeaveCriticalSection</tt>
	 *
	 *  \sa ::CRITICAL_SECTION */
	#define LEAVE_CRITICAL_SECTION(x) pthread_mutex_unlock(x)

	/** Alias para la creación de un mutex.
	 *
	 *  - \c Linux Se ha hecho equivalente a ::INI_CRITICAL_SECTION
	 *  - \c Windows <tt>CreateMutex</tt>
	 */
	#define CREATE_MUTEX(x)    INI_CRITICAL_SECTION(&x)
	/** Alias para la liberación de los recursos de un mutex.
	 *
	 *  - \c Linux Se ha hecho equivalente a ::DEL_CRITICAL_SECTION
	 *  - \c Windows <tt>CloseHandle</tt>
	 */
	#define RELEASE_MUTEX(x)   DEL_CRITICAL_SECTION(&x)
	/** Alias para el bloqueo de un mutex.
	 *
	 *  - \c Linux Se ha hecho equivalente a ::ENTER_CRITICAL_SECTION
	 *  - \c Windows <tt>WaitForSingleObject</tt>
	 */
	#define LOCK_MUTEX(x)      ENTER_CRITICAL_SECTION(&x)
	/** Alias para el desbloqueo de un mutex.
	 *
	 *  - \c Linux Se ha hecho equivalente a ::LEAVE_CRITICAL_SECTION
	 *  - \c Windows <tt>ReleaseMutex</tt>
	 */
	#define UNLOCK_MUTEX(x)    LEAVE_CRITICAL_SECTION(&x)

	//-- Para el cálculo de las prioridades del hilo
	/** Alias para la prioridad <i>ausente</i> de un hilo. */
	#define THREAD_PRIORITY_IDLE			1
	/** Alias para la prioridad <i>mónima</i> de un hilo. */
	#define THREAD_PRIORITY_LOWEST			2
	/** Alias para la prioridad <i>por debajo de lo normal</i> de un hilo. */
	#define THREAD_PRIORITY_BELOW_NORMAL	3
	/** Alias para la prioridad <i>normal</i> de un hilo. */
	#define THREAD_PRIORITY_NORMAL			4
	/** Alias para la prioridad <i>por encima de lo normal</i> de un hilo. */
	#define THREAD_PRIORITY_ABOVE_NORMAL	5
	/** Alias para la prioridad <i>móxima</i> de un hilo. */
	#define THREAD_PRIORITY_HIGHEST			6
	/** Alias para la prioridad <i>crítica en tiempo</i> de un hilo. */
	#define THREAD_PRIORITY_TIME_CRITICAL	7
#endif
//-----------------------------------------------------------------------------
#include <cstdlib>
#include <iostream>
//-----------------------------------------------------------------------------

/**
 * Agrupa las estructuras, clases, enumerados y demás elementos necesarios para
 * la encapsulación de un hilo.
 */
namespace nsThread
{
    //-------------------------------------------------------------------------
    class TThread;
    /** Define la forma de una función de hilo */
    typedef void (*fncThread)(TThread*, void*);
    //-------------------------------------------------------------------------

    /**
     * Encapsula la gestión y función de un hilo paralelo de ejecución.
     *
     *   Diferentes plataformas usan APIs, métodos y estructuras diferentes a
     * la hora de gestionar hilos de ejecución. Por otro lado, en todos los
     * casos, las APIs nativas están estructuradas de manera
     * <i>estructurada</i> y nunca <i>orientada a objetos</i>. TThread por un
     * lado, encapsula esta API en una clase y por otro, con la ayuda de
     * macros, defines y aliases lo hace de manera independiente de la
     * plataforma de compilación (Linux o Windows, se entiende).
     */
	class TThread
    {
    private:
    	/** Descriptor del hilo en el SO
    	 * \sa Handle()*/
        thrd_handle m_thHandle;
        /** Descriptor del mutex empleado para sincronización del hilo */
        thrd_mutex  m_tmMutex;
        /** Puntero al parámetro que recibiró la función del hilo */
        void*       m_vUserData;
        /** Puntero a la función que ejecutará el hilo */
		fncThread   m_fncUserFunc;
		/** Identificador del hilo. (Sólo necesario en Windows) */
        DWORD       m_thrdID;
        /** Flag que indica si el hilo ha sido creado */
        bool		m_bCreated;
        /** Flag que indica al hilo que debe terminar su ejecución */
        bool        m_bTerminate;
        /** Flag para suspender la ejecución del hilo */
		bool        m_bSuspend;
        
		/** Función principal del hilo. */
        #ifdef WIN32
			static DWORD WINAPI mainThreadFunc(LPVOID Owner);
		#else
			static void* mainThreadFunc(void* Owner);
		#endif
		/**  Traduce la prioridad del hilo a la API del SO corresponiente */
		unsigned int translatePriority(unsigned int& DesiredPriority);

	protected:
    public:
		/** Constructor del hilo */
        TThread();
		/** Destructor del hilo */
        virtual ~TThread();

        /** Crea el hilo de ejecución */
		bool create(fncThread MainFunction, void* UserData, bool bSuspended = false);
		/** Libera los recursos del hilo */
		void destroy(unsigned int uiWaitTime = 1000);
        /** Termina el hilo de ejecución */
        bool terminate();
        /** Suspende la ejecución del hilo */
        bool suspend();
        /** Reanuda la ejecución del hilo */
		bool resume();

		/** Lee la prioridad actual del hilo */
		unsigned int priority();
		/** Asigna una nueva prioridad al hilo */
		void 		 priority(unsigned int uiPriority);

		/** Espera el fin de la ejecución del hilo */
		bool waitFor(unsigned int uiWaitTime = 0);

		/** Permite cambiar la función del hilo.
		 *
		 * \param [in] MainFunction Puntero a la nueva función que ejecutará el
		 * hilo
		 *
		 * Mediante este método se puede cambiar la función que el hilo
		 * ejecuta en <i>caliente</i>. El parámetro MainFunction apunta a la
		 * nueva función que se desea ejacutar.
		 *
		 * \return nada
		 *
		 * \sa mainThreadFunc() , m_fncUserFunc*/
		inline void changeThreadFunction(fncThread MainFunction)
										{ m_fncUserFunc = MainFunction; }

		/** Devuelve el descriptor que el SO ha asignado al hilo. (sólo
		 * lectura).
		 * \sa m_thHandle */
        inline thrd_handle handle() 	  { return m_thHandle;   }
        /** Indica si el hilo ha terminado su ejecución. (sólo lectura).
         * \sa m_bTerminate, terminate(), suspend(), resume(), */
        inline bool        isTerminated() { return m_bTerminate; }
        /** Indica si el hilo está suspendido. (sólo lectura).
         * \sa m_bSuspend, terminate(), suspend(), resume(), */
		inline bool        isSuspended()  { return m_bSuspend;   }
        /** Indica si el hilo está creado. (sólo lectura).
         * \sa m_bSuspend, terminate(), suspend(), resume(), */
		inline bool        isCreated()    { return m_bCreated;   }
	};
    //-------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------

/**
 * \example example.cpp
 *
 * Este ejemplo muestra como instanciar un objeto de la clase TThread, crear
 * una función de hilo, terminarlo y esperar a que concluya. El programa crea
 * un hilo, y un contador. El hilo irá incrementando el valor del contador
 * hasta llegar a 10, pintando su valor en cada increment. Al llegar a 10
 * terminará su ejecución. Mientras, el hilo principal creará otro contador.
 * Cuando llegue también a 10, esperará a que termine la ejecución del hilo
 * para por fin terminar.
 *
 * El ejemplo muestra el uso de las funciones de creación de la función hilo,
 * la creación de la función callback del hilo, y el uso de las funciones de
 * terminación y espera de hilo. Además, se aprende el uso de la variable
 * "void*" de la función callback del hilo.
 */

