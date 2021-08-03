/*
 * main.cpp
 *
 *  Created on: 10 abr. 2019
 *      Author: Administrador
 */

#if defined(__arm__) && !defined(__linux_build__)
#define __linux_build__
#endif


//-----------------------------------------------------------------------------
#include <cstdio>			//-- For sscanf, sprintf
#include <cstring>			//-- For memset
//#include <cmath>			//-- for pow
#include <math.h>			//-- for pow
#include <iostream>
#include <queue>
#include <chrono>
#include <ctime>
#include <fstream>
#include <random>
#include <functional>
#include <limits>
#include "mb_config.h"
#include "mqtt_domoticz.h"
#ifdef __linux_build__
#include <wiringPi.h>
#include <softPwm.h>
#include <wiringSerial.h>
#endif
//-----------------------------------------------------------------------------
#ifdef __linux_build__
#define PWM_WPI_PIN		1
#define DHT_PIN			7
#define DHT_MAXTIMINGS	85
#endif
//-----------------------------------------------------------------------------

#define LOG_STREAM(os, txt)		os << txt
#define LOG(txt)			LOG_STREAM(std::cout, txt)
#define LOG_IF_STREAM(c, os, txt)	if(c) os << txt
#define LOG_IF(c, txt)			LOG_IF_STREAM(c, std::cout, txt)
#define SHOW_STREAM(os, txt)		if(p.verbose) os << txt
#define SHOW(txt)			SHOW_STREAM(std::cout, txt)
#define SHOW_P(prm,txt)			if(prm.verbose) std::cout << txt
#define SHOW_IF_STREAM(c, os, txt)	if((c) and p.verbose) os << txt
#define SHOW_IF(c, txt)			SHOW_IF_STREAM(c, std::cout, txt)
//-----------------------------------------------------------------------------
#define ABS(x)	(x < 0 ? -x : x)
//-----------------------------------------------------------------------------

void setPWM(float fPer)
{
#ifdef __linux_build__
	//-- The PWM is not linear. We have found an empirical law that lets us turn
	//   desired power % to pwm.
	float f_pwm = 2.19 * pow(fPer, 3.0) - 3.147 * pow(fPer, 2.0) + 1.9061 * fPer;
	int val = int(f_pwm * 1024);
	val = (val > 1024 ? 1024 : (val < 0 ? 0 : val));
	pwmWrite(PWM_WPI_PIN, val);
#endif
}
//-----------------------------------------------------------------------------

template<typename T>
class TMovingMean
{
private:
	uint32_t      m_cnt;
	uint32_t      m_iSpike;
	std::queue<T> m_numbers;
	T             m_spkThreshold;

	void evaluateValue(T _new);

protected:
	T m_accum;

public:
	TMovingMean(uint32_t navgs, T _thrs = 0) : m_cnt(navgs), m_iSpike(0), m_spkThreshold(_thrs), m_accum(0.0) {}

	uint32_t numAverages() { return m_cnt;            }
	size_t   size()        { return m_numbers.size(); }

	void clear()          { while(m_numbers.size() > 0) m_numbers.pop(); m_accum = 0; }
	bool isPosibleSpike() { return m_iSpike > 0; }

	T mean()  { return m_numbers.size() ? m_accum / m_numbers.size() : m_accum; }
	T trend() { return last() - mean();  }
	T last()  { return m_numbers.size() ? m_numbers.back() : 0; }
	T accum() { return m_accum; }

	T add(T tNewEntry);
};
//-----------------------------------------------------------------------------

template<typename T>
void TMovingMean<T>::evaluateValue(T _new)
{
	if(m_spkThreshold == 0)
		return;

	static T _old = 0;
	T delta = (_new - _old);
	_old = _new;

	if(delta > m_spkThreshold)
		m_iSpike = m_numbers.size() / 2;
	else if(-delta > m_spkThreshold)
		m_iSpike = 0;
	else if(m_iSpike > 0)
		--m_iSpike;
}
//-----------------------------------------------------------------------------

