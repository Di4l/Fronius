//-----------------------------------------------------------------------------
/** \file
 *
 */
//-----------------------------------------------------------------------------
#include <cstring> //jm
//-----------------------------------------------------------------------------
#include "threads.hpp"
//-----------------------------------------------------------------------------
#define ERROR_MAX_LEN   256
#define ERROR_WHY_MAX   1024
//-----------------------------------------------------------------------------

namespace nsError
{
	class TError
	{
	public:
		TError(const char* what, const char* why, const unsigned int line, const char* file)
		{
			std::cout << file << " [" << line << "]: " << what << " -> " << why << std::endl;
		}
	};
}
//-----------------------------------------------------------------------------
#ifndef __BCPLUSPLUS__
using namespace nsThread;
#else
namespace nsThread
{
#endif
//-----------------------------------------------------------------------------

/**
 *   Inicializa los parámetros del hilo. Los valores por defecto son 0, NULL o
 * false, segón corresponda por el tipo de dato.
 *
 * \return nada
 *
 * \sa ~TThread()
 */
TThread::TThread()
{
#ifdef WIN32
    m_thHandle    = NULL;
    m_tmMutex     = NULL;
#endif
    m_vUserData   = NULL;
    m_fncUserFunc = NULL;
    m_thrdID      = 0;
    m_bCreated	  = false;
    m_bTerminate  = false;
    m_bSuspend    = false;
}
//-----------------------------------------------------------------------------

/**
 *   Libera los recursos empleados por el hilo. Si el hilo está en ejecución,
 * le indica terminar (terminate()), espera a que lo haga (waitFor()) y
 * finalmente destruye el hilo y los recursos empleados.
 *
 * \return nada
 *
 * \sa TThread(), destroy()
 */
TThread::~TThread()
{
	destroy();
}
//-----------------------------------------------------------------------------

/**
 * \param [in] Owner Objeto TThread que creó el hilo
 *
 *   La función principal del hilo se encapsula en un mótodo privado y comón a
 * todas las instancias TThread. Esto nos permite llevar un control del bucle
 * de ejecución del hilo, encapsulando cuóndo detenerlo o suspenderlo o incluso
 * cambiar la función <i>usuario</i> del hilo.
 *
 *   El parámetro <i>Owner</i> es un puntero a la instancia TThread que creó el
 * hilo. Al tratarse de un mótodo \c static, es comón a todas las instacnias
 * TThread, por lo que el parámetro <i>Owner</i> es indispensable para
 * distinguir de entre todas a la que se hace referencia.
 *
 *   MainThreadFunc(), tras unas comprobaciones previas, crea un bucle del que
 * no sale hasta que se le indique al hilo terminar su ejecución mediante el
 * mótodo terminate(). Dentro del bucle, se llama cada vez a la función que el
 * usuario ha especificado en el mótodo create(). Esta función puede ser
 * cambiada a posteriori mediante el mótodo changeThreadFunction().
 *
 * \exception nsError::TError Si el <i>Owner</i> es NULL.
 *
 * \return 0 o NULL cuando sale de la función
 *
 * \sa create(), terminate(), suspend(), resume(), waitFor()
 */
#ifdef WIN32
DWORD WINAPI TThread::mainThreadFunc(LPVOID Owner)
#else
void* TThread::mainThreadFunc(void* Owner)
#endif
{
	if(NULL == Owner)
	{
		return 0;
/*
		char c_why[ERROR_WHY_MAX];
		sprintf(c_why, "Puntero a objeto TThread es nulo");
		throw nsError::TError("Puntero nulo", c_why, __LINE__, __FILE__);
*/
	}

	TThread* thread = (TThread*)Owner;
	
	if(NULL != thread->m_fncUserFunc)
	{
		//-- Bucle del hilo principal
		while(!thread->m_bTerminate)
		{
			//-- Tenemos que suspender la ejecucion?
			#ifndef WIN32
			pthread_mutex_lock(&thread->m_tmMutex);
			pthread_mutex_unlock(&thread->m_tmMutex);
			#endif
			//-- Llamamos a la funcion del hilo de usuario
			thread->m_fncUserFunc(thread, thread->m_vUserData);
		}
	}
	thread->m_bCreated = false;
    return 0;
}
//-----------------------------------------------------------------------------

/**
 * \param [in] DesiredPriority Prioridad deseada para el hilo
 *
 *   Cada SO tiene una manera diferente de establecer las prioridades. Este
 * mótodo traduce los niveles de prioridad definidos para la PSI en el valor
 * correspondiente de acuerdo al SO en el que se está ejecutando.
 *
 * \return Nueva prioridad traducida al intervalo aceptado por el SO
 * correspondiente.
 *
 * \sa priority(), priority(unsigned int)
 */
unsigned int TThread::translatePriority(unsigned int& DesiredPriority)
{
	#ifdef WIN32
		return DesiredPriority;
	#else
		int max = sched_get_priority_max(SCHED_OTHER);
		int min = sched_get_priority_min(SCHED_OTHER);

		return min + ((DesiredPriority * (max - min)) / THREAD_PRIORITY_TIME_CRITICAL);
	#endif
}
//-----------------------------------------------------------------------------

/**
 * \param [in] MainFunction Función de ejecución del hilo
 * \param [in] UserData Puntero a un valor de uso exclusivo para el programador
 * \param [in] bSuspended Flag para indicar si el hilo debe crearse suspendido
 *
 *   Crea el hilo que ejecutará la función <i>MainFunction</i> (ver
 * MainThreadFunc()). Si <i>bSuspended</i> es \c true el hilo se crea en estado
 * suspendido y no comenzará a ejecutarse hasta que se llame a resume().
 *
 *   <i>UserData</i> es un puntero a una variable que se pasará como parámetro
 * a la función <i>MainFunction</i> cada vez que sea llamada. Su uso está
 * reservado al programador, que puede emplearla de la manera que mejor le
 * convenga.
 *
 * \exception nsError::TError Si el SO ha producido un error en la creación del
 * hilo
 *
 * \return \c true si el hilo se ha credo con óxito, \c false en cualquier otro
 * caso
 *
 * \sa terminate(), suspend(), resume(), waitFor()
 */
bool TThread::create(fncThread MainFunction, void* UserData, bool bSuspended)
{
	if(m_bCreated)
		return false;

	m_bTerminate  = false;
	m_bSuspend    = bSuspended;
	m_vUserData   = UserData;
	m_fncUserFunc = MainFunction;

#ifdef WIN32
	m_thHandle = CreateThread(NULL, 0, mainThreadFunc, (void*)this,
								m_bSuspend ? CREATE_SUSPENDED : 0, &m_thrdID);

	m_bCreated = (m_thHandle != NULL);
	if(NULL == m_thHandle)
	{
/*
		char c_why[ERROR_WHY_MAX];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR)c_why, ERROR_WHY_MAX, NULL);
		throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
*/
	}

	return m_thHandle != NULL;
#else
	pthread_attr_t attr;    //-- Atributos del hilo

	//-- Haremos el hilo "joinable" explicitamente
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	//-- Se inicializa el mutex
	pthread_mutex_init(&m_tmMutex, NULL);

	//-- Creamos el hilo
	int rtn = pthread_create(&m_thHandle, &attr, mainThreadFunc, (void*)this);
	m_bCreated = rtn ? false : true;
	if(rtn)
	{
		char c_why[ERROR_WHY_MAX];
		strerror_r(rtn, c_why, ERROR_WHY_MAX);
		throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
	}
    //-- Desturimos los atributos. No los usaremos mas
    pthread_attr_destroy(&attr);
	return m_bCreated;
#endif
}
//-----------------------------------------------------------------------------

void TThread::destroy(unsigned int uiWaitTime)
{
    if(m_bCreated)
    {
		terminate();
		waitFor(uiWaitTime);
	#ifdef WIN32
		CloseHandle(m_thHandle);
	#else
		pthread_mutex_destroy(&m_tmMutex);
	#endif
	}
#ifdef WIN32
	m_thHandle    = NULL;
	m_tmMutex     = NULL;
#endif
	m_vUserData   = NULL;
	m_fncUserFunc = NULL;
	m_thrdID      = 0;
	m_bCreated    = false;
	m_bTerminate  = false;
	m_bSuspend    = false;
}
//-----------------------------------------------------------------------------

/**
 * Activa el flag de terminación de hilo. En la siguiente pasada del bucle en
 * MainThreadFunc() se saldró del bucle de ejecución y se terminaró el hilo.
 * Para evitar entrar en un deadlock, una vez activado el flag, se llama al
 * mótodo resume() para despertar el hilo en caso de que estuviera en suspenso.
 *
 * \return \c true si el flag se activa correctamente, \c false si no hay hilo
 * creado.
 *
 * \sa create(), suspend(), resume(), waitFor()
 */
bool TThread::terminate()
{
	if(!m_bCreated)
		return false;

	m_bTerminate = true;
    //-- Si esta suspendido el hilo, lo reanudamos
    resume();
	return true;
}
//-----------------------------------------------------------------------------

/**
 *   Suspende la ejecución del hilo y activa el flag de <i>suspensión</i>. Si
 * el hilo ya está suspendido o está marcado para terminar, este mótodo no hace
 * nada y devuelve \c false.
 *
 * \exception nsError::TError Si el SO lanza un error al intentar suspender el
 * hilo
 *
 * \return \c true si el hilo se suspende con óxito, \c false si no se puede
 * detener o el hilo ya está suspendido o marcado para terminar su ejecución
 *
 * \sa resume(), waitFor()
 */
bool TThread::suspend()
{
	if(m_bSuspend || m_bTerminate)
		return false;

	bool b_rtn = true;
	if(m_bCreated)
	{
		m_bSuspend = true;
	#ifdef WIN32
		if(0xFFFFFFFF == SuspendThread(m_thHandle))
		{
			b_rtn = false;
/*
			char c_why[ERROR_WHY_MAX];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR)c_why, ERROR_WHY_MAX, NULL);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
*/
		}
	#else
		int i_rtn = pthread_mutex_lock(&m_tmMutex);
		b_rtn = i_rtn ? false : true;
		if(i_rtn)
		{
			char c_why[ERROR_WHY_MAX];
			strerror_r(i_rtn, c_why, ERROR_WHY_MAX);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
		}
	#endif
	}

	return b_rtn;
}
//-----------------------------------------------------------------------------

/**
 *    Reanuda la ejecución del hilo sólo si estuviera en suspensión. En caso de
 *  ya estar en ejecución el mótodo devuelve \c false.
 *
 * \exception nsError::TError Si el SO lanza un error al intentar desperar el
 * hilo
 *
 * \return \c true si el hilo se reanuda con óxito, \c false si no se puede
 * reanudar o el hilo ya estuviera en ejecución.
 *
 * \sa suspend(), waitFor()
 */
bool TThread::resume()
{
	if(!m_bSuspend)
		return false;

	bool b_rtn = true;
	if(m_bCreated)
	{
		m_bSuspend = false;
	#ifdef WIN32
		if(0xFFFFFFFF == ResumeThread(m_thHandle))
		{
			b_rtn = false;
/*
			char c_why[ERROR_WHY_MAX];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR)c_why, ERROR_WHY_MAX, NULL);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
*/
		}
	#else
		int i_rtn = pthread_mutex_unlock(&m_tmMutex);
		b_rtn = i_rtn ? false : true;
		if(i_rtn)
		{
			char c_why[ERROR_WHY_MAX];
			strerror_r(i_rtn, c_why, ERROR_WHY_MAX);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
		}
	#endif
	}

	return b_rtn;
}
//-----------------------------------------------------------------------------

/**
 *   Devuelve la prioridad actual de hilo, de acuerdo al baremo definido por
 * los alias de la PSI (ver ::THREAD_PRIORITY_IDLE hasta
 * ::THREAD_PRIORITY_TIME_CRITICAL).
 *
 * \exception nsError::TError Si el SI lanza un error al intentar obtener la
 * prioridad del hilo
 *
 * \return nómero indicando la prioridad del hilo o \c 0 en caso de no poder
 * obtener dicho valor.
 *
 * \sa translatePriority(), priority(unsigned int)
 */
unsigned int TThread::priority()
{
	unsigned int ui_ret = 0;
	if(m_bCreated)
	{
	#ifdef WIN32
		ui_ret = GetThreadPriority(m_thHandle);
		if(THREAD_PRIORITY_ERROR_RETURN == ui_ret)
		{
/*
			char c_why[ERROR_WHY_MAX];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR)c_why, ERROR_WHY_MAX, NULL);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
*/
		}
	#else
		sched_param param;
		int			policy;
		int 		ret = pthread_getschedparam(m_thHandle, &policy, &param);
		if(ret)
		{
			char c_why[ERROR_WHY_MAX];
			strerror_r(ret, c_why, ERROR_WHY_MAX);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
		}
		ui_ret = translatePriority((unsigned int&)param.sched_priority);
	#endif
	}
	return ui_ret;
}
//-----------------------------------------------------------------------------

/**
 * \param [in] uiPriority
 *
 *   Cambia la prioridad del hilo al valor especificado por <i>uiPriority</i>.
 * El parámetro debe corresponderse con alguno de los valores especificados por
 * los defines THREAD_PRIORITY_xxx.
 *
 * \exception nsError::TError Si el SO lanza un error al intentar cambiar la
 * prioridad del hilo
 *
 * \return nada
 *
 * \sa translatePriority(), priority()
 */
void TThread::priority(unsigned int uiPriority)
{
	if(m_bCreated)
	{
	#ifdef WIN32
		if(!SetThreadPriority(m_thHandle, uiPriority))
		{
/*
			char c_why[ERROR_WHY_MAX];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR)c_why, ERROR_WHY_MAX, NULL);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
*/
		}
	#else
		sched_param param;
		int			policy;
		int 		ret = pthread_getschedparam(m_thHandle, &policy, &param);
		if(ret)
		{
			char c_why[ERROR_WHY_MAX];
			strerror_r(ret, c_why, ERROR_WHY_MAX);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
		}
		param.sched_priority = translatePriority(uiPriority);
		policy				 = SCHED_OTHER;
		ret = pthread_setschedparam(m_thHandle, policy, &param);
		if(ret)
		{
			char c_why[ERROR_WHY_MAX];
			strerror_r(ret, c_why, ERROR_WHY_MAX);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
		}
	#endif
    }
}
//-----------------------------------------------------------------------------

/**
 *   Esta función no retorna hasta que el hilo ha terminado su ejecución
 * resultando en una espera para el hilo que realiza esta llamada. La espera se
 * realiza de manera eficiente, mediante llamadas a la API del SO para no
 * cosumir tiempo de ejecución. No es, por tanto, una espera activa.
 *
 * \exception nsError::TError si el SO lanza un error en la llamada de espera
 * al hilo.
 *
 * \return nada
 *
 * \sa create(), terminate(), suspend(), resume()
 */
bool TThread::waitFor(unsigned int uiWaitTime)
{
	bool b_ret = false;

	if(m_bCreated)
	{
		//-- Desbloqueamos el hilo no sea que este bloqueado
		resume();
	#ifdef WIN32
		DWORD dw_wait = WaitForSingleObject(m_thHandle, uiWaitTime ? uiWaitTime : INFINITE);
		switch(dw_wait)
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			b_ret = true;
			break;

		case WAIT_TIMEOUT:
		case WAIT_FAILED:
		default:
			b_ret = false;
			break;
		}
/*
			char c_why[ERROR_WHY_MAX];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR)c_why, ERROR_WHY_MAX, NULL);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
*/
	#else
		int i_rtn = pthread_join(m_thHandle, NULL);
		if(i_rtn)
		{
			char c_why[ERROR_WHY_MAX];
			strerror_r(i_rtn, c_why, ERROR_WHY_MAX);
			throw nsError::TError("Error de hilo", c_why, __LINE__, __FILE__);
		}
		else
		{
			b_ret = true;
		}
	#endif
	}

	return b_ret;
}
//-----------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
}
#endif
//-----------------------------------------------------------------------------

