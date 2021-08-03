//-----------------------------------------------------------------------------
#include <iostream>
#include <unistd.h>

#include "mqtt_domoticz.h"
//-----------------------------------------------------------------------------
using namespace mosqcpp;
//-----------------------------------------------------------------------------
uint32_t TMosquitto::m_uiInstances = 0;
//-----------------------------------------------------------------------------

TMosquitto::TMosquitto(const std::string _i)
	: m_mosquitto(nullptr), m_host("localhost"), m_id(_i), m_port(1883)
	, m_keepalv(120), m_qos(eAtLeastOnce), m_bConnected(false)
{
	if(!m_uiInstances)
		mosquitto_lib_init();
	++m_uiInstances;

	m_mosquitto = mosquitto_new(nullptr, true, this);
	if(m_mosquitto)
	{
		mosquitto_connect_callback_set(m_mosquitto, m_onConnect);
		mosquitto_disconnect_callback_set(m_mosquitto, m_onDisconnect);
		mosquitto_subscribe_callback_set(m_mosquitto, m_onSubscribe);
		mosquitto_message_callback_set(m_mosquitto, m_onMessage);
	}
}
//-----------------------------------------------------------------------------

TMosquitto::~TMosquitto()
{
	disconnect();
	loop_stop(true);
	if(m_mosquitto)
		mosquitto_destroy(m_mosquitto);
	m_mosquitto = nullptr;
	if(!--m_uiInstances)
		mosquitto_lib_cleanup();
}
//-----------------------------------------------------------------------------

void TMosquitto::m_onConnect(mosquitto* _mosq, void* _obj, int _rc)
{
	if(_mosq and _obj)
	{
		auto tmosq = (TMosquitto*)(_obj);
		if(tmosq->m_mosquitto == _mosq)
		{
			tmosq->m_bConnected = !_rc;
			tmosq->on_connect(_rc);
		}
	}
}
//-----------------------------------------------------------------------------

void TMosquitto::m_onDisconnect(mosquitto* _mosq, void* _obj, int _rc)
{
	if(_mosq and _obj)
	{
		auto tmosq = (TMosquitto*)(_obj);
		if(tmosq->m_mosquitto == _mosq)
		{
			tmosq->m_bConnected = false;
			if(tmosq->on_disconnect)
				tmosq->on_disconnect(_rc);
		}
	}
}
//-----------------------------------------------------------------------------

void TMosquitto::m_onSubscribe(mosquitto* _mosq, void* _obj, int _mid, int _qos_count, const int* _granted_qos)
{
	if(_mosq and _obj)
	{
		auto tmosq = (TMosquitto*)(_obj);
		if(tmosq->m_mosquitto == _mosq)
		{
			tmosq->on_subscribe(_mid, _qos_count, _granted_qos);
		}
	}
}
//-----------------------------------------------------------------------------

void TMosquitto::m_onMessage(mosquitto* _mosq, void* _obj, const mosquitto_message* _mssg)
{
	if(_mosq and _obj)
	{
		auto tmosq = (TMosquitto*)(_obj);
		if(tmosq->m_mosquitto == _mosq)
		{
			tmosq->on_message(_mssg);
		}
	}
}
//-----------------------------------------------------------------------------

void TMosquitto::connect(const std::string _host, int _port, int _ka)
{
	if(m_mosquitto)
	{
		if(_host.size()) m_host    = _host;
		if(_port)        m_port    = _port;
		if(_ka)          m_keepalv = _ka;
		mosquitto_connect_async(m_mosquitto, m_host.c_str(), m_port, m_keepalv);
	}
}
//-----------------------------------------------------------------------------

void TMosquitto::disconnect()
{
	if(m_mosquitto)
	{
		mosquitto_disconnect(m_mosquitto);
	}
}
//-----------------------------------------------------------------------------

void TMosquitto::reconnect()
{
	if(m_mosquitto)
	{
		mosquitto_reconnect_async(m_mosquitto);
	}
}
//-----------------------------------------------------------------------------

void TMosquitto::subscribe(int* mid, const std::string sub)
{
	if(m_mosquitto)
	{
		mosquitto_subscribe(m_mosquitto, mid, sub.c_str(), m_qos);
	}
}
//-----------------------------------------------------------------------------

int TMosquitto::publish(int* mid, const std::string topic, int payloadlen, const void* payload, bool retain)
{
	if(m_mosquitto)
	{
		return mosquitto_publish(m_mosquitto, mid, topic.c_str(), payloadlen, payload, m_qos, retain);
	}
	return MOSQ_ERR_ERRNO;
}
//-----------------------------------------------------------------------------

void TMosquitto::loop_start()
{
	if(m_mosquitto)
	{
		mosquitto_loop_start(m_mosquitto);
	}
}
//-----------------------------------------------------------------------------

void TMosquitto::loop_stop(bool _force)
{
	if(m_mosquitto)
	{
		mosquitto_loop_stop(m_mosquitto, _force);
	}
}
//-----------------------------------------------------------------------------












MqttDomoticz::MqttDomoticz(const std::string _i, const std::string _h, const int _p)
	: mosqcpp::TMosquitto(_i), onMessage(nullptr), onSubscribe(nullptr)
{
	connect(_h, _p);
	loop_start();
}
//-----------------------------------------------------------------------------

MqttDomoticz::~MqttDomoticz()
{
	// disconnect();
	// loop_stop();
}
//-----------------------------------------------------------------------------

void MqttDomoticz::on_connect(int rc)
{
	if(rc == 0)
	{
		char c_topic[256];
		sprintf(c_topic, "domoticz/out/%s/commands", id().c_str());
		subscribe(NULL, std::string(c_topic));
	}
}
//-----------------------------------------------------------------------------

void MqttDomoticz::on_message(const mosquitto_message* mssg)
{
	if(onMessage)
		onMessage(mssg->topic, reinterpret_cast<const char*>(mssg->payload));
}
//-----------------------------------------------------------------------------

void MqttDomoticz::on_subscribe(int mid, int qos_count, const int* granted_qos)
{
	if(onSubscribe)
		onSubscribe(mid, qos_count, granted_qos);
}
//-----------------------------------------------------------------------------

bool MqttDomoticz::sendMessage(const char* _mssg)
{
	char c_topic[256];
	sprintf(c_topic, "domoticz/in/%s/sensors", id().c_str());
	return publish(NULL, std::string(c_topic), strlen(_mssg), _mssg, false) == MOSQ_ERR_SUCCESS;
}
//-----------------------------------------------------------------------------