template<typename T>
T TMovingMean<T>::add(T tNewEntry)
{
	m_accum += tNewEntry;
	m_numbers.push(tNewEntry);

	if(m_numbers.size() > m_cnt)
	{
		m_accum -= m_numbers.front();
		m_numbers.pop();
	}

	T new_mean = mean();
	evaluateValue(new_mean);

	return new_mean;
}
//-----------------------------------------------------------------------------

template <typename T>
class PID
{
private:
	typedef std::numeric_limits<T> limits;

	T m_p;	   //-- Proportional
	T m_i;	   //-- Integral
	T m_d;	   //-- Derivative
	T m_min;
	T m_max;
	T m_accum;
	TMovingMean<T> m_deriv = TMovingMean<T>(10);

	T accumulate(T _error, T _dt)
	{
		//-- _dt has been made sure to be greater than 0
		m_accum += (_error * _dt);
		if(m_min > -limits::max())
			m_accum = m_accum < m_min ? m_min : m_accum;
		if(m_max < limits::max())
			m_accum = m_accum > m_max ? m_max : m_accum;
		return m_accum;
	}

	T differentiate(T _error, T _dt)
	{
		//-- _dt has been made sure to be greater than 0
		T ret = (_error - m_deriv.mean()) / _dt;
		m_deriv.add(_error);
		return ret;
	}

public:
	PID(T p, T i, T d) : m_p(p), m_i(i), m_d(d), m_min(-limits::max()), m_max(limits::max()), m_accum(0.0) {}
	PID(T p, T i, T d, T mn, T mx) : m_p(p), m_i(i), m_d(d), m_min(mn/i), m_max(mx/i), m_accum(0.0) {}
	PID(T p, T i, T d, T lim) : m_p(p), m_i(i), m_d(d), m_min(-lim/i), m_max(lim/i), m_accum(0.0) {}

	inline T& p()   { return m_p;   }
	inline T& i()   { return m_i;   }
	inline T& d()   { return m_d;   }
	inline T  min() { return m_min; }
	inline T  max() { return m_max; }

	T    calculate(T _error, T _dt = 0.0);
	void clear() { m_accum = 0.0; m_deriv.clear(); }
};
//-----------------------------------------------------------------------------

template <typename T>
T PID<T>::calculate(T _error, T _dt)
{
	T dt  = _dt > 0.0 ? _dt : 1.0;
	T acc = accumulate(_error, dt);
	T drv = differentiate(_error, dt);
//std::cout << "p=" << (m_p * _error) << " i=" << (m_i * acc) << " d=" << (m_d * drv) << std::endl;
	return (m_p * _error) + (m_i * acc) + (m_d * drv);
}
//-----------------------------------------------------------------------------

template <typename T>
std::ostream& operator <<(std::ostream& os, PID<T>& pid)
{
	return os << "[" << pid.p() << ";" << pid.i() << ";" << pid.d() << "]";
}
//-----------------------------------------------------------------------------

struct elec_info
{
	float volts;
	float current;
	float power;
};

struct env_info
{
	float temperature;
	float humidity;
	int   status;	// 0 - normal, 1 - comfortable, 2 - dry, 3 - wet
};

elec_info dc_s1, dc_s2, dc, ac, dcsf;
uint16_t  dcst_s1, dcst_s2, dcst;
bool      ecpconn;

#define PWR_GOAL		50.0
#define TEMP_MIN		19.0
#define TEMP_MAX		24.0
#define TIME_TO_RUN		60		//-- seconds

#define NUM_AVGS		10
//-----------------------------------------------------------------------------

struct MqttConfig
{
	std::string id;
	std::string host;
	uint32_t    port;
};
//-----------------------------------------------------------------------------

struct SParams
{
	bool        hasChanged;
	std::string entryLine;
	std::string logFile;
	std::string binFile;
	float       powerGoal;
	float       finalPWM;
	int         timeToRun;
	bool        forceGoal;
	float       tempMin;
	float       tempMax;
	bool        verbose;
	bool        isActive;
	MqttConfig  mqtt;

