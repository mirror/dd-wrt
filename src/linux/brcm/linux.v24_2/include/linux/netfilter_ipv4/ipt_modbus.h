#ifndef _IPT_MODBUS_H
#define _IPT_MODBUS_H


/* Structure we use for storing the user values */
struct ipt_modbus
{

  u_int16_t t_id;
  u_int16_t p_id;
  u_int16_t length[2];
  
  u_int8_t unit_id;
  u_int8_t func_code[2];
  u_int16_t ref_num;
  u_int16_t word_cnt;
  
  /* Storing inverse flags for the three arguments supported */
  u_int8_t  invflags_funccode;
  u_int8_t  invflags_unitid;
  u_int8_t  invflags_refnum;
  u_int8_t  invflags_length;

  int funccode_flags;
  int unitid_flags;
  int refnum_flags;
  int length_flags;
  int allow_tcp;
};

#define IPT_MODBUS_INV_FUNCCODE  0x01
#define IPT_MODBUS_INV_UNITID    0x01
#define IPT_MODBUS_INV_REFNUM    0x01
#define IPT_MODBUS_INV_LENGTH    0x01

#endif /*_IPT_MODBUS_H*/

