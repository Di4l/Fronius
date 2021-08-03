//-----------------------------------------------------------------------------
#include <ctime>
#include <cstring>
#include "modbus.hpp"
//-----------------------------------------------------------------------------
using namespace nsModbus;
//-----------------------------------------------------------------------------
WORD TModbus::m_transacion_id = 0;
//-----------------------------------------------------------------------------

//static const char* MBFunctionNames[] = {
//								   "No Function",                   // 0x00
//                                   "Read Coils",                    // 0x01
//                                   "Read Discrete Inputs",          // 0x02
//                                   "Read Holding Registers",        // 0x03
//                                   "Read Input Registers",          // 0x04
//                                   "Write Single Coil",             // 0x05
//                                   "Write Single Register",         // 0x06
//                                   "Read Exception Status",         // 0x07
//                                   "Diagnostics",                   // 0x08
//                                   "No Function",                   // 0x09
//                                   "No Function",                   // 0x0A
//                                   "Get Comm Event Counter",        // 0x0B
//                                   "Get Comm Event Log",            // 0x0C
//                                   "No Function",                   // 0x0D
//                                   "No Function",                   // 0x0E
//                                   "Write Multiple Coils",          // 0x0F
//                                   "Write Multiple Registers",      // 0x10
//                                   "Report Slave ID",               // 0x11
//                                   "No Function",                   // 0x12
//                                   "No Function",                   // 0x13
//                                   "Read File Record",              // 0x14
//                                   "Write File Record",             // 0x15
//                                   "Mask Write Register",           // 0x16
//                                   "Read & Write Multiple Registers",   // 0x17
//                                   "Read FIFO Queue",               // 0x18
//                                   "No Function",                   // 0x19
//                                   "No Function",                   // 0x1A
//                                   "No Function",                   // 0x1B
//                                   "No Function",                   // 0x1C
//                                   "No Function",                   // 0x1D
//                                   "No Function",                   // 0x1E
//                                   "No Function",                   // 0x1F
//                                   "No Function",                   // 0x20
//                                   "No Function",                   // 0x21
//                                   "No Function",                   // 0x22
//                                   "No Function",                   // 0x23
//                                   "No Function",                   // 0x24
//                                   "No Function",                   // 0x25
//                                   "No Function",                   // 0x26
//                                   "No Function",                   // 0x27
//                                   "No Function",                   // 0x28
//                                   "No Function",                   // 0x29
//                                   "No Function",                   // 0x2A
//                                   "Encapsulated Interface Transport" };    // 0x2B
//-----------------------------------------------------------------------------

namespace NsError
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













int TModbus::sendRecv(MB_Std_Message& msMssg, void* user)
{
    char         c_buffer[MB_MAX_SOCK_BUFFER_DATA_LEN];
    int          i_rtn = 0, i_len = 0;
    unsigned int ui_rcv = MB_MAX_SOCK_BUFFER_DATA_LEN;

    if(m_transacion_id > 0x7fff) m_transacion_id = 0;
    msMssg.header.transactionID = ++m_transacion_id;
	i_len = 3 * sizeof(WORD) + ntohs(msMssg.header.length);
	//-- Eviamos datos
	m_tcpip.send((const char*)&msMssg, i_len);
	//-- Recivimos datos
	i_rtn = m_tcpip.recv(c_buffer, ui_rcv);
	if(i_rtn > 0)
		socketReceived(c_buffer, i_rtn, user);

    return i_rtn;
}
//-----------------------------------------------------------------------------

void TModbus::socketReceived(char* Data, int Length, void* user)
{
	MB_Std_Message* mbs_msg = (MB_Std_Message*)Data;
    //-- Si la longitud no coincide, error
	if(Length != int(3 * sizeof(WORD) + ntohs(mbs_msg->header.length)))
		throw NsError::TError("Datos de Socket", "Longitud de datos erronea", __LINE__, __FILE__);
	else if(mbs_msg->header.transactionID != m_transacion_id)
		throw NsError::TError("Datos de Socket", "Respuesta a otro mensaje", __LINE__, __FILE__);
	else if(mbs_msg->header.function & MB_MSG_EXCEPTION)
		throw NsError::TError("Datos de Socket", "Error de funcion", __LINE__, __FILE__);
	else if(onReceive != NULL)
		onReceive(this, mbs_msg, user);
}
//-----------------------------------------------------------------------------

void TModbus::open(const char* HostName, short Port)
{
	char c_url[512];
	sprintf(c_url, "tcp://0:%s:%d", HostName, Port);
	m_tcpip.openSocket(std::string(c_url));
}
//-----------------------------------------------------------------------------

void TModbus::close()
{
	m_tcpip.closeSocket();
}
//-----------------------------------------------------------------------------