	SParams() : hasChanged(false), entryLine(""), logFile("")
		, binFile(""), powerGoal(0.0), finalPWM(0.0), timeToRun(15)
		, forceGoal(false), tempMin(0.0), tempMax(0.0), verbose(false)
		, isActive(false), mqtt( {"", "", 0} ) {}
};
//-----------------------------------------------------------------------------
mb_ret getResult(nsModbus::MB_Std_Message* mssg);
void   onReceive(nsModbus::TModbus* mb, nsModbus::MB_Std_Message* mssg, void* usr);
void   parseCommandLine(int argc, char** argv, SParams& p);
void   showCommandLineHelp(SParams& p);
void   readDht11(env_info& ei);
//-----------------------------------------------------------------------------

void saveTime(std::ostream& os)
{
	time_t t_now   = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm*  t_info    = std::localtime(&t_now);

	os << t_info->tm_hour << ":" << t_info->tm_min << ":" << t_info->tm_sec << std::endl;
}
//-----------------------------------------------------------------------------

bool hasDaytimeChanged(bool& bday)
{
	time_t t_now   = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm*  t_info    = std::localtime(&t_now);
	bool b_was_day = bday;

	bday = t_info->tm_hour < 22 and t_info->tm_hour > 11;
	return bday != b_was_day;
}
//-----------------------------------------------------------------------------

SParams prg_params;

#define MQTT_AC_POWER		0
#define MQTT_DC_POWER		1
#define MQTT_TEMP_HUMID		2
#define MQTT_PWM		3
#define MQTT_T_SEL		4
#define MQTT_PWR_GOAL		5
#define MQTT_ACTIVE		6
#define MQTT_DC_VOLTS           7
//-----------------------------------------------------------------------------

void onMqttMessage(const char* _tpc, const char* _cmd)
{
	std::string str_cmd = std::string(_cmd);
	size_t      pos;

	SHOW_P(prg_params, "Message on " << _tpc << ": '" << _cmd << "'" << std::endl);

	prg_params.hasChanged = true;
	if((pos = str_cmd.find("--wake-up")) != std::string::npos)
		prg_params.isActive = true;
	else if((pos = str_cmd.find("--sleep")) != std::string::npos)
		prg_params.isActive = false;
	else if((pos = str_cmd.find("-p")) != std::string::npos)
		sscanf(&_cmd[pos + 2], "%f", &prg_params.powerGoal);
	else if((pos = str_cmd.find("-ts")) != std::string::npos)
	{
		float tsel;
		sscanf(&_cmd[pos + 3], "%f", &tsel);
		prg_params.tempMin = tsel - 1.0;
		prg_params.tempMax = tsel + 1.0;
	}
	else if((pos = str_cmd.find("-fg")) != std::string::npos)
	{
		int aux;
		sscanf(&_cmd[pos + 3], "%d", &aux);
		prg_params.forceGoal = aux;
	}
	else
		prg_params.hasChanged = false;
}
//-----------------------------------------------------------------------------

void syncMqtt(MqttDomoticz* domo, const char* arr, SParams& par)
{
	if(domo and arr and par.hasChanged)
	{
		SHOW_P(par, "MQTT synching!" << std::endl);
		int   msg_len = 512;
		char  c_aux[1024], mqtt_mssg[1024];
		float t_sel = (par.tempMax + par.tempMin) / 2.0f;

		sprintf(c_aux, "%s", &arr[MQTT_T_SEL * msg_len]);
		sprintf(mqtt_mssg, c_aux, t_sel);
		domo->sendMessage(mqtt_mssg);

		sprintf(c_aux, "%s", &arr[MQTT_PWR_GOAL * msg_len]);
		sprintf(mqtt_mssg, c_aux, par.powerGoal);
		domo->sendMessage(mqtt_mssg);

		sprintf(c_aux, "%s", &arr[MQTT_ACTIVE * msg_len]);
		sprintf(mqtt_mssg, c_aux, (par.isActive ? 1 : 0));
		domo->sendMessage(mqtt_mssg);

		par.hasChanged = false;
	}
}
//-----------------------------------------------------------------------------

