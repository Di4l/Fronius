//-----------------------------------------------------------------------------
#ifndef __MQTT_DOMOTICZ_H__
#define __MQTT_DOMOTICZ_H__
//-----------------------------------------------------------------------------
#include <mosquitto.h>
#include <string.h>
//-----------------------------------------------------------------------------

typedef void (*v_dom_cc)(const char*, const char*);
typedef void (*v_dom_iipci)(int, int, const int*);
//-----------------------------------------------------------------------------

namespace mosqcpp
{
	//-------------------------------------------------------------------------

	enum EMqttQos : int { eAtMostOnce = 0, eAtLeastOnce = 1, eExactlyOnce = 2 };
	//-------------------------------------------------------------------------

	typedef void (*v_tmosq_evt_i)(int);
	//-------------------------------------------------------------------------

	class TMosquitto
	{
	private:
		static uint32_t m_uiInstances;

		static void m_onConnect(mosquitto* _mosq, void* _obj, int _rc);
		static void m_onDisconnect(mosquitto* _mosq, void* _obj, int _rc);
		static void m_onSubscribe(mosquitto* _mosq, void* _obj, int _mid, int _qos_count, const int* _granted_qos);
		static void m_onMessage(mosquitto* _mosq, void* _obj, const mosquitto_message* _mssg);

		mosquitto*  m_mosquitto;
		std::string m_host;
		std::string m_id;
		int         m_port;
		int         m_keepalv;
		EMqttQos    m_qos;

		bool m_bConnected;

	protected:
		virtual void on_connect(int rc) = 0;
		virtual void on_subscribe(int mid, int qos_count, const int* granted_qos) = 0;
		virtual void on_message(const mosquitto_message* mssg) = 0;

	public:
		TMosquitto(const std::string _i);
		virtual ~TMosquitto();

		inline std::string host() { return m_host; }
		inline std::string id()   { return m_id;   }
		inline EMqttQos&   qos()  { return m_qos;  }

		inline int keepAlive() { return m_keepalv; }

		void connect(const std::string _host = std::string("localhost"), int _port = 1883, int _ka = 120);
		void reconnect();
		void disconnect();

		void subscribe(int* mid, const std::string sub);
		int  publish(int* mid, const std::string topic,	int	payloadlen, const void* payload, bool retain);

		void loop_start();
		void loop_stop(bool _force);

		v_tmosq_evt_i on_disconnect = nullptr;
	};
	//-------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------

class MqttDomoticz : public mosqcpp::TMosquitto
{
private:
protected:
	void on_connect(int rc);
	void on_message(const mosquitto_message* mssg);
	void on_subscribe(int mid, int qos_count, const int* granted_qos);

public:
	MqttDomoticz(const std::string _i, const std::string _h = "localhost", const int _p = 1883);
	~MqttDomoticz();

	bool sendMessage(const char* _mssg);

	v_dom_cc    onMessage;
	v_dom_iipci onSubscribe;
};
//-----------------------------------------------------------------------------
#endif //__MQTT_DOMOTICZ_H__
