#include <libubox/usock.h>
#include <arpa/inet.h>
#include <inttypes.h>

#include "memory_utils.h"
#include "msghandler.h"
#include "crypto.h"
#include "datastorage.h"
#include "tcpsocket.h"

#define STR_EVAL(x) #x
#define STR_QUOTE(x) STR_EVAL(x)

#define HEADER_SIZE sizeof(uint32_t)
#define PING_STR "ping"
#define PONG_STR "pong"
#define PING_SIZE (strlen(PING_STR)+1)
#define PONG_SIZE (strlen(PONG_STR)+1)

LIST_HEAD(tcp_sock_list);
LIST_HEAD(cli_list);

struct network_con_s *tcp_list_contains_address(struct sockaddr_in entry);

static struct uloop_fd server;
static struct client *next_client = NULL; // TODO: Why here? Only used in sever_cb()

enum socket_read_status {
    READ_STATUS_READY,
    READ_STATUS_COMMENCED,
    READ_STATUS_COMPLETE
};

struct client {
    struct list_head list;
    struct sockaddr_in sin;

    struct ustream_fd s;
    int ctr;
    int counter;
    char *str; // message buffer
    enum socket_read_status state; // messge read state
    uint32_t final_len; // full message length
    uint32_t curr_len; // bytes read so far
    time_t time_alive;
};


static void client_close(struct ustream *s) {
    dawnlog_debug_func("Entering...");

    struct client *cl = container_of(s, struct client, s.stream);

    dawnlog_warning("Connection closed\n");
    ustream_free(s);
    dawn_unregmem(s);
    close(cl->s.fd.fd);
    list_del(&cl->list);
    dawn_free(cl);
    cl = NULL;
}

static void client_notify_write(struct ustream *s, int bytes) {
    return;
}


// FIXME: This void function tries to return a value sometimes...
static void client_notify_state(struct ustream *s) {
    dawnlog_debug_func("Entering...");

    struct client *cl = container_of(s, struct client, s.stream);
    if (!s->write_error && !s->eof)
        return;

    dawnlog_error("Closing client-connection, pending: %d, total: %d\n", s->w.data_bytes, cl->ctr);
    client_close(s);
}

static void client_to_server_close(struct ustream *s) {
    struct network_con_s *con = container_of(s, struct network_con_s, stream.stream);

    dawnlog_debug_func("Entering...");

    dawnlog_warning("Connection to server closed\n");
    ustream_free(s);
    dawn_unregmem(s);

    close(con->fd.fd);
    list_del(&con->list);
    dawn_free(con);
    con = NULL;
}

static void client_to_server_state(struct ustream *s) {
    struct client *cl = container_of(s, struct client, s.stream);

    dawnlog_debug_func("Entering...");

    if (!s->write_error && !s->eof)
        return;

    dawnlog_error("Closing connection, pending: %d, total: %d\n", s->w.data_bytes, cl->ctr);
    client_to_server_close(s);
}

