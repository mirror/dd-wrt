#ifndef NETSNMP_FACTORY_H
#define NETSNMP_FACTORY_H

#ifdef __cplusplus
extern "C" {
#elif 0
}
#endif

typedef struct netsnmp_factory_s {
    /*
     * a string describing the product the factory creates
     */
    const char                           *product;

    /*
     * a function to allocate a new container
     */
    netsnmp_container *                 (*produce)(void);
} netsnmp_factory;

#ifdef __cplusplus
}
#endif

#endif /* NETSNMP_FACTORY_H */