int main(int argc, char** argv)
{
#ifdef __linux_build__
	wiringPiSetup();
	pinMode(PWM_WPI_PIN, PWM_OUTPUT);
#endif
	setPWM(0);

	char domoticz_mssgs[][512] =
	{
		"AC_POWER:%0.1f",		// AC Power (mean)
		"DC_POWER:%0.1f",		// DC Power (mean)
		"TEMP_HUMID:%0.1f;%0.1f;%d",	// Temperature+Humidity
		"PWM:%0.1f",			// PWM
		"TSEL:%0.1f",			// TEMP. GOAL
		"POWER_GOAL:%0.0f",		// POWER GOAL
		"ON_OFF:%d",				// ACTIVE
		"DC_VOLTS:%01.f"		// DC Voltage (mean)
	};
//	std::default_random_engine gen;
//	std::normal_distribution<float> dist(200.0, 50.0);
//	auto dice = std::bind(dist, gen);

	typedef std::chrono::milliseconds ms;

	SParams& p = prg_params;
	parseCommandLine(argc, argv, p);

	std::ofstream o_file, o_bin;
	char          mqtt_mssg[1024], c_aux[1024];
	MqttDomoticz* domoticz   = new MqttDomoticz(p.mqtt.id.c_str(), p.mqtt.host.c_str(), p.mqtt.port);
	bool          b_log_file = p.logFile.size() > 0;
	bool          b_log_bin  = p.binFile.size() > 0;
	float         goal       = 0.0;
	float         acdcdiff   = 0.0;
	float         pwm        = 0.0;
	float         err_pwr    = 0.0;
	float         app_pwr    = 0.0;
//	float         lst_pwr    = 0.0;
	bool          b_daytime  = false;
	bool          b_send_pwm = false;
	env_info      env;

//	TMovingMean<float> mm_dcpwr1(NUM_AVGS), mm_dcpwr2(NUM_AVGS);
	TMovingMean<float> mm_acpwr(NUM_AVGS, 80.0), mm_dcpwr(NUM_AVGS), mm_dcvolts(NUM_AVGS);
//	TMovingMean<float> mm_acerr(20);

	auto  t_beg = std::chrono::system_clock::now();
	auto  t_now = t_beg;
	auto  t_lst = t_now;
	float dt    = 0.0;
	float dtbin = 0.0;
	float dp    = 0.0;
	float run_s = 0.0;
	float pid_scl = 10000.0;
//	float avg_err = 0.0;
//	PID<float> pid(0.15, 0.075, 0.1, 50000.0);
//	PID<float> pid(1.3, 1.0, 1.4, -1.0 * pid_scl, 1.5 * pid_scl);
//		1.4, 0.5, 1.4
	PID<float> pid(1.4, 0.45, 1.5, -1.5 * pid_scl, 1.5 * pid_scl);
	float num_pid = 0.0;

	nsModbus::TModbus modbus;
	modbus.onReceive = onReceive;

	domoticz->onMessage = onMqttMessage;

	memset(&env, 0, sizeof(env_info));

	modbus.open("192.168.1.18");
	if(b_log_file)
	{
		o_file.open(p.logFile);
		o_file << pid << std::endl;
		saveTime(o_file);
		o_file << "dt;time_from_start;ac_mean;dc_mean;goal;pwm;dpwm" << std::endl;
	}
	if(b_log_bin)
	{
		o_bin.open(p.binFile, std::ostream::binary);
		o_bin << pid.p() << pid.i() << pid.d();
	}

	//-- Scale factors only may change after a firmware update
	readInverter(modbus, INVADDR_DCSF_CURRENT, dcsf.current);
	readInverter(modbus, INVADDR_DCSF_VOLTAGE, dcsf.volts);
	readInverter(modbus, INVADDR_DCSF_POWER,   dcsf.power);

	SHOW(dcsf.current  << "A" << std::endl
	     << dcsf.volts << "V" << std::endl
	     << dcsf.power << "W" << std::endl);

	//-- Calculate if it is now daytime or nightime to set the power goal
	hasDaytimeChanged(b_daytime);
	readDht11(env);
	uint8_t cnt = 20;
	while(!env.temperature and --cnt)	//-- Sometimes the function does not read correctly
	{
		readDht11(env);
		#ifdef __linux_build__
		delay(100);
		#endif
	}
	if(b_daytime)
		goal = p.forceGoal ? p.powerGoal : (ecpconn ? PWR_GOAL : 0.0);
	else
		goal = env.temperature < p.tempMin ? p.powerGoal : 0.0;

	SHOW("Run for " << p.timeToRun << " seconds" << std::endl
	     << "It is " << (b_daytime ? "DAYTIME" : "NIGHTIME") << std::endl
	     << "Temperature range (C): " << p.tempMin << " - " << p.tempMax << std::endl
	     << "Room temperature (C) " << env.temperature << std::endl
	     << "Power goal (W): " << goal << std::endl);
	SHOW_IF(p.forceGoal, "Force power goal" << std::endl);
	SHOW_IF(b_log_file,  "Output data file to '" << p.logFile << "'" << std::endl);
	SHOW_IF(b_log_bin,   "Output binary file to '" << p.binFile << "'" << std::endl);

	run_s = (std::chrono::duration_cast<ms>(t_now - t_beg)).count() / 1000.0;
	while(!p.timeToRun or (run_s < p.timeToRun))
	{
		dc_s1.power = dc_s2.power = dc.power = 0.0;
		dc.volts    = 0.0;

		readMeter(modbus, METADDR_AC_POWER,        ac.power);
		readInverter(modbus, INVADDR_ECPCONN,      ecpconn);
		readInverter(modbus, INVADDR_OPERATING_STATE, dcst);
		if(dcst == 4)
		{
			readInverter(modbus, INVADDR_DC_POWER, dc.power);
			readInverter(modbus, INVADDR_DC_VOLTAGE_S1, dc_s1.volts);
			readInverter(modbus, INVADDR_DC_VOLTAGE_S2, dc_s2.volts);
		}
		readDht11(env);		//-- Read temperature (and humidity). Sensor is DHT11

		mm_dcvolts.add((dc_s1.volts + dc_s2.volts) * dcsf.volts);
		mm_dcpwr.add(dc.power);
		app_pwr  = mm_acpwr.add(ac.power);
//		app_pwr  = app_pwr < 0.0 ? 0.0 : app_pwr;
		acdcdiff = app_pwr - mm_dcpwr.mean();
		t_now    = std::chrono::system_clock::now();
		run_s    = (std::chrono::duration_cast<ms>(t_now - t_beg)).count() / 1000.0;
		dt       = (std::chrono::duration_cast<ms>(t_now - t_lst)).count() / 1000.0;
		dtbin   += dt;

//		if(acdcdiff < 10.0)
//			mm_accons.calcNext(mm_acpwr.mean());

		SHOW_IF(hasDaytimeChanged(b_daytime), "Changed to " << (b_daytime ? "DAYTIME" : "NIGHTTIME") << " tarif" << std::endl);
		if(b_daytime)
		{
			float prev = goal;
			goal = p.forceGoal ? p.powerGoal : (acdcdiff < 10.0 ? PWR_GOAL : 0.0);
//			goal = p.forceGoal ? p.forceGoal :((1.0 - pwm) * 50.0 + PWR_GOAL);
			SHOW_IF(goal != prev, "ECP is " << (ecpconn ? "ON" : "OFF")
					<< ", power diff is " << acdcdiff
					<< "W. Changing power goal to "
					<< goal << "W" << std::endl);
		}
		else
		{
			float prev = goal;
			goal = (p.forceGoal or env.temperature < p.tempMin) ? p.powerGoal : (env.temperature > p.tempMax ? 0.0 : goal);
			SHOW_IF(goal != prev, "Temperature is " << env.temperature << ", changing power goal to " << goal << "W" << std::endl);
		}

		//-- Set the changes in values
		err_pwr = goal - app_pwr;
//		avg_err = mm_acerr.add(err_pwr);
//		avg_err = ABS(avg_err);
//		vals.p   = err_pwr;
//		vals.i  += err_pwr * dt;
//		vals.d  = (lst_pwr - app_pwr) / dt;
//		lst_pwr = app_pwr;
		t_lst   = t_now;

		SHOW("Power: AC " << mm_acpwr.mean() << "W (" << ac.power    << "W {" << goal    << "W}) [Temp: " << env.temperature << "C]" << std::endl
		     << "       DC " << mm_dcpwr.mean() << "W (" << dc.power << "W [" << dcst << "])" << std::endl);
		sprintf(c_aux, "%s", domoticz_mssgs[MQTT_AC_POWER]); sprintf(mqtt_mssg, c_aux, mm_acpwr.mean());
		domoticz->sendMessage(mqtt_mssg);
		sprintf(c_aux, "%s", domoticz_mssgs[MQTT_DC_POWER]); sprintf(mqtt_mssg, c_aux, mm_dcpwr.mean());
		domoticz->sendMessage(mqtt_mssg);
		sprintf(c_aux, "%s", domoticz_mssgs[MQTT_DC_VOLTS]); sprintf(mqtt_mssg, c_aux, mm_dcvolts.mean());
		domoticz->sendMessage(mqtt_mssg);
		sprintf(c_aux, "%s", domoticz_mssgs[MQTT_TEMP_HUMID]); sprintf(mqtt_mssg, c_aux, env.temperature, env.humidity, env.status);
		domoticz->sendMessage(mqtt_mssg);

		if(p.isActive)
		{
			if(!mm_acpwr.isPosibleSpike() or (mm_acpwr.isPosibleSpike() and err_pwr > 0))
			{
				num_pid  = pid.calculate(err_pwr, dt) / pid_scl;
				dp       = num_pid - pwm;
				pwm      = num_pid;

//std::cout << "errPwr: " << err_pwr << "W (dPWM: " << dp << " [PWM: " << pwm << "])" << std::endl;

				pwm      = (pwm > 1.0 ? 1.0 : (pwm < 0.0 ? 0.0 : pwm));

				if(pwm == 0.0 and err_pwr < 0.0)
					pid.clear();
			}
			b_send_pwm = true;

		}
		else if(pwm != 0.0)
		{
			pwm = 0.0f;
		}

		if(b_send_pwm)
		{
			SHOW("   - PWM: " << pwm << " (" << dp << ")" << std::endl);
			sprintf(c_aux, "%s", domoticz_mssgs[MQTT_PWM]); sprintf(mqtt_mssg, c_aux, pwm * 100.0);
			domoticz->sendMessage(mqtt_mssg);
			setPWM(pwm);
			if(!p.isActive)
			{
				b_send_pwm = false;
				pid.clear();
			}
		}

		//-- Make sure we are in synch with Domoticz.
		syncMqtt(domoticz, (const char*)domoticz_mssgs, p);

		if(/*dtbin > 1.0 and*/ (b_log_bin or b_log_file))
		{
			LOG_IF_STREAM(b_log_file, o_file,
					dt << ";" << run_s << ";" << mm_acpwr.mean() << ";" << mm_dcpwr.mean()
					   << ";" << goal << ";" << pwm << ";" << dp << ";" << (mm_acpwr.isPosibleSpike() ? "1" : "0") << std::endl);
			LOG_IF_STREAM(b_log_bin, o_bin,
					dt << run_s << ac.power << dc_s1.power << pwm << goal << env.temperature << dcst_s1 << std::flush);
			dtbin = 0.0;
		}
	}

	SHOW("Set final PWM to " << p.finalPWM << std::endl);
	setPWM(p.finalPWM);

	if(b_log_file)
		o_file.close();
	if(b_log_bin)
		o_bin.close();
	modbus.close();

	if(domoticz)
		delete domoticz;
	domoticz = nullptr;

	return 0;
}
//-----------------------------------------------------------------------------