void TModbus::readCoils(char UnitID, unsigned short StartAddress,
                            unsigned short Quantity, void* UserData)
{
	MB_Std_Message msm_msg;
	int            i_msg_pos = 0;
    unsigned short us_addr   = htons(StartAddress);
    unsigned short us_qtty   = htons(Quantity);

    if(Quantity < 1)
		throw NsError::TError("Funcion Modbus::ReadCoils", "Numero minimo de registros no alcanzado", __LINE__, __FILE__);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

	//-- Initialize the MODBUS header
    msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(6);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_READ_COILS;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_qtty, sizeof(unsigned short));

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::readDiscreteInputs(char UnitID, unsigned short StartAddress,
                            unsigned short Quantity, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
	unsigned short us_addr   = htons(StartAddress);
    unsigned short us_qtty   = htons(Quantity);

    if(Quantity < 1)
		throw NsError::TError("Funcion Modbus::ReadDiscreteInputs", "Numero minimo de registros minimo no alcanzado", __LINE__, __FILE__);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID    = PROTOCOL_MODBUS;
	msm_msg.header.length        = htons(6);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_READ_DISCRETE_INPUTS;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_qtty, sizeof(unsigned short));

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::readHoldingRegisters(char UnitID, unsigned short StartAddress,
                            unsigned short Quantity, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
    unsigned short us_addr   = htons(StartAddress);
    unsigned short us_qtty   = htons(Quantity);

    if(Quantity < 1)
		throw NsError::TError("Funcion Modbus::ReadHoldingRegisters", "Numero minimo de registros no alcanzado", __LINE__, __FILE__);
	if(Quantity > 0x007D)
		throw NsError::TError("Funcion Modbus::ReadHoldingRegisters", "Numero maximo de registros excedido", __LINE__, __FILE__);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
	msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(6);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_READ_HOLDING_REGISTERS;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_qtty, sizeof(unsigned short));

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::readInputRegisters(char UnitID, unsigned short StartAddress,
                            unsigned short Quantity, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
	unsigned short us_addr   = htons(StartAddress);
    unsigned short us_qtty   = htons(Quantity);

    if(Quantity < 1)
		throw NsError::TError("Funcion Modbus::ReadInputRegisters", "Numero minimo de registros no alcanzado", __LINE__, __FILE__);
	if(Quantity > 0x007D)
		throw NsError::TError("Funcion Modbus::ReadInputRegisters", "Numero maximo de registros superado", __LINE__, __FILE__);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(6);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_READ_INPUT_REGISTERS;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_qtty, sizeof(unsigned short));

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::writeSingleCoil(char UnitID, unsigned short Address, bool State, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
    unsigned short us_state  = htons(State ? 0xFF00 : 0x0000);
	unsigned short us_addr   = htons(Address);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(6);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_WRITE_SINGLE_COIL;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_state, sizeof(unsigned short));

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::writeSingleRegister(char UnitID, unsigned short Address, short Value, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
    unsigned short us_addr   = htons(Address);
    unsigned short us_value  = htons(Value);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
	msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(6);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_WRITE_SINGLE_REGISTER;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_value, sizeof(short));

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::writeMultipleCoils(char UnitID, unsigned short StartAddress,
                            unsigned short Quantity, bool* Values, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
	char           c_values  = 0;
    unsigned short us_addr   = htons(StartAddress);
    unsigned short us_qtty   = htons(Quantity);
    unsigned short us_bytes  = Quantity / 8 + (Quantity % 8 ? 1 : 0);

    if(Quantity < 1)
		throw NsError::TError("Funcion Modbus::WriteMultipleCoils", "Numero minimo de registros no alcanzado", __LINE__, __FILE__);
	if(Quantity > 0x007B0)
		throw NsError::TError("Funcion Modbus::WriteMultipleCoils", "Numero maximo de registros superado", __LINE__, __FILE__);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(7 + us_bytes);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_WRITE_MULTIPLE_COILS;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));
	i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_qtty, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    msm_msg.message[i_msg_pos++] = char(us_bytes);
    for(int i = 0; i < Quantity; i++)
    {
        c_values |= ((Values[i] ? 0x01 : 0x00) << (i % 8));
        if(!((i + 1) % 8))
        {
            msm_msg.message[i_msg_pos++] = c_values;
            c_values = 0;
        }
    }
    msm_msg.message[i_msg_pos] = c_values;

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::writeMultipleRegisters(char UnitID, unsigned short StartAddress,
                            unsigned short Quantity, short* Values, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
    unsigned short us_addr   = htons(StartAddress);
    unsigned short us_qtty   = htons(Quantity);
    short          s_value;

    if(Quantity < 1)
		throw NsError::TError("Funcion Modbus::WriteMultipleRegisters", "Numero minimo de registros no alcanzado", __LINE__, __FILE__);
	if(Quantity > 0x0078)
		throw NsError::TError("Funcion Modbus::WriteMultipleRegisters", "Numero maximo de registros superado", __LINE__, __FILE__);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(7 + (2 * Quantity));
    msm_msg.header.unitID        = UnitID;
	msm_msg.header.function      = MB_FUNC_WRITE_MULTIPLE_REGISTERS;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_qtty, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    msm_msg.message[i_msg_pos++] = char(Quantity * sizeof(short));

    for(int i = 0; i < Quantity; ++i)
    {
        s_value = htons(Values[i]);
        memcpy(&msm_msg.message[i_msg_pos], &s_value, sizeof(short));
        i_msg_pos += sizeof(short);
    }

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::readFileRecord(char UnitID, unsigned short GroupCount,
        unsigned short* FileNums, unsigned short* RecordNums, unsigned short* RecordLengths,
        void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
    unsigned char  uc_byt_cnt = (unsigned char)(7 * GroupCount);
    unsigned short us_filnum, us_recnum, us_reclen;

    if(GroupCount < 1)
		throw NsError::TError("Funcion Modbus::ReadFileRecord", "Numero minimo de grupos no alcanzado", __LINE__, __FILE__);
	if(GroupCount > 0xF5)
		throw NsError::TError("Funcion Modbus::ReadFileRecord", "Numero maximo de grupos superado", __LINE__, __FILE__);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID = PROTOCOL_MODBUS;
    msm_msg.header.length     = htons(3 + 7 * GroupCount);
	msm_msg.header.unitID     = UnitID;
    msm_msg.header.function   = MB_FUNC_READ_FILE_RECORD;

    //-- Initialize the MODBUS message
    msm_msg.message[i_msg_pos++] = uc_byt_cnt;
    for(int i = 0; i < GroupCount; ++i)
    {
        if(RecordNums[i] > 0x270F)
			throw NsError::TError("Funcion Modbus::ReadFileRecord", "Limite de registro superado", __LINE__, __FILE__);

        us_filnum = htons(FileNums[i]);
        us_recnum = htons(RecordNums[i]);
        us_reclen = htons(RecordLengths[i]);

        msm_msg.message[i_msg_pos++] = MB_SUBFUNC_REF_TYPE_GROUP;
        memcpy(&msm_msg.message[i_msg_pos], &us_filnum, sizeof(unsigned short));
        i_msg_pos += sizeof(unsigned short);
        memcpy(&msm_msg.message[i_msg_pos], &us_recnum, sizeof(unsigned short));
        i_msg_pos += sizeof(unsigned short);
		memcpy(&msm_msg.message[i_msg_pos], &us_reclen, sizeof(unsigned short));
        i_msg_pos += sizeof(unsigned short);
    }

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::writeFileRecord(char UnitID, unsigned short GroupCount,
        unsigned short* FileNums, unsigned short* RecordNums, unsigned short* RecordLengths,
        void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
    unsigned char  uc_byt_cnt = (unsigned char)(7 * GroupCount);
    unsigned short us_filnum, us_recnum, us_reclen;

    if(GroupCount < 1)
		throw NsError::TError("Funcion Modbus::WriteFileRecord", "Numero minimo de grupos no alcanzado", __LINE__, __FILE__);
	if(GroupCount > 0xF5)
		throw NsError::TError("Funcion Modbus::WriteFileRecord", "Numero maximo de grupos superado", __LINE__, __FILE__);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID = PROTOCOL_MODBUS;
    msm_msg.header.length     = htons(3 + 7 * GroupCount);
    msm_msg.header.unitID     = UnitID;
    msm_msg.header.function   = MB_FUNC_WRITE_FILE_RECORD;

    //-- Initialize the MODBUS message
    msm_msg.message[i_msg_pos++] = uc_byt_cnt;
    for(int i = 0; i < GroupCount; ++i)
    {
		if(RecordNums[i] > 0x270F)
			throw NsError::TError("Funcion Modbus::WriteFileRecord", "Limite de registro superado", __LINE__, __FILE__);

		us_filnum = htons(FileNums[i]);
		us_recnum = htons(RecordNums[i]);
		us_reclen = htons(RecordLengths[i]);

        msm_msg.message[i_msg_pos++] = MB_SUBFUNC_REF_TYPE_GROUP;
        memcpy(&msm_msg.message[i_msg_pos], &us_filnum, sizeof(unsigned short));
        i_msg_pos += sizeof(unsigned short);
        memcpy(&msm_msg.message[i_msg_pos], &us_recnum, sizeof(unsigned short));
        i_msg_pos += sizeof(unsigned short);
        memcpy(&msm_msg.message[i_msg_pos], &us_reclen, sizeof(unsigned short));
        i_msg_pos += sizeof(unsigned short);
    }

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::maskWriteRegister(char UnitID, unsigned short Address,
                    unsigned short AndMask, unsigned short OrMask, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
	unsigned short us_addr   = htons(Address);
    unsigned short us_andmsk = htons(AndMask);
    unsigned short us_ormsk  = htons(OrMask);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(8);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_MASK_WRITE_REGISTER;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_andmsk, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_ormsk, sizeof(unsigned short));

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::readWriteMultipleRegisters(char UnitID, unsigned short ReadStartAddress,
                        unsigned short ReadQuantity, unsigned short WriteStartAddress,
                        unsigned short WriteQuantity, short* Values, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos  = 0;
    unsigned short us_rd_addr = htons(ReadStartAddress);
    unsigned short us_rd_qtty = htons(ReadQuantity);
    unsigned short us_wr_addr = htons(WriteStartAddress);
    unsigned short us_wr_qtty = htons(WriteQuantity);
    short          s_value;

    if(ReadQuantity < 1)
		throw NsError::TError("Funcion Modbus::ReadWriteMultipleRegisters", "Numero minimo de registros de lectura no alcanzado", __LINE__, __FILE__);
	if(ReadQuantity > 0x0076)
		throw NsError::TError("Funcion Modbus::ReadWriteMultipleRegisters", "Numero maximo de registros de lectura sueprado", __LINE__, __FILE__);
	if(WriteQuantity < 1)
		throw NsError::TError("Funcion Modbus::ReadWriteMultipleRegisters", "Numero minimo de registros de escritura no alcanzado", __LINE__, __FILE__);
	if(WriteQuantity > 0x0076)
		throw NsError::TError("Funcion Modbus::ReadWriteMultipleRegisters", "Numero maximo de registros de escritua superado", __LINE__, __FILE__);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(11 + 2 * WriteQuantity);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_RD_WR_MULTIPLE_REGISTERS;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_rd_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_rd_qtty, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_wr_addr, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    memcpy(&msm_msg.message[i_msg_pos], &us_wr_qtty, sizeof(unsigned short));
    i_msg_pos += sizeof(unsigned short);
    msm_msg.message[i_msg_pos++] = char(WriteQuantity * sizeof(short));

    for(int i = 0; i < WriteQuantity; ++i)
    {
        s_value = htons(Values[i]);
        memcpy(&msm_msg.message[i_msg_pos], &s_value, sizeof(short));
        i_msg_pos += sizeof(short);
    }

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------

void TModbus::readFIFOQueue(char UnitID, unsigned short Address, void* UserData)
{
    MB_Std_Message msm_msg;
    int            i_msg_pos = 0;
	unsigned short us_addr = htons(Address);

    memset(&msm_msg, 0, sizeof(MB_Std_Message));

    //-- Initialize the MODBUS header
    msm_msg.header.protocolID    = PROTOCOL_MODBUS;
    msm_msg.header.length        = htons(4);
    msm_msg.header.unitID        = UnitID;
    msm_msg.header.function      = MB_FUNC_READ_FIFO_QUEUE;

    //-- Initialize the MODBUS message
    memcpy(&msm_msg.message[i_msg_pos], &us_addr, sizeof(unsigned short));

    sendRecv(msm_msg, UserData);
}
//-----------------------------------------------------------------------------







/*
bool nsModbus::operator==(MB_Task& left, MB_Task& right)
{
	if(&left == &right)
		return true;
	else if(left.socketID != right.socketID)
		return false;
	else if(left.task.standard.header.transactionID != right.task.standard.header.transactionID)
		return false;
	else if(left.task.standard.header.length != right.task.standard.header.length)
		return false;
	else if(left.task.standard.header.unitID != right.task.standard.header.unitID)
		return false;
	else if(left.task.standard.header.function != right.task.standard.header.function)
		return false;
	
	return true;
}
//-----------------------------------------------------------------------------

TModbusTasks::TModbusTasks()
{
	CREATE_MUTEX(m_mSection);
    clear();
}
//-----------------------------------------------------------------------------

TModbusTasks::~TModbusTasks()
{
    clear();
	RELEASE_MUTEX(m_mSection);
}
//-----------------------------------------------------------------------------

void TModbusTasks::clear()
{
    LOCK_MUTEX(m_mSection);
    while(m_vTasks.size() > 0)
    {
		delete (*m_vTasks.begin());
		(*m_vTasks.begin()) = NULL;
        m_vTasks.erase(m_vTasks.begin());
    }
    m_wNextTransaction = 0;

    m_uiTotalTaskCount = 0;
    memset(m_uiTaskCount, 0, 44 * sizeof(unsigned int));
    memset(m_strStatMessage, 0, 1024);

    UNLOCK_MUTEX(m_mSection);
}
//-----------------------------------------------------------------------------

int TModbusTasks::findTask(MB_Task* mbTask)
{
    MB_Task* mb_tsk = NULL;
    int      i_ndx  = 0;
	m_viIterator = m_vTasks.begin();
	while(m_viIterator != m_vTasks.end())
	{
		mb_tsk = (MB_Task*)(*m_viIterator);
		if(mb_tsk && (*mb_tsk == *mbTask))
			return i_ndx;
		++i_ndx;
		++m_viIterator;
	}
	return -1;
}
//-----------------------------------------------------------------------------

inline MB_Task* TModbusTasks::getTask(int Index)
{
    return (Index < 0 || Index >= int(m_vTasks.size())) ? NULL : (MB_Task*)(m_vTasks[Index]);
}
//-----------------------------------------------------------------------------

char* TModbusTasks::getTaskStats()
{
    LOCK_MUTEX(m_mSection);
    float f_aux;

    memset(m_strStatMessage, 0, 1024);
    sprintf(m_strStatMessage, "\nTotal number of tasks: %d\n", m_uiTotalTaskCount);
    for(int i = 0; i < 44; ++i)
    {
        if(strcmp(MBFunctionNames[i], MBFunctionNames[0]) != 0)
        {
            f_aux = m_uiTotalTaskCount > 0
                  ? (100.0 * float(m_uiTaskCount[i]) / float(m_uiTotalTaskCount))
                  : 0.0;
            sprintf(&m_strStatMessage[strlen(m_strStatMessage)], "\n0x%02X\t%s: %d (%01.2f%%)",
                        i, MBFunctionNames[i], m_uiTaskCount[i], f_aux);
        }
    }
    strcat(m_strStatMessage, "\n\n");

    UNLOCK_MUTEX(m_mSection);
    return m_strStatMessage;
}
//-----------------------------------------------------------------------------

MB_Task* TModbusTasks::createTask(short SocketID, short MaxRetries)
{
    LOCK_MUTEX(m_mSection);
    MB_Task* mb_tsk = new MB_Task;
    time_t   t_rtn  = time(NULL);

    memset(mb_tsk, 0, sizeof(MB_Task));

    mb_tsk->socketID   = SocketID;
    mb_tsk->maxRetries = MaxRetries;
    memcpy(&mb_tsk->requested, localtime(&t_rtn), sizeof(tm));

    UNLOCK_MUTEX(m_mSection);
    return mb_tsk;
}
//-----------------------------------------------------------------------------

void TModbusTasks::addTask(MB_Task* mbTask)
{
	if(!mbTask)
    	return;

	LOCK_MUTEX(m_mSection);

	if(findTask(mbTask) < 0)
	{
		mbTask->task.standard.header.transactionID = htons(m_wNextTransaction++);
		m_vTasks.push_back(mbTask);

		m_uiTotalTaskCount++;
		if(m_uiTotalTaskCount == 0xffffffff)
			m_uiTotalTaskCount = 0;
		m_uiTaskCount[int(mbTask->task.standard.header.function)]++;
	}
	else
	{
		delete mbTask;
		mbTask = NULL;
	}
    UNLOCK_MUTEX(m_mSection);
}
//-----------------------------------------------------------------------------

MB_Task* TModbusTasks::getNextTask()
{
    LOCK_MUTEX(m_mSection);
    MB_Task* mb_tsk = m_vTasks.size() > 0 ? (MB_Task*)(m_vTasks[0]) : NULL;
    time_t   t_rtn  = time(NULL);

    if(mb_tsk != NULL)
        memcpy(&mb_tsk->executed, localtime(&t_rtn), sizeof(tm));

    UNLOCK_MUTEX(m_mSection);
    return mb_tsk;
}
//-----------------------------------------------------------------------------

void TModbusTasks::retryTask(MB_Task* mbTask)
{
    LOCK_MUTEX(m_mSection);
    if(findTask(mbTask) >= 0)
        m_vTasks.erase(m_viIterator);

    mbTask->retries++;
	UNLOCK_MUTEX(m_mSection);
	if(mbTask->retries >= mbTask->maxRetries)
	{
		freeTask(mbTask);
	}
	else
	{
		addTask(mbTask);
	}
}
//-----------------------------------------------------------------------------

void TModbusTasks::freeTask(MB_Task* mbTask)
{
	LOCK_MUTEX(m_mSection);
	std::vector<MB_Task*>::iterator iter = m_vTasks.begin();
	while(iter != m_vTasks.end())
	{
		if(*iter == mbTask)
		{
			delete *iter;
			*iter = NULL;
			m_vTasks.erase(iter);
			break;
		}
		++iter;
	}
    UNLOCK_MUTEX(m_mSection);
}
//-----------------------------------------------------------------------------








unsigned long nsModbus::getHostAddress(const char* cHost)
{
    if(cHost == NULL)
        return 0;

    struct hostent *he_host;

    he_host = gethostbyname(cHost);
    if(he_host == NULL)
        return 0;

    return *((unsigned long*)he_host->h_addr);
}
//-----------------------------------------------------------------------------








TModbusSocket::TModbusSocket()
{
    m_sHandle = 0;
    memset(&m_cHost,         0, MB_MAX_SOCK_BUFFER_DATA_LEN);
    memset(&m_adAddress,     0, sizeof(sockaddr_in));
    memset(&m_bfReadBuffer,  0, sizeof(MB_Buffer));
    memset(&m_bfWriteBuffer, 0, sizeof(MB_Buffer));
}
//-----------------------------------------------------------------------------

TModbusSocket::~TModbusSocket()
{
    close();
    memset(&m_adAddress,     0, sizeof(sockaddr_in));
    memset(&m_bfReadBuffer,  0, sizeof(MB_Buffer));
    memset(&m_bfWriteBuffer, 0, sizeof(MB_Buffer));
    memset(&m_cHost,         0, MB_MAX_SOCK_BUFFER_DATA_LEN);
}
//-----------------------------------------------------------------------------

void TModbusSocket::setSocketAddress(const char* HostName, short Port)
{
    unsigned long ul_addr = getHostAddress(HostName);

    memset(&m_cHost,     0, MB_MAX_SOCK_BUFFER_DATA_LEN);
    memset(&m_adAddress, 0, sizeof(struct sockaddr_in));

    strcpy(m_cHost, HostName);
    m_adAddress.sin_family = AF_INET;
    m_adAddress.sin_port = htons(Port);
    m_adAddress.sin_addr.s_addr = ul_addr == 0 ? ADDR_ANY : ul_addr;
}
//-----------------------------------------------------------------------------

void TModbusSocket::createSocket()
{
    int i_on = 1;
    int i_error;

    m_sHandle = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sHandle == INVALID_SOCKET)
    {
        m_sHandle = 0;
        throw NsError::TError("Creando Socket", WSAGetLastError(), __LINE__, __FILE__);
    }

    //-- Reutilizacion sockets
    if(setsockopt(m_sHandle, SOL_SOCKET, SO_REUSEADDR, (const char*)&i_on, sizeof(i_on)) != 0)
    {
        i_error = WSAGetLastError();
        close();
        throw NsError::TError("Propiedades de Socket", i_error, __LINE__, __FILE__);
    }
}
//-----------------------------------------------------------------------------

void TModbusSocket::setBuffer(char* Data, int Length)
{
    memset(&m_bfWriteBuffer, 0, sizeof(MB_Buffer));
    if(Data != NULL && Length > 0)
    {
        memcpy(m_bfWriteBuffer.data, Data, Length);
        m_bfWriteBuffer.length = Length;
    }
}
//-----------------------------------------------------------------------------

int TModbusSocket::getBuffer(char* Data, int Length)
{
    int i_len = 0;
    if(Data != NULL && Length > 0)
    {
        i_len = Length < m_bfReadBuffer.length ? Length : m_bfReadBuffer.length;
        memcpy(Data, m_bfReadBuffer.data, i_len);
    }
    return i_len;
}
//-----------------------------------------------------------------------------

bool TModbusSocket::open(const char* HostName, short Port)
{
    bool b_rtn = false;

    setSocketAddress(HostName, Port);
    createSocket();

    if(connect(m_sHandle, (struct sockaddr*)&m_adAddress, sizeof(struct sockaddr_in)) != 0)
    {
        close();
    }
    else
    {
        b_rtn = true;
    }

    return b_rtn;
}
//-----------------------------------------------------------------------------

bool TModbusSocket::close()
{
    if(m_sHandle != 0)
    {
        #ifdef WIN32
        closesocket(m_sHandle);
        #else
        close(m_sHandle);
        #endif
        m_sHandle = 0;
    }
    return true;
}
//-----------------------------------------------------------------------------

char* TModbusSocket::getHost()
{
    return m_cHost;
}
//-----------------------------------------------------------------------------

short TModbusSocket::getPort()
{
    return ntohs(m_adAddress.sin_port);
}
//-----------------------------------------------------------------------------

void TModbusSocket::send()
{
    if(m_sHandle != 0 and ::send(m_sHandle, m_bfWriteBuffer.data, m_bfWriteBuffer.length, 0) == SOCKET_ERROR)
        throw NsError::TError("Enviando a Socket", WSAGetLastError(), __LINE__, __FILE__);
}
//-----------------------------------------------------------------------------

int TModbusSocket::receive(int Seconds)
{
    m_bfReadBuffer.length = 0;

    if(m_sHandle != 0)
    {
        struct timeval m_timeout;
        fd_set read_set;
        int    i_slct;

        FD_ZERO(&read_set);
        FD_SET(m_sHandle, &read_set);
        memset(&m_timeout, 0, sizeof(struct timeval));
        m_timeout.tv_sec = Seconds;

        //-- Escuchamos esperando recibir datos
        i_slct = select(m_sHandle + 1, &read_set, NULL, NULL, &m_timeout);
        if(i_slct == SOCKET_ERROR)
        {
			throw NsError::TError("Esperando a Socket", WSAGetLastError(), __LINE__, __FILE__);
        }
        else if(i_slct > 0)
        {
            if(FD_ISSET(m_sHandle, &read_set))
            {
                m_bfReadBuffer.length = recv(m_sHandle, m_bfReadBuffer.data, MB_MAX_SOCK_BUFFER_DATA_LEN, 0);
				if(m_bfReadBuffer.length == SOCKET_ERROR)
					throw NsError::TError("Reciviendo de Socket", WSAGetLastError(), __LINE__, __FILE__);
			}
        }
    }

    return m_bfReadBuffer.length;
}
//-----------------------------------------------------------------------------








TModbusSockets::TModbusSockets()
{
    #ifdef WIN32
    if(WSAStartup(MAKEWORD(1, 0), &m_wsaData) != 0)
    {
		throw NsError::TError("Inicializando librería de sockets", WSAGetLastError(), __LINE__, __FILE__);
    }
    #endif

	onSocketClose  = NULL;
	onReceivedData = NULL;
	m_iTimeout     = 1;    //-- 1 segundo
    m_sLastSocket  = 0;
    clear();
}
//-----------------------------------------------------------------------------

TModbusSockets::~TModbusSockets()
{
    clear();
	m_hThread.terminate();
		
    #ifdef WIN32
    WSACleanup();
    #endif

	onReceivedData = NULL;
	onSocketClose  = NULL;
}
//-----------------------------------------------------------------------------

void TModbusSockets::thrdProcess(nsThread::TThread* Sender, void* User)
{
	TModbusSockets* mb_skcts = (TModbusSockets*)User;
    MB_Task* 		mb_tsk;
    
    if(NULL == mb_skcts)
		throw NsError::TError("Hilo de Sockets Modbus", "Puntero a ModbusSockets es NULO", __LINE__, __FILE__);
	//-- Se suspende el hilo si no hay tareas pendientes
	if(mb_skcts->m_mbTasks.getCount() <= 0)
		Sender->suspend();
	//-- Se procesan todas las tareas pendientes
	while((mb_tsk = mb_skcts->m_mbTasks.getNextTask()) != NULL)
	{
		try
		{
			if(mb_skcts->sendRecv(mb_tsk) <= 0)
				mb_skcts->close(mb_tsk->socketID, mb_tsk->userData);
		}
		catch(...)
		{
			mb_skcts->close(mb_tsk->socketID, mb_tsk->userData);
		}
		mb_skcts->m_mbTasks.freeTask(mb_tsk);
	};
}
//-----------------------------------------------------------------------------

void TModbusSockets::clear()
{
    TModbusSocket* mb_sckt;

    while(!m_mapSockets.empty())
    {
        mb_sckt = (TModbusSocket*)((*m_mapSockets.begin()).second);
        delete mb_sckt;

        m_mapSockets.erase(m_mapSockets.begin());
    }
}
//-----------------------------------------------------------------------------

TModbusSocket* TModbusSockets::getSocket(short Socket)
{
    return (TModbusSocket*)m_mapSockets[Socket];
}
//-----------------------------------------------------------------------------

short TModbusSockets::getSocketId(TModbusSocket* msSocket)
{
	std::map<short, TModbusSocket*>::iterator m_iter;
    short s_rtn = -1;

    m_iter = m_mapSockets.begin();
    while(s_rtn < 0 && m_iter != m_mapSockets.end())
    {
        if((TModbusSocket*)((*m_iter).second) == msSocket)
        {
            s_rtn = (*m_iter).first;
        }
        m_iter++;
    }

    return s_rtn;
}
//-----------------------------------------------------------------------------

int TModbusSockets::sendRecv(MB_Task* mbTask)
{
    char           c_buffer[MB_MAX_SOCK_BUFFER_DATA_LEN];
    int            i_rtn    = 0, i_len = 0;
	TModbusSocket* ms_sckt  = getSocket(mbTask->socketID);

    if(ms_sckt != NULL)
    {
        i_len = 3 * sizeof(WORD) + ntohs(mbTask->task.standard.header.length);

		//-- Eviamos datos
		ms_sckt->setBuffer((char*)&mbTask->task.standard, i_len);
		ms_sckt->send();

		//-- Recivimos datos
		i_rtn = ms_sckt->receive(m_iTimeout);
		if(i_rtn > 0)
		{
			memset(c_buffer, 0, MB_MAX_SOCK_BUFFER_DATA_LEN);
			i_rtn = ms_sckt->getBuffer(c_buffer, MB_MAX_SOCK_BUFFER_DATA_LEN);
			if(i_rtn > 0 && onReceivedData != NULL)
				onReceivedData(mbTask->userData, mbTask->socketID, c_buffer, i_rtn);
		}
		else if(i_rtn == 0 || i_rtn == SOCKET_ERROR)
		{
			close(mbTask->socketID, mbTask->userData);
        }
	}

    return i_rtn;
}
//-----------------------------------------------------------------------------

short TModbusSockets::getFirstSocket()
{
    m_miSocket = m_mapSockets.begin();
    return m_miSocket == m_mapSockets.end() ? MB_NO_SOCKET : (*m_miSocket).first;
}
//-----------------------------------------------------------------------------

short TModbusSockets::getNextSocket()
{
    m_miSocket++;
    return m_miSocket == m_mapSockets.end() ? MB_NO_SOCKET : (*m_miSocket).first;
}
//-----------------------------------------------------------------------------

short TModbusSockets::open(const char* HostName, short Port)
{
    TModbusSocket* ms_sckt = new TModbusSocket;

    if(ms_sckt->open(HostName, Port) == true)
    {
		m_mapSockets[++m_sLastSocket] = ms_sckt;
		m_hThread.create(thrdProcess, this);
		return m_sLastSocket;
    }
	delete ms_sckt;
    return MB_NO_SOCKET;
}
//-----------------------------------------------------------------------------

void TModbusSockets::close(short Socket, void* User)
{
    TModbusSocket* ms_sckt = getSocket(Socket);
    std::map<short, TModbusSocket*>::iterator m_iter;

    if(ms_sckt != NULL)
    {
        m_iter = m_mapSockets.begin();
		while((TModbusSocket*)((*m_iter).second) != ms_sckt && m_iter != m_mapSockets.end())
			m_iter++;
		//-- Se notifica que este socket ha cerrado
		if(NULL != onSocketClose)
			onSocketClose(User, Socket);
		//-- Y se elimina
		delete ms_sckt;
		if(m_iter != m_mapSockets.end())
			m_mapSockets.erase(m_iter);
	}
}
//-----------------------------------------------------------------------------

short TModbusSockets::getSocketId(int Index)
{
    short s_rtn = MB_NO_SOCKET;
    int   i_ndx = 0;

    if(Index < 0 || Index >= int(count()))
        return MB_NO_SOCKET;

    s_rtn = getFirstSocket();
    while(i_ndx < Index)
    {
        s_rtn = getNextSocket();
        i_ndx++;
    }

    return s_rtn;
}
//-----------------------------------------------------------------------------

unsigned int TModbusSockets::count()
{
    return m_mapSockets.size();
}
//-----------------------------------------------------------------------------

char* TModbusSockets::getSocketHost(short Socket)
{
    TModbusSocket* ms_socket = getSocket(Socket);
    return ms_socket == NULL ? NULL : ms_socket->getHost();
}
//-----------------------------------------------------------------------------

short TModbusSockets::getSocketPort(short Socket)
{
    TModbusSocket* ms_socket = getSocket(Socket);
    return ms_socket == NULL ? 0 : ms_socket->getPort();
}
//-----------------------------------------------------------------------------

void TModbusSockets::addTask(void* UserData, short SocketID, MB_Std_Message* Message, short MaxRetries)
{
    MB_Task* mb_tsk = m_mbTasks.createTask(SocketID, MaxRetries);

    memcpy(&(mb_tsk->task.standard), Message, sizeof(MB_Std_Message));
    mb_tsk->userData = UserData;
    m_mbTasks.addTask(mb_tsk);

    m_hThread.resume();
}
//-----------------------------------------------------------------------------

void TModbusSockets::addTask(void* UserData, short SocketID, MB_New_Message* Message, short MaxRetries)
{
    MB_Task* mb_tsk = m_mbTasks.createTask(SocketID, MaxRetries);

    memcpy(&(mb_tsk->task.standard), Message, sizeof(MB_New_Message));
	mb_tsk->userData = UserData;
    m_mbTasks.addTask(mb_tsk);

    m_hThread.resume();
}
//-----------------------------------------------------------------------------

void TModbusSockets::setTimeout(int Seconds)
{
    m_iTimeout = Seconds;
}
//-----------------------------------------------------------------------------



TModbusSockets* TModbus::m_msSockets   = NULL;
unsigned int    TModbus::m_uiInstances = 0;
unsigned int    TModbus::m_uiNextID    = 0;
//-----------------------------------------------------------------------------
*/
