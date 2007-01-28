/*
 * auth_ntlm.h
 * Internal declarations for the ntlm auth module
 */

#ifndef __AUTH_NTLM_H__
#define __AUTH_NTLM_H__

#define DefaultAuthenticateChildrenMax  32	/* 32 processes */

typedef enum {
    AUTHENTICATE_STATE_NONE,
    AUTHENTICATE_STATE_INITIAL,
    AUTHENTICATE_STATE_NEGOTIATE,
    AUTHENTICATE_STATE_FINISHED,
    AUTHENTICATE_STATE_DONE,
    AUTHENTICATE_STATE_FAILED
} auth_state_t;			/* connection level auth state */

/* Generic */
typedef struct {
    void *data;
    auth_user_request_t *auth_user_request;
    RH *handler;
} authenticateStateData;

struct _ntlm_user {
    /* what username did this connection get? */
    char *username;
};

struct _ntlm_request {
    /*we need to store the helper server between requests */
    helper_stateful_server *authserver;
    /* how far through the authentication process are we? */
    auth_state_t auth_state;
    /* currently waiting for helper response */
    int waiting;
    /* what connection is this associated with */
    ConnStateData *conn;
    /* our current blob to pass to the client */
    char *server_blob;
    /* our current blob to pass to the server */
    char *client_blob;
};

/* configuration runtime data */
struct _auth_ntlm_config {
    int authenticateChildren;
    int keep_alive;
    wordlist *authenticate;
};

typedef struct _ntlm_user ntlm_user_t;
typedef struct _ntlm_request ntlm_request_t;
typedef struct _auth_ntlm_config auth_ntlm_config;

#endif