mb_ret getResult(nsModbus::MB_Std_Message* mssg)
{
	int    len = mssg->message[0];
	mb_ret ret;

	switch(len)//mssg->header.transactionID % NUM_QUERYS)
	{
	case 1:
		ret.ubyte = mssg->message[1];
		break;
	case 2:
		ret.uword = ntohs(*((unsigned short*)&(mssg->message[1])));
		break;
	case 4:
		ret.udword = ntohl(*((unsigned int*)&(mssg->message[1])));
		break;
	default:
		mssg->message[len + 1] = '\0';
		break;
	}

	return ret;
}
//-----------------------------------------------------------------------------

void onReceive(nsModbus::TModbus* mb, nsModbus::MB_Std_Message* mssg, void* usr)
{
	if(usr == &ecpconn)
		ecpconn = getResult(mssg).uword & 0x01;
	else if(usr == &dcsf.power or usr == &dcsf.current or usr == &dcsf.volts)
		*((float*)usr) = pow(10.0, getResult(mssg).word);
	else if(usr == &ac.power or usr == &dc.power or usr == &dc.volts)
	{
		float aux = getResult(mssg).real;
		if(isnormal(aux))
			*((float*)usr) = aux;
	}
	else if(usr == &dcst_s1 or usr == &dcst_s2 or usr == &dcst)
		*((uint16_t*)usr) = getResult(mssg).uword;
	else if(usr) //-- (usr == &dc.power or usr == &dc.current or usr == &dc.volts)
		*((float*)usr) = getResult(mssg).uword;
	else
		std::cerr << " ---- User pointer is NULL ----" << std::endl;
}
//-----------------------------------------------------------------------------

