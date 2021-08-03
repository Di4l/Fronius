//-----------------------------------------------------------------------------
#ifndef __MODBUS_DEFINES_HPP__
#define __MODBUS_DEFINES_HPP__
//-----------------------------------------------------------------------------
#define PROTOCOL_MODBUS						0x0000
#define MB_PORT								502
//-----------------------------------------------------------------------------
#define MB_ALL_SOCKETS                      -1
#define MB_NO_SOCKET                        -1
#define MB_MAX_SOCK_BUFFER_DATA_LEN			253
#define MB_MAX_STANDARD_DATA_LEN			253
#define MB_MAX_FRAGMENT_DATA_LEN			190
//-----------------------------------------------------------------------------

/*-- MODBUS COMMON FLAGS --*/
#define MB_MSG_EXCEPTION					0x80
//-----------------------------------------------------------------------------

/*-- MODBUS FRAGMENT FLAGS --*/
#define MB_IS_FRAGMENT						0x80
#define MB_LAST_FRAGMENT					0x40
#define MB_SEQ_FRAGMENT				    	0x07
//-----------------------------------------------------------------------------

/*-- MODBUS FUNCTIONS --*/
#define MB_FUNC_READ_COILS					0x01
#define MB_FUNC_READ_DISCRETE_INPUTS		0x02
#define MB_FUNC_READ_HOLDING_REGISTERS		0x03
#define MB_FUNC_READ_INPUT_REGISTERS		0x04
#define MB_FUNC_WRITE_SINGLE_COIL			0x05
#define MB_FUNC_WRITE_SINGLE_REGISTER		0x06
#define MB_FUNC_READ_EXCEPTION_STATUS		0x07	//-- Serial Line only
#define MB_FUNC_DIAGNOSTICS					0x08	//-- Serial Line only
#define MB_FUNC_GET_COMM_EVENT_COUNTER		0x0B	//-- Serial Line only
#define MB_FUNC_GET_COMM_EVENT_LOG			0x0C	//-- Serial Line only
#define MB_FUNC_WRITE_MULTIPLE_COILS		0x0F
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS	0x10
#define MB_FUNC_REPORT_SLAVE_ID				0x11	//-- Serial Line only
#define MB_FUNC_READ_FILE_RECORD			0x14
#define MB_FUNC_WRITE_FILE_RECORD			0x15
#define MB_FUNC_MASK_WRITE_REGISTER			0x16
#define MB_FUNC_RD_WR_MULTIPLE_REGISTERS	0x17
#define MB_FUNC_READ_FIFO_QUEUE				0x18
#define MB_FUNC_ENCAPSULATED_NTRFC_TRNSPRT	0x2B	//-- Encapsulated Interface Transport
//-----------------------------------------------------------------------------

/*-- MODBUS ENCAPSULATION PROTOCOL SUB-MESSAGES --*/
/*-- 		(THEY ALL USE FUNCTION 0x2B)		--*/
#define MB_MEI_CAN_GENERAL_REF_REQ_RES		0x0D	//-- CANopen General Reference Request and Response PDU
#define MB_MEI_READ_DEVICE_IDENTIFICATION	0x0E
//-----------------------------------------------------------------------------

/*-- MODBUS DIAGNOSTIC (FUNC 0x14 & FUNC 0x15) SUBFUNCTIONS --*/
#define MB_SUBFUNC_REF_TYPE_GROUP           0x06
//-----------------------------------------------------------------------------

/*-- MODBUS DIAGNOSTIC (FUNC 0x08) SUBFUNCTIONS (Serial Line only) --*/
#define MB_SUBFUNC_RTRN_QUERY_DATA			0x00	//-- Return Query Data
#define MB_SUBFUNC_RSTRT_COMM_OPTIONS		0x01	//-- Restart Communications Option
#define MB_SUBFUNC_RTRN_DIAG_REGISTER		0x02	//-- Return Diagnostic Register
#define MB_SUBFUNC_CHNG_ASCII_INPUT_DELIM	0x03	//-- Change ASCII Input Delimiter
#define MB_SUBFUNC_FORCE_LISTEN_ONLY		0x04	//-- Force Listen Only Mode
#define MB_SUBFUNC_CLR_CNTRS_AND_DIAG_REG	0x0A	//-- Clear Counters and Diagnostic Register
#define MB_SUBFUNC_RTRN_BUS_MSSG_CNT		0x0B	//-- Return Bus Message Count
#define MB_SUBFUNC_RTRN_BUS_COMM_ERR_CNT	0x0C	//-- Return Bus Communication Error Count
#define MB_SUBFUNC_RTRN_BUS_EXCP_ERR_CNT	0x0D	//-- Return Bus Exception Error Count
#define MB_SUBFUNC_RTRN_SLV_MSSG_CNT		0x0E	//-- Return Slave Message Count
#define MB_SUBFUNC_RTRN_SLV_NO_RSPNS_CNT	0x0F	//-- Return Slave No Response Count
#define MB_SUBFUNC_RTRN_SLV_NAK_CNT			0x10	//-- Return Slave NAK Count
#define MB_SUBFUNC_RTRN_SLV_BUSY_CNT		0x11	//-- Return Slave Busy Count
#define MB_SUBFUNC_RTRN_BUS_CHR_OVRUN_CNT	0x12	//-- Return Bus Character Overrun Count
#define MB_SUBFUNC_CLR_OVRUN_CNTR_AND_FLAG	0x14	//-- Clear Overrun Counter and Flag
//-----------------------------------------------------------------------------

/*-- MODBUS EXCEPTIONS --*/
#define MB_XCPTN_ILLEGAL_FUNCTION			0x01
#define MB_XCPTN_ILLEGAL_DATA_ADDRESS		0x02
#define MB_XCPTN_ILLEGAL_DATA_VALUE			0x03
#define MB_XCPTN_SLAVE_DEVICE_FAILURE		0x04
#define MB_XCPTN_ACKNOWLEDGE				0x05
#define MB_XCPTN_SLAVE_DEVICE_BUSY			0x06
#define MB_XCPTN_MEMORY_PARITY_ERROR		0x08
#define MB_XCPTN_GWAY_PATH_UNAVAILABLE		0x0A	//-- Gateway Path Unavailable
#define MB_XCPTN_GWAY_TGT_DVC_RESPONSE_FAIL	0x0B	//-- Gateway Target Device Failed to Respond
//-----------------------------------------------------------------------------
#endif
