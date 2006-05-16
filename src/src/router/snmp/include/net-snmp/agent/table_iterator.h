/*
 * table_iterator.h 
 */
#ifndef _TABLE_ITERATOR_HANDLER_H_
#define _TABLE_ITERATOR_HANDLER_H_

#ifdef __cplusplus
extern          "C" {
#endif

    struct netsnmp_iterator_info_s;

    typedef netsnmp_variable_list *(Netsnmp_First_Data_Point) (void
                                                               **loop_context,
                                                               void
                                                               **data_context,
                                                               netsnmp_variable_list
                                                               *,
                                                               struct
                                                               netsnmp_iterator_info_s
                                                               *);
    typedef netsnmp_variable_list *(Netsnmp_Next_Data_Point) (void
                                                              **loop_context,
                                                              void
                                                              **data_context,
                                                              netsnmp_variable_list
                                                              *,
                                                              struct
                                                              netsnmp_iterator_info_s
                                                              *);
    typedef void   *(Netsnmp_Make_Data_Context) (void *loop_context,
                                                 struct
                                                 netsnmp_iterator_info_s
                                                 *);
    typedef void    (Netsnmp_Free_Loop_Context) (void *,
                                                 struct
                                                 netsnmp_iterator_info_s
                                                 *);
    typedef void    (Netsnmp_Free_Data_Context) (void *,
                                                 struct
                                                 netsnmp_iterator_info_s
                                                 *);

    typedef struct netsnmp_iterator_info_s {
        Netsnmp_First_Data_Point *get_first_data_point;
        Netsnmp_Next_Data_Point *get_next_data_point;
        Netsnmp_Make_Data_Context *make_data_context;
        Netsnmp_Free_Loop_Context *free_loop_context;
        Netsnmp_Free_Data_Context *free_data_context;
        Netsnmp_Free_Loop_Context *free_loop_context_at_end;

        void           *myvoid;

        netsnmp_table_registration_info *table_reginfo;
    } netsnmp_iterator_info;

#define TABLE_ITERATOR_NAME "table_iterator"

    netsnmp_mib_handler
        *netsnmp_get_table_iterator_handler(netsnmp_iterator_info *iinfo);
    int            
        netsnmp_register_table_iterator(netsnmp_handler_registration
                                        *reginfo,
                                        netsnmp_iterator_info *iinfo);

    void           *netsnmp_extract_iterator_context(netsnmp_request_info
                                                     *);

    Netsnmp_Node_Handler netsnmp_table_iterator_helper_handler;

#ifdef __cplusplus
};
#endif

#endif                          /* _TABLE_ITERATOR_HANDLER_H_ */