#ifdef __linux_build__
uint32_t expectPulse(bool level)
{
	uint32_t count = 0;
	while(digitalRead(DHT_PIN) == level)
	{
		delayMicroseconds(1);
		if(++count == 255)
			return 0;	// Exceeded timeout, fail.
	}
	return count;
}
#endif
//-----------------------------------------------------------------------------

void readDht11(env_info& ei)
{
#ifdef __linux_build__
	uint8_t  data[5];
	uint32_t cycles[80];

	memset(data, 0, 5 * sizeof(uint8_t));
	memset(cycles, 0, 80 * sizeof(uint32_t));

	pinMode(DHT_PIN, OUTPUT);
	digitalWrite(DHT_PIN, LOW);
	delay(18);
	digitalWrite(DHT_PIN, HIGH);
	delayMicroseconds(40);
	pinMode(DHT_PIN, INPUT);
	delayMicroseconds(10);

	if(!expectPulse(LOW))
		return;
	if(!expectPulse(HIGH))
		return;

	for(int i = 0; i < 80; i += 2)
	{
		cycles[i]     = expectPulse(LOW);
		cycles[i + 1] = expectPulse(HIGH);
	}

	for(int i = 0; i < 40; ++i)
	{
		uint32_t low  = cycles[2 * i];
		uint32_t high = cycles[2 * i + 1];

	    if(!low or !high)
			return;

	    data[i / 8] <<= 1;
		if(high > low)
			data[i / 8] |= 1;
	}

//	std::cout << "   (" << int(data[0]) << "." << int(data[1]) << "% " << int(data[2]) << "." << int(data[3]) << "C): " << int(data[4]) << std::endl;
	if(data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xff) )
	{
		char c_aux[1024];
		sprintf(c_aux, "%d.%d", data[0], data[1]);
		sscanf(c_aux, "%f", &ei.humidity);
		sprintf(c_aux, "%d.%d", data[2], data[3]);
		sscanf(c_aux, "%f", &ei.temperature);
		ei.status = ei.humidity < 39.0 ? 2 : (ei.humidity > 70.0 ? 3 : 1);
//		float f = dht11_dat[2] * 9. / 5. + 32;
//		printf("Humidity = %d.%d %% Temperature = %d.%d C (%.1f F)\n",
//			dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3], f);
	}