static void client_read_cb(struct ustream *s, int bytes) {
    struct client *cl = container_of(s, struct client, s.stream);

    dawnlog_debug_func("Entering...");

    while(1) {
        if (cl->state == READ_STATUS_READY)
        {
            dawnlog_debug("tcp_socket: commencing message...\n");
            cl->str = dawn_malloc(HEADER_SIZE);
            if (!cl->str) {
                dawnlog_error("not enough memory (" STR_QUOTE(__LINE__) ")\n");
                break;
            }

            uint32_t avail_len = ustream_pending_data(s, false);

            if (avail_len < HEADER_SIZE){//ensure recv sizeof(uint32_t)
                dawnlog_debug("not complete msg, len:%d\n", avail_len);
                dawn_free(cl->str);
                cl->str = NULL;
                break;
            }

            if (ustream_read(s, cl->str, HEADER_SIZE) != HEADER_SIZE) // read msg length bytes
            {
                dawnlog_error("msg length read failed\n");
                dawn_free(cl->str);
                cl->str = NULL;
                break;
            }

            cl->curr_len += HEADER_SIZE;
            cl->final_len = ntohl(*(uint32_t *)cl->str);

            // On failure, dawn_realloc returns a null pointer. The original pointer str
            // remains valid and may need to be deallocated.
            char *str_tmp = dawn_realloc(cl->str, cl->final_len);
            if (!str_tmp) {
                dawnlog_error("not enough memory (%" PRIu32 " @ " STR_QUOTE(__LINE__) ")\n", cl->final_len);
                dawn_free(cl->str);
                cl->str = NULL;
                break;
            }

            cl->str = str_tmp;
            str_tmp = NULL; // Aboutt o go out of scope, but just in case it gets moved around...
            cl->state = READ_STATUS_COMMENCED;
        }

        if (cl->state == READ_STATUS_COMMENCED)
        {
            dawnlog_debug("tcp_socket: reading message...\n");
            uint32_t read_len = ustream_pending_data(s, false);

            if (read_len == 0)
                break;

            if (read_len > (cl->final_len - cl->curr_len))
                    read_len = cl->final_len - cl->curr_len;

            dawnlog_debug("tcp_socket: reading %" PRIu32 " bytes to add to %" PRIu32 " of %" PRIu32 "...\n",
                    read_len, cl->curr_len, cl->final_len);

            uint32_t this_read = ustream_read(s, cl->str + cl->curr_len, read_len);
            cl->curr_len += this_read;
            dawnlog_debug("tcp_socket: ...and we're back, now have %" PRIu32 " bytes\n", cl->curr_len);
            if (cl->curr_len == cl->final_len){//ensure recv final_len bytes.
                // Full message now received
                cl->state = READ_STATUS_COMPLETE;
                dawnlog_debug("tcp_socket: message completed\n");
            }
        }

        if (cl->state == READ_STATUS_COMPLETE)
        {
            dawnlog_debug("tcp_socket: processing message...\n");

            /* received pong */
            if (cl->final_len == HEADER_SIZE + PONG_SIZE && memcmp(cl->str + HEADER_SIZE, PONG_STR, PONG_SIZE) == 0) {
                dawnlog_info("Server: received pong from %s:%u, now=%d\n", inet_ntoa(cl->sin.sin_addr), ntohs(cl->sin.sin_port), (int)time(0));
                goto process_done;
            }

            if (network_config.use_symm_enc) {
                char *dec = gcrypt_decrypt_msg(cl->str + HEADER_SIZE, cl->final_len - HEADER_SIZE);//len of str is final_len
                if (!dec) {
                    dawnlog_error("not enough memory (" STR_QUOTE(__LINE__) ")\n");
                    dawn_free(cl->str);
                    cl->str = NULL;
                    break;
                }
                handle_network_msg(dec);
                dawn_free(dec);
                dec = NULL;
            } else {
                handle_network_msg(cl->str + HEADER_SIZE);//len of str is final_len
            }

process_done:
            cl->state = READ_STATUS_READY;
            cl->curr_len = 0;
            cl->final_len = 0;
            dawn_free(cl->str);
            cl->str = NULL;
            cl->time_alive = time(0);
        }
    }

    dawnlog_debug("tcp_socket: leaving\n");
    return;
}

static void server_cb(struct uloop_fd *fd, unsigned int events) {
    struct client *cl; //MUSTDO: check free() of this
    unsigned int sl = sizeof(struct sockaddr_in);
    int sfd;

    dawnlog_debug_func("Entering...");

    if (!next_client)
        next_client = dawn_calloc(1, sizeof(*next_client));

    cl = next_client;

    sfd = accept(server.fd, (struct sockaddr *) &cl->sin, &sl);
    if (sfd < 0) {
        dawnlog_error("Accept failed\n");
        return;
    }

    cl->s.stream.string_data = 1;
    cl->s.stream.notify_read = client_read_cb;
    cl->s.stream.notify_state = client_notify_state;
    cl->s.stream.notify_write = client_notify_write;
    cl->time_alive = time(0);
    list_add(&cl->list, &cli_list);
    ustream_fd_init(&cl->s, sfd);
    dawn_regmem(&cl->s);
    next_client = NULL;  // TODO: Why is this here?  To avoid resetting if above return happens?
    dawnlog_info("New connection\n");
}

int run_server(int port) {
    dawnlog_debug("Adding socket!\n");
    char port_str[12];
    sprintf(port_str, "%d", port); // TODO: Manage buffer length

    server.cb = server_cb;
    server.fd = usock(USOCK_TCP | USOCK_SERVER | USOCK_IPV4ONLY | USOCK_NUMERIC, INADDR_ANY, port_str);
    if (server.fd < 0) {
        dawnlog_perror("usock");
        return 1;
    }

    uloop_fd_add(&server, ULOOP_READ);

    return 0;
}

