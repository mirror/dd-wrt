/*
 *    rfc1157_snmp.h
 *
 *    "RFC1157-SNMP" ASN.1 module C type definitions and prototypes
 *
 *    This .h file was by snacc on Fri Jan 31 14:49:02 1997
 *
 *    UBC snacc written compiler by Mike Sample
 *
 *    NOTE: This is a machine generated file - editing not recommended
 */


#ifndef _rfc1157_snmp_h_
#define _rfc1157_snmp_h_





#define COLDSTART 0
#define WARMSTART 1
#define LINKDOWN 2
#define LINKUP 3
#define AUTHENTICATIONFAILURE 4
#define EGPNEIGHBORLOSS 5
#define ENTERPRISESPECIFIC 6

typedef AsnInt Trap_PDUInt; /* INTEGER { COLDSTART(0), WARMSTART(1), LINKDOWN(2), LINKUP(3), AUTHENTICATIONFAILURE(4), EGPNEIGHBORLOSS(5), ENTERPRISESPECIFIC(6) }  */

#define BEncTrap_PDUIntContent BEncAsnIntContent

#define BDecTrap_PDUIntContent BDecAsnIntContent

#define PrintTrap_PDUInt PrintAsnInt





#define NOERROR 0
#define TOOBIG 1
#define NOSUCHNAME 2
#define BADVALUE 3
#define READONLY 4
#define GENERR 5

typedef AsnInt PDUInt; /* INTEGER { NOERROR(0), TOOBIG(1), NOSUCHNAME(2), BADVALUE(3), READONLY(4), GENERR(5) }  */

#define BEncPDUIntContent BEncAsnIntContent

#define BDecPDUIntContent BDecAsnIntContent

#define PrintPDUInt PrintAsnInt





#define VERSION_1 0

typedef AsnInt MessageInt; /* INTEGER { VERSION_1(0) }  */

#define BEncMessageIntContent BEncAsnIntContent

#define BDecMessageIntContent BDecAsnIntContent

#define PrintMessageInt PrintAsnInt




typedef struct VarBind /* SEQUENCE */
{
    ObjectName name; /* ObjectName */
    struct ObjectSyntax* value; /* ObjectSyntax */
} VarBind;

AsnLen BEncVarBindContent PROTO((BUF_TYPE b, VarBind* v));

void BDecVarBindContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, VarBind* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintVarBind PROTO((FILE* f, VarBind* v, unsigned short int indent));



typedef AsnList VarBindList; /* SEQUENCE OF VarBind */

AsnLen BEncVarBindListContent PROTO((BUF_TYPE b, VarBindList* v));

void BDecVarBindListContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, VarBindList* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintVarBindList PROTO((FILE* f, VarBindList* v, unsigned short int indent));



typedef struct PDU /* SEQUENCE */
{
    AsnInt request_id; /* INTEGER */
    PDUInt error_status; /* PDUInt */
    AsnInt error_index; /* INTEGER */
    VarBindList* variable_bindings; /* VarBindList */
} PDU;

AsnLen BEncPDUContent PROTO((BUF_TYPE b, PDU* v));

void BDecPDUContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, PDU* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintPDU PROTO((FILE* f, PDU* v, unsigned short int indent));



typedef struct Trap_PDU /* [4] IMPLICIT SEQUENCE */
{
    AsnOid enterprise; /* OBJECT IDENTIFIER */
    struct NetworkAddress* agent_addr; /* NetworkAddress */
    Trap_PDUInt generic_trap; /* Trap-PDUInt */
    AsnInt specific_trap; /* INTEGER */
    TimeTicks time_stamp; /* TimeTicks */
    VarBindList* variable_bindings; /* VarBindList */
} Trap_PDU;

AsnLen BEncTrap_PDUContent PROTO((BUF_TYPE b, Trap_PDU* v));

void BDecTrap_PDUContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, Trap_PDU* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintTrap_PDU PROTO((FILE* f, Trap_PDU* v, unsigned short int indent));



typedef struct PDU GetRequest_PDU; /* [0] IMPLICIT PDU */

#define BEncGetRequest_PDUContent BEncPDUContent

#define BDecGetRequest_PDUContent BDecPDUContent

#define PrintGetRequest_PDU PrintPDU




typedef struct PDU GetNextRequest_PDU; /* [1] IMPLICIT PDU */

#define BEncGetNextRequest_PDUContent BEncPDUContent

#define BDecGetNextRequest_PDUContent BDecPDUContent

#define PrintGetNextRequest_PDU PrintPDU




typedef struct PDU GetResponse_PDU; /* [2] IMPLICIT PDU */

#define BEncGetResponse_PDUContent BEncPDUContent

#define BDecGetResponse_PDUContent BDecPDUContent

#define PrintGetResponse_PDU PrintPDU




typedef struct PDU SetRequest_PDU; /* [3] IMPLICIT PDU */

#define BEncSetRequest_PDUContent BEncPDUContent

#define BDecSetRequest_PDUContent BDecPDUContent

#define PrintSetRequest_PDU PrintPDU




typedef struct PDUs /* CHOICE */
{
    enum PDUsChoiceId
    {
        PDUS_GET_REQUEST,
        PDUS_GET_NEXT_REQUEST,
        PDUS_GET_RESPONSE,
        PDUS_SET_REQUEST,
        PDUS_TRAP
    } choiceId;
    union PDUsChoiceUnion
    {
    GetRequest_PDU* get_request; /* GetRequest-PDU */
    GetNextRequest_PDU* get_next_request; /* GetNextRequest-PDU */
    GetResponse_PDU* get_response; /* GetResponse-PDU */
    SetRequest_PDU* set_request; /* SetRequest-PDU */
    struct Trap_PDU* trap; /* Trap-PDU */
    } a;
} PDUs;

AsnLen BEncPDUsContent PROTO((BUF_TYPE b, PDUs* v));

void BDecPDUsContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, PDUs* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintPDUs PROTO((FILE* f, PDUs* v, unsigned short int indent));



typedef struct Message /* SEQUENCE */
{
    MessageInt version; /* MessageInt */
    AsnOcts community; /* OCTET STRING */
    struct PDUs* data; /* PDUs */
} Message;

AsnLen BEncMessage PROTO((BUF_TYPE b, Message* v));

void BDecMessage PROTO(( BUF_TYPE b, Message* result, AsnLen* bytesDecoded, ENV_TYPE env));
AsnLen BEncMessageContent PROTO((BUF_TYPE b, Message* v));

void BDecMessageContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, Message* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintMessage PROTO((FILE* f, Message* v, unsigned short int indent));




#endif /* conditional include of rfc1157_snmp.h */
