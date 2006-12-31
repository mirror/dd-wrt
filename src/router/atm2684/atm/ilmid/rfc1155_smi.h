/*
 *    rfc1155_smi.h
 *
 *    "RFC1155-SMI" ASN.1 module C type definitions and prototypes
 *
 *    This .h file was by snacc on Fri Jan 31 14:49:02 1997
 *
 *    UBC snacc written compiler by Mike Sample
 *
 *    NOTE: This is a machine generated file - editing not recommended
 */


#ifndef _rfc1155_smi_h_
#define _rfc1155_smi_h_




typedef AsnOid ObjectName; /* OBJECT IDENTIFIER */

#define BEncObjectNameContent BEncAsnOidContent

#define BDecObjectNameContent BDecAsnOidContent

#define PrintObjectName PrintAsnOid




typedef AsnOcts IpAddress; /* [APPLICATION 0] IMPLICIT OCTET STRING (SIZE (4)) */

#define BEncIpAddressContent BEncAsnOctsContent

#define BDecIpAddressContent BDecAsnOctsContent

#define PrintIpAddress PrintAsnOcts




typedef AsnInt Counter; /* [APPLICATION 1] IMPLICIT INTEGER (0..2147483647) */

#define BEncCounterContent BEncAsnIntContent

#define BDecCounterContent BDecAsnIntContent

#define PrintCounter PrintAsnInt




typedef AsnInt Gauge; /* [APPLICATION 2] IMPLICIT INTEGER (0..2147483647) */

#define BEncGaugeContent BEncAsnIntContent

#define BDecGaugeContent BDecAsnIntContent

#define PrintGauge PrintAsnInt




typedef AsnInt TimeTicks; /* [APPLICATION 3] IMPLICIT INTEGER (0..2147483647) */

#define BEncTimeTicksContent BEncAsnIntContent

#define BDecTimeTicksContent BDecAsnIntContent

#define PrintTimeTicks PrintAsnInt




typedef AsnOcts Opaque; /* [APPLICATION 4] IMPLICIT OCTET STRING */

#define BEncOpaqueContent BEncAsnOctsContent

#define BDecOpaqueContent BDecAsnOctsContent

#define PrintOpaque PrintAsnOcts




typedef struct SimpleSyntax /* CHOICE */
{
    enum SimpleSyntaxChoiceId
    {
        SIMPLESYNTAX_NUMBER,
        SIMPLESYNTAX_STRING,
        SIMPLESYNTAX_OBJECT,
        SIMPLESYNTAX_EMPTY
    } choiceId;
    union SimpleSyntaxChoiceUnion
    {
    AsnInt number; /* INTEGER */
    AsnOcts* string; /* OCTET STRING */
    AsnOid* object; /* OBJECT IDENTIFIER */
    AsnNull empty; /* NULL */
    } a;
} SimpleSyntax;

AsnLen BEncSimpleSyntaxContent PROTO((BUF_TYPE b, SimpleSyntax* v));

void BDecSimpleSyntaxContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, SimpleSyntax* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintSimpleSyntax PROTO((FILE* f, SimpleSyntax* v, unsigned short int indent));



typedef struct NetworkAddress /* CHOICE */
{
    enum NetworkAddressChoiceId
    {
        NETWORKADDRESS_INTERNET
    } choiceId;
    union NetworkAddressChoiceUnion
    {
    IpAddress* internet; /* IpAddress */
    } a;
} NetworkAddress;

AsnLen BEncNetworkAddressContent PROTO((BUF_TYPE b, NetworkAddress* v));

void BDecNetworkAddressContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, NetworkAddress* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintNetworkAddress PROTO((FILE* f, NetworkAddress* v, unsigned short int indent));



typedef struct ApplicationSyntax /* CHOICE */
{
    enum ApplicationSyntaxChoiceId
    {
        APPLICATIONSYNTAX_ADDRESS,
        APPLICATIONSYNTAX_COUNTER,
        APPLICATIONSYNTAX_GAUGE,
        APPLICATIONSYNTAX_TICKS,
        APPLICATIONSYNTAX_ARBITRARY
    } choiceId;
    union ApplicationSyntaxChoiceUnion
    {
    struct NetworkAddress* address; /* NetworkAddress */
    Counter counter; /* Counter */
    Gauge gauge; /* Gauge */
    TimeTicks ticks; /* TimeTicks */
    Opaque* arbitrary; /* Opaque */
    } a;
} ApplicationSyntax;

AsnLen BEncApplicationSyntaxContent PROTO((BUF_TYPE b, ApplicationSyntax* v));

void BDecApplicationSyntaxContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, ApplicationSyntax* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintApplicationSyntax PROTO((FILE* f, ApplicationSyntax* v, unsigned short int indent));



typedef struct ObjectSyntax /* CHOICE */
{
    enum ObjectSyntaxChoiceId
    {
        OBJECTSYNTAX_SIMPLE,
        OBJECTSYNTAX_APPLICATION_WIDE
    } choiceId;
    union ObjectSyntaxChoiceUnion
    {
    struct SimpleSyntax* simple; /* SimpleSyntax */
    struct ApplicationSyntax* application_wide; /* ApplicationSyntax */
    } a;
} ObjectSyntax;

AsnLen BEncObjectSyntaxContent PROTO((BUF_TYPE b, ObjectSyntax* v));

void BDecObjectSyntaxContent PROTO(( BUF_TYPE b, AsnTag tagId0, AsnLen elmtLen0, ObjectSyntax* v, AsnLen* bytesDecoded, ENV_TYPE env));


void PrintObjectSyntax PROTO((FILE* f, ObjectSyntax* v, unsigned short int indent));



extern AsnOid internet;
extern AsnOid directory;
extern AsnOid mgmt;
extern AsnOid experimental;
extern AsnOid private;
extern AsnOid enterprises;

#endif /* conditional include of rfc1155_smi.h */