static void client_ping_read_cb(struct ustream *s, int bytes) {
    int len;
    char buf[2048];
    uint32_t ping_len = HEADER_SIZE + PING_SIZE;

    dawnlog_debug_func("Entering...");

    len = ustream_read(s, buf, ping_len);

    /* client received ping, send pong back to server */
    if (len == ping_len && ntohl(*(uint32_t *)buf) == ping_len && memcmp(buf + HEADER_SIZE, PING_STR, PING_SIZE) == 0) {
        struct network_con_s *con = container_of(s, struct network_con_s, stream.stream);
        int len_ustream;
        const char *msg = PONG_STR;
        size_t msglen = PONG_SIZE;
        uint32_t final_len = msglen + HEADER_SIZE;
        char final_str[HEADER_SIZE + PONG_SIZE];
        uint32_t *msg_header = (uint32_t *)final_str;

        dawnlog_info("Client: received ping from %s:%u, now=%d\n", inet_ntoa(con->sock_addr.sin_addr), ntohs(con->sock_addr.sin_port), (int)time(0));
        con->time_alive = time(0);

        *msg_header = htonl(final_len);
        memcpy(final_str + HEADER_SIZE, msg, msglen);
        len_ustream = ustream_write(&con->stream.stream, final_str, final_len, 0);
        if (len_ustream <= 0) {
            dawnlog_error("Ustream error(" STR_QUOTE(__LINE__) ")!\n");
            //ERROR HANDLING!
            if (con->stream.stream.write_error) {
                ustream_free(&con->stream.stream);
                dawn_unregmem(&con->stream.stream);
                close(con->fd.fd);
                list_del(&con->list);
                dawn_free(con);
                con = NULL;
            }
        }

    } else {
        buf[len] = 0;
        dawnlog_error("Read %d bytes upexpected: %s\n", len, buf);
    }

}

static void connect_cb(struct uloop_fd *f, unsigned int events) {

    struct network_con_s *entry = container_of(f, struct network_con_s, fd);

    dawnlog_debug_func("Entering...");

    if (f->eof || f->error) {
        dawnlog_error("Connection failed (%s)\n", f->eof ? "EOF" : "ERROR");
        close(entry->fd.fd);
        list_del(&entry->list);
        dawn_free(entry);
        entry = NULL;
        return;
    }

    dawnlog_debug("Connection established\n");
    uloop_fd_delete(&entry->fd);

    entry->stream.stream.notify_read = client_ping_read_cb;
    entry->stream.stream.notify_state = client_to_server_state;

    ustream_fd_init(&entry->stream, entry->fd.fd);
    dawn_regmem(&entry->stream);

    entry->connected = 1;
}

int add_tcp_connection(char *ipv4, int port) {
    struct sockaddr_in serv_addr;

    dawnlog_debug_func("Entering...");

    char port_str[12];
    sprintf(port_str, "%d", port); // TODO: Manage buffer length

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ipv4);
    serv_addr.sin_port = htons(port);

    struct network_con_s *tmp = tcp_list_contains_address(serv_addr);
    if (tmp != NULL) {
        if(tmp->connected == true)
        {
            return 0;
        } else{
            // Delete already existing entry
            close(tmp->fd.fd);
            list_del(&tmp->list);
            dawn_free(tmp);
            tmp = NULL;
        }
    }

    struct network_con_s *tcp_entry = dawn_calloc(1, sizeof(struct network_con_s));
    tcp_entry->fd.fd = usock(USOCK_TCP | USOCK_NONBLOCK, ipv4, port_str);
    tcp_entry->sock_addr = serv_addr;

    if (tcp_entry->fd.fd < 0) {
        dawn_free(tcp_entry);
        tcp_entry = NULL;
        return -1;
    }
    tcp_entry->fd.cb = connect_cb;
    uloop_fd_add(&tcp_entry->fd, ULOOP_WRITE | ULOOP_EDGE_TRIGGER);

    dawnlog_debug("New TCP connection to %s:%d\n", ipv4, port);
    tcp_entry->time_alive = time(0);
    list_add(&tcp_entry->list, &tcp_sock_list);

    return 0;
}

