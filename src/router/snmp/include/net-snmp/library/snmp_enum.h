#ifndef SNMP_ENUM_H
#define SNMP_ENUM_H

#ifdef __cplusplus
extern          "C" {
#endif

    struct snmp_enum_list {
        struct snmp_enum_list *next;
        int             value;
        char           *label;
    };

#define SE_MAX_IDS 5
#define SE_MAX_SUBIDS 32        /* needs to be a multiple of 8 */

    /*
     * begin storage definitions 
     */
    /*
     * These definitions correspond with the "storid" argument to the API 
     */
#define SE_LIBRARY_ID     0
#define SE_MIB_ID         1
#define SE_APPLICATION_ID 2
#define SE_ASSIGNED_ID    3

    /*
     * library specific enum locations 
     */

    /*
     * error codes 
     */
#define SE_OK            0
#define SE_NOMEM         1
#define SE_ALREADY_THERE 2
#define SE_DNE           -2

    int             init_snmp_enum(void);
    struct snmp_enum_list *se_find_list(unsigned int major,
                                        unsigned int minor);
    int             se_store_list(struct snmp_enum_list *,
                                  unsigned int major, unsigned int minor);
    struct snmp_enum_list *se_find_list(unsigned int major,
                                        unsigned int minor);
    int             se_find_value(unsigned int major, unsigned int minor,
                                  char *label);
    char           *se_find_label(unsigned int major, unsigned int minor,
                                  int value);
    int             se_add_pair(unsigned int major, unsigned int minor,
                                char *label, int value);

    /*
     * finds a list of enums in a list of enum structs associated by a name. 
     */
    /*
     * not as fast as the above routines, since two lists must be traversed. 
     */
    char           *se_find_label_in_slist(const char *listname,
                                           int value);
    int             se_find_value_in_slist(const char *listname,
                                           char *label);
    int             se_add_pair_to_slist(const char *listname, char *label,
                                         int value);

    /*
     * operates directly on a possibly external list 
     */
    char           *se_find_label_in_list(struct snmp_enum_list *list,
                                          int value);
    int             se_find_value_in_list(struct snmp_enum_list *list,
                                          char *label);
    int             se_add_pair_to_list(struct snmp_enum_list **list,
                                        char *label, int value);


#ifdef __cplusplus
}
#endif
#endif                          /* SNMP_ENUM_H */
