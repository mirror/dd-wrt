#ifndef EXPVALUETABLE_H
#define EXPVALUETABLE_H

/*
 * function declarations 
 */
void              init_expValueTable(void);
void              shutdown_expValueTable(void);

/*
 * column number definitions for table expValueTable 
 */
#define COLUMN_EXPVALUEINSTANCE			1
#define COLUMN_EXPVALUECOUNTER32VAL		2
#define COLUMN_EXPVALUEUNSIGNED32VAL		3
#define COLUMN_EXPVALUETIMETICKSVAL		4
#define COLUMN_EXPVALUEINTEGER32VAL		5
#define COLUMN_EXPVALUEIPADDRESSVAL		6
#define COLUMN_EXPVALUEOCTETSTRINGVAL		7
#define COLUMN_EXPVALUEOIDVAL			8
#define COLUMN_EXPVALUECOUNTER64VAL		9
#endif                          /* EXPVALUETABLE_H */
