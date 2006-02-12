#include <net-snmp/net-snmp-config.h>

#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/library/check_varbind.h>



int
netsnmp_check_vb_type_and_size(netsnmp_variable_list *var,
                               int type, size_t size)
{
    register int rc = SNMP_ERR_NOERROR;

    if (NULL == var)
        return SNMP_ERR_GENERR;
    
    if (var->type != type) {
        rc = SNMP_ERR_WRONGTYPE;
    } else if (var->val_len != size) {
        rc = SNMP_ERR_WRONGLENGTH;
    }

    return rc;
}

int
netsnmp_check_vb_int_range(netsnmp_variable_list *var, int low, int high)
{
    register int rc = SNMP_ERR_NOERROR;
    
    if ((rc = netsnmp_check_vb_type_and_size(var, ASN_INTEGER, sizeof(int))))
        return rc;
    
    if ((*var->val.integer < low) || (*var->val.integer > high)) {
        rc = SNMP_ERR_BADVALUE;
    }

    return rc;
}

int
netsnmp_check_vb_truthvalue(netsnmp_variable_list *var)
{
    register int rc = SNMP_ERR_NOERROR;
    
    if ((rc = netsnmp_check_vb_type_and_size(var, ASN_INTEGER, sizeof(int))))
        return rc;
    
    return netsnmp_check_vb_int_range(var, 1, 2);
}

int
netsnmp_check_vb_rowstatus(netsnmp_variable_list *var, int old_value)
{
    register int rc = SNMP_ERR_NOERROR;

    if ((rc = netsnmp_check_vb_type_and_size(var, ASN_INTEGER, sizeof(int))))
        return rc;
    
    if ((rc = netsnmp_check_vb_int_range(var, SNMP_ROW_NONEXISTENT,
                                         SNMP_ROW_DESTROY)))
        return rc;

    return check_rowstatus_transition(old_value, *var->val.integer);
}

int
netsnmp_check_vb_storagetype(netsnmp_variable_list *var, int old_value)
{
    int rc = SNMP_ERR_NOERROR;

    if ((rc = netsnmp_check_vb_type_and_size(var, ASN_INTEGER, sizeof(int))))
        return rc;
    
    if ((rc = netsnmp_check_vb_int_range(var, SNMP_STORAGE_NONE,
                                        SNMP_STORAGE_READONLY)))
        return rc;
        
    return check_storage_transition(old_value, *var->val.integer);
}