#endif
}
//-----------------------------------------------------------------------------

void parseCommand(int idx, char** argv, SParams& p)
{
	std::string str_param = std::string(argv[idx]);
	char c_aux[512];

	if(str_param.find("--help") != std::string::npos or str_param.find("-h") != std::string::npos)
	{
		showCommandLineHelp(p);
		exit(0);
	}
	else if(str_param.find("--final-pwm") != std::string::npos or str_param.find("-fp") != std::string::npos)
		sscanf(argv[++idx], "%f", &p.finalPWM);
	else if(str_param.find("--force-goal") != std::string::npos or str_param.find("-fg") != std::string::npos)
		p.forceGoal = true;
	else if(str_param.find("--temp-min") != std::string::npos or str_param.find("-tn") != std::string::npos)
		sscanf(argv[++idx], "%f", &p.tempMin);
	else if(str_param.find("--temp-max") != std::string::npos or str_param.find("-tx") != std::string::npos)
		sscanf(argv[++idx], "%f", &p.tempMax);
	else if(str_param.find("--output-file") != std::string::npos or str_param.find("-o") != std::string::npos)
		p.logFile = std::string(argv[++idx]);
	else if(str_param.find("--output-bin") != std::string::npos or str_param.find("-b") != std::string::npos)
		p.binFile = std::string(argv[++idx]);
	else if(str_param.find("--power-goal") != std::string::npos or str_param.find("-p") != std::string::npos)
		sscanf(argv[++idx], "%f", &p.powerGoal);
	else if(str_param.find("--time-to-run") != std::string::npos or str_param.find("-t") != std::string::npos)
		sscanf(argv[++idx], "%d", &p.timeToRun);
	else if(str_param.find("--verbose") != std::string::npos or str_param.find("-v") != std::string::npos)
		p.verbose = true;
	else if(str_param.find("--mqtt-id") != std::string::npos or str_param.find("-mi") != std::string::npos)
	{
		sscanf(argv[++idx], "%s", c_aux);
		p.mqtt.id = c_aux;
	}
	else if(str_param.find("--mqtt-host") != std::string::npos or str_param.find("-mh") != std::string::npos)
	{
		sscanf(argv[++idx], "%s", c_aux);
		p.mqtt.host = c_aux;
	}
	else if(str_param.find("--mqtt-port") != std::string::npos or str_param.find("-mp") != std::string::npos)
		sscanf(argv[++idx], "%d", &p.mqtt.port);
}
//-----------------------------------------------------------------------------