void send_tcp(char *msg) {
    dawnlog_debug_func("Entering...");

    if (dawnlog_showing(DAWNLOG_DEBUG))
        print_tcp_array();

    struct network_con_s *con, *tmp;
    if (network_config.use_symm_enc) {
        int length_enc;
        size_t msglen = strlen(msg)+1;
        char *enc = gcrypt_encrypt_msg(msg, msglen, &length_enc);
        if (!enc){
            dawnlog_error("Ustream error: not enough memory (" STR_QUOTE(__LINE__) ")\n");
            return;
        }

        uint32_t final_len = length_enc + sizeof(final_len);
        char *final_str = dawn_malloc(final_len);
        if (!final_str){
            dawn_free(enc);
            enc = NULL;
            dawnlog_error("Ustream error: not enough memory (" STR_QUOTE(__LINE__) ")\n");
            return;
        }
        uint32_t *msg_header = (uint32_t *)final_str;
        *msg_header = htonl(final_len);
        memcpy(final_str+sizeof(final_len), enc, length_enc);
        list_for_each_entry_safe(con, tmp, &tcp_sock_list, list)
        {
            if (con->connected) {
                int len_ustream = ustream_write(&con->stream.stream, final_str, final_len, 0);
                dawnlog_debug("Ustream send: %d\n", len_ustream);
                if (len_ustream <= 0) {
                    dawnlog_error("Ustream error(" STR_QUOTE(__LINE__) ")!\n");
                    //ERROR HANDLING!
                    if (con->stream.stream.write_error) {
                        ustream_free(&con->stream.stream);
                        dawn_unregmem(&con->stream.stream);
                        close(con->fd.fd);
                        list_del(&con->list);
                        dawn_free(con);
                        con = NULL;
                    }
                }
            }

        }

        dawn_free(final_str);
        final_str = NULL;
        dawn_free(enc);
        enc = NULL;
    } else {
        size_t msglen = strlen(msg) + 1;
        uint32_t final_len = msglen + sizeof(final_len);
        char *final_str = dawn_malloc(final_len);
        if (!final_str){
            dawnlog_error("Ustream error: not enough memory (" STR_QUOTE(__LINE__) ")\n");
            return;
        }
        uint32_t *msg_header = (uint32_t *)final_str;
        *msg_header = htonl(final_len);
        memcpy(final_str+sizeof(final_len), msg, msglen);

        list_for_each_entry_safe(con, tmp, &tcp_sock_list, list)
        {
            if (con->connected) {
                int len_ustream = ustream_write(&con->stream.stream, final_str, final_len, 0);
                dawnlog_debug("Ustream send: %d\n", len_ustream);
                if (len_ustream <= 0) {
                    //ERROR HANDLING!
                    dawnlog_error("Ustream error(" STR_QUOTE(__LINE__) ")!\n");
                    if (con->stream.stream.write_error) {
                        ustream_free(&con->stream.stream);
                        dawn_unregmem(&con->stream.stream);
                        close(con->fd.fd);
                        list_del(&con->list);
                        dawn_free(con);
                        con = NULL;
                    }
                }
            }
        }
        dawn_free(final_str);
        final_str = NULL;
    }
}

void server_to_clients_ping(void)
{
    struct client *cl, *tmp;
    const char *msg = PING_STR;
    size_t msglen = PING_SIZE;
    uint32_t final_len = msglen + HEADER_SIZE;
    char final_str[HEADER_SIZE + PING_SIZE];
    uint32_t *msg_header = (uint32_t *)final_str;
    *msg_header = htonl(final_len);
    memcpy(final_str + HEADER_SIZE, msg, msglen);

    list_for_each_entry_safe(cl, tmp, &cli_list, list)
    {
        int len_ustream = ustream_write(&cl->s.stream, final_str, final_len, 0);
        if (len_ustream <= 0) {
            dawnlog_error("Ustream error(" STR_QUOTE(__LINE__) ")!\n");
            if (cl->s.stream.write_error) {
                client_close(&cl->s.stream);
            }
        }
    }
}

void check_timeout(int timeout) {
    do {
        struct client *cl, *tmp;
        time_t now = time(0);
        list_for_each_entry_safe(cl, tmp, &cli_list, list)
        {
            if (now - cl->time_alive > timeout || now - cl->time_alive < -timeout) {
                dawnlog_info("Client: close client connection! timeout=%d\n", (int)(now - cl->time_alive));
                client_close(&cl->s.stream);
            }
        }
    } while (0);

    do {
        struct network_con_s *con, *tmp;
        time_t now = time(0);
        list_for_each_entry_safe(con, tmp, &tcp_sock_list, list)
        {
            if (now - con->time_alive > timeout || now - con->time_alive < -timeout) {
                dawnlog_info("Server: close client_to_server connection! timeout=%d\n", (int)(now - con->time_alive));
                ustream_free(&con->stream.stream);
                dawn_unregmem(&con->stream.stream);
                close(con->fd.fd);
                list_del(&con->list);
                dawn_free(con);
            }
        }
    } while (0);
}

struct network_con_s* tcp_list_contains_address(struct sockaddr_in entry) {
    struct network_con_s *con;

    dawnlog_debug_func("Entering...");

    list_for_each_entry(con, &tcp_sock_list, list)
    {
        if(entry.sin_addr.s_addr == con->sock_addr.sin_addr.s_addr)
        {
            return con;
        }
    }
    return NULL;
}

void print_tcp_array() {
    struct network_con_s *con;
    dawnlog_debug("--------Connections------\n");
    list_for_each_entry(con, &tcp_sock_list, list)
    {
        dawnlog_debug("Connecting to Port: %d, Connected: %s\n", ntohs(con->sock_addr.sin_port), con->connected ? "True" : "False");
    }
    dawnlog_debug("------------------\n");
}