void parseCommandLine(int argc, char** argv, SParams& p)
{
	//memset((void*)&p, 0, sizeof(p));

	p.hasChanged = true;
	p.entryLine  = argv[0];
	p.timeToRun  = TIME_TO_RUN;
	p.powerGoal  = PWR_GOAL;
	p.tempMin    = TEMP_MIN;
	p.tempMax    = TEMP_MAX;
	p.mqtt.id   = std::string("radiator");
	p.mqtt.host = std::string("localhost");
	p.mqtt.port = 1883;

	for(int i = 1; i < argc; ++i)
		parseCommand(i, argv, p);
}
//-----------------------------------------------------------------------------

void showCommandLineHelp(SParams& p)
{
	std::string bin_name = p.entryLine.substr(p.entryLine.rfind('/') + 1);
	std::cout << bin_name << " [params]" << std::endl << std::endl
	          << "--output-file, -o  <filename>    Log data into <filename> as a csv format file."          << std::endl
		  << "--output-bin,  -b  <filename>    Log data into <filename> in binary format."              << std::endl
	          << "--power-goal,  -p  <pwr>         Maximum power to take from the grid. <pwr> is in Watts." << std::endl
		  << "                                 Default = 50.0W."                                        << std::endl
		  << "--final-pwm,   -fp <value>       Final value to set PWM at end of program. <value> is a"  << std::endl
		  << "                                 decimal number ranging between 0.0 and 1.0. Default = 0" << std::endl
		  << "--time-to-run, -t  <time>        Specifies the time (in seconds) the program will run."   << std::endl
		  << "                                 for. Default time is 60 seconds."                        << std::endl
		  << "--force-goal,  -fg               Force to seek --power-goal"                              << std::endl
		  << "--temp-min,    -tn <temp>        Minimum temperature in degrees Celsius. Default = 19.0"  << std::endl
		  << "--temp-max,    -tx <temp>        Maximum temperature in degrees Celsius. Default = 26.0"  << std::endl
		  << "--mqtt-id,     -mi <id>          MQTT device identification. Default = radiator"          << std::endl
		  << "--mqtt-host,   -mh <host>        MQQT broker to connect to. Default = localhost"          << std::endl
		  << "--mqtt-port,   -mh <port>        MQTT broker connection port. Default = 1883"             << std::endl;
}
//-----------------------------------------------------------------------------
