
#include <config.h>

#include <string.h>

#include "app.h"
#include "dnscrypt_proxy.h"
#include "logger.h"
#include "utils.h"
#include "windows_service.h"

#ifndef _WIN32

int
main(int argc, char *argv[])
{
    return dnscrypt_proxy_main(argc, argv);
}

#else

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "logger.h"
#include "utils.h"

static char                 *windows_service_name;
static SERVICE_STATUS        service_status;
static SERVICE_STATUS_HANDLE service_status_handle;
static _Bool                 app_is_running_as_a_service;

static void WINAPI
control_handler(const DWORD wanted_state)
{
    if (wanted_state == SERVICE_CONTROL_STOP &&
        dnscrypt_proxy_loop_break() == 0) {
        service_status.dwCurrentState = SERVICE_STOPPED;
    }
    SetServiceStatus(service_status_handle, &service_status);
}

static char **
cmdline_clone_options(const int argc, char ** const argv)
{
    char **argv_new;

    if (argc >= INT_MAX || (size_t) argc >= SIZE_MAX / sizeof *argv_new ||
        (argv_new = calloc((unsigned int) argc + 1U,
                           sizeof *argv_new)) == NULL) {
        return NULL;
    }
    memcpy(argv_new, argv, (unsigned int) (argc + 1U) * sizeof *argv_new);

    return argv_new;
}

static int
cmdline_add_option(int * const argc_p, char *** const argv_p,
                   const char * const arg)
{
    char  *arg_dup;
    char **argv_new;

    if (*argc_p >= INT_MAX ||
        SIZE_MAX / sizeof *argv_new <= (unsigned int) (*argc_p + 2U)) {
        return -1;
    }
    if ((argv_new = realloc(*argv_p, (unsigned int) (*argc_p + 2U) *
                            sizeof *argv_new)) == NULL) {
        return -1;
    }
    if ((arg_dup = strdup(arg)) == NULL) {
        free(argv_new);
        return -1;
    }
    argv_new[(*argc_p)++] = arg_dup;
    argv_new[*argc_p] = NULL;
    *argv_p = argv_new;
    logger(NULL, LOG_INFO, "Adding command-line option: [%s]", arg_dup);

    return 0;
}

typedef struct WindowsServiceParseMultiSzCb_ {
    void  (*cb)(struct WindowsServiceParseMultiSzCb_ *, const char *string);
    int    * const argc_p;
    char *** const argv_p;
    int    * const err_p;
} WindowsServiceParseMultiSzCb;

static void
windows_service_parse_multi_sz_cb(WindowsServiceParseMultiSzCb * const cb,
                                  const char *string)
{
    assert(cb->cb == windows_service_parse_multi_sz_cb);
    *(cb->err_p) += cmdline_add_option(cb->argc_p, cb->argv_p, "--plugin");
    *(cb->err_p) += cmdline_add_option(cb->argc_p, cb->argv_p, string);
}

static int
windows_service_parse_multi_sz(WindowsServiceParseMultiSzCb * const cb,
                               const char * const multi_sz,
                               const size_t multi_sz_len)
{
    const char *multi_sz_pnt = multi_sz;
    const char *zero;
    size_t      len;
    size_t      multi_sz_remaining_len = multi_sz_len;
    size_t      zlen;

    while (multi_sz_remaining_len > (size_t) 0U &&
           (zero = memchr(multi_sz_pnt, 0, multi_sz_remaining_len)) != NULL) {
        if ((len = (size_t) (zero - multi_sz_pnt)) > (size_t) 0U) {
            cb->cb(cb, multi_sz_pnt);
        }
        zlen = len + (size_t) 1U;
        assert(zlen <= multi_sz_remaining_len);
        multi_sz_remaining_len -= zlen;
        multi_sz_pnt += zlen;
    }
    return 0;
}

char *
windows_service_registry_parameters_key(void)
{
    static char *key;
    size_t      sizeof_key;

    if (key != NULL) {
        return key;
    }
    sizeof_key = (sizeof WINDOWS_SERVICE_REGISTRY_PARAMETERS_PATH - 1) +
        strlen(get_windows_service_name()) +
        (sizeof WINDOWS_SERVICE_REGISTRY_PARAMETERS_NAME - 1) + 1;
    if ((key = malloc(sizeof_key)) == NULL) {
        exit(1);
    }
    evutil_snprintf(key, sizeof_key, "%s%s%s",
                    WINDOWS_SERVICE_REGISTRY_PARAMETERS_PATH,
                    get_windows_service_name(),
                    WINDOWS_SERVICE_REGISTRY_PARAMETERS_NAME);

    return key;
}

static int
windows_service_registry_read_multi_sz(const char * const key,
                                       WindowsServiceParseMultiSzCb * const cb)
{
    BYTE   *value = NULL;
    HKEY    hk = NULL;
    DWORD   value_len;
    DWORD   value_type;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     windows_service_registry_parameters_key(),
                     (DWORD) 0, KEY_READ, &hk) != ERROR_SUCCESS) {
        return -1;
    }
    if (RegQueryValueEx(hk, key, 0,
                        &value_type, NULL, &value_len) == ERROR_SUCCESS &&
        value_type == (DWORD) REG_MULTI_SZ &&
        value_len <= SIZE_MAX && value_len > (DWORD) 0 &&
        (value = malloc((size_t) value_len)) != NULL) {
        if (RegQueryValueEx(hk, key, 0,
                            &value_type, value, &value_len) != ERROR_SUCCESS ||
            value_type != (DWORD) REG_MULTI_SZ) {
            free(value);
            value = NULL;
        }
        assert(value == NULL || value_len == 0 ||
               (value_len > 0 && value[value_len - 1] == 0));
    }
    RegCloseKey(hk);
    if (value == NULL) {
        return -1;
    }
    windows_service_parse_multi_sz(cb, (const char *) value,
                                   (size_t) value_len);
    free(value);

    return 0;
}

static int
windows_service_registry_read_string(const char * const key,
                                     char ** const value_p)
{
    BYTE   *value = NULL;
    HKEY    hk = NULL;
    DWORD   value_len;
    DWORD   value_type;

    *value_p = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     windows_service_registry_parameters_key(),
                     (DWORD) 0, KEY_READ, &hk) != ERROR_SUCCESS) {
        return -1;
    }
    if (RegQueryValueEx(hk, key, 0,
                        &value_type, NULL, &value_len) == ERROR_SUCCESS &&
        value_type == (DWORD) REG_SZ &&
        value_len <= SIZE_MAX && value_len > (DWORD) 0 &&
        (value = malloc((size_t) value_len)) != NULL) {
        if (RegQueryValueEx(hk, key, 0,
                            &value_type, value, &value_len) != ERROR_SUCCESS ||
            value_type != (DWORD) REG_SZ) {
            free(value);
            value = NULL;
        }
        assert(value == NULL || value_len == 0 ||
               (value_len > 0 && value[value_len - 1] == 0));
    }
    RegCloseKey(hk);
    *value_p = (char *) value;

    return - (value == NULL);
}

static int
windows_service_registry_read_dword(const char * const key,
                                    DWORD * const value_p)
{
    HKEY   hk = NULL;
    DWORD  value = 0;
    DWORD  value_len = (DWORD) sizeof value;
    DWORD  value_type;
    int    ret = -1;

    *value_p = (DWORD) 0;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     windows_service_registry_parameters_key(),
                     (DWORD) 0, KEY_READ, &hk) != ERROR_SUCCESS) {
        return -1;
    }
    if (RegQueryValueEx(hk, key, 0, &value_type, (void *) &value, &value_len)
        == ERROR_SUCCESS && value_type == (DWORD) REG_DWORD) {
        *value_p = value;
        ret = 0;
    }
    RegCloseKey(hk);

    return ret;
}

static int
windows_service_registry_write_string(const char * const key,
                                      const char * const value)
{
    HKEY   hk = NULL;
    size_t value_len;
    int    ret = 0;

    value_len = strlen(value);
    if (value_len > 0x7fffffff) {
        return -1;
    }
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       windows_service_registry_parameters_key(),
                       (DWORD) 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
                       NULL, &hk, NULL) != ERROR_SUCCESS) {
        return -1;
    }
    if (RegSetValueEx(hk, key, (DWORD) 0, REG_SZ, (const BYTE *) value,
                      (DWORD) value_len) != ERROR_SUCCESS) {
        ret = -1;
    }
    RegCloseKey(hk);

    return ret;
}

static int
windows_build_command_line_from_registry(int * const argc_p,
                                         char *** const argv_p)
{
    char   dword_string[sizeof "2147483648"];
    char  *string_value;
    DWORD  dword_value;
    int    err = 0;

    if ((*argv_p = cmdline_clone_options(*argc_p, *argv_p)) == NULL) {
        exit(1);
    }
    if (windows_service_registry_read_string
        ("ConfigFile", &string_value) == 0) {
        err += cmdline_add_option(argc_p, argv_p, string_value);
        free(string_value);
        return -(err != 0);
    }
    if (windows_service_registry_read_string
        ("ResolversList", &string_value) == 0) {
        err += cmdline_add_option(argc_p, argv_p, "--resolvers-list");
        err += cmdline_add_option(argc_p, argv_p, string_value);
        free(string_value);
    }
    if (windows_service_registry_read_string
        ("ResolverName", &string_value) == 0) {
        err += cmdline_add_option(argc_p, argv_p, "--resolver-name");
        err += cmdline_add_option(argc_p, argv_p, string_value);
        free(string_value);
    }
    if (windows_service_registry_read_string
        ("LocalAddress", &string_value) == 0) {
        err += cmdline_add_option(argc_p, argv_p, "--local-address");
        err += cmdline_add_option(argc_p, argv_p, string_value);
        free(string_value);
    }
    if (windows_service_registry_read_string
        ("ProviderKey", &string_value) == 0) {
        err += cmdline_add_option(argc_p, argv_p, "--provider-key");
        err += cmdline_add_option(argc_p, argv_p, string_value);
        free(string_value);
    }
    if (windows_service_registry_read_string
        ("ProviderName", &string_value) == 0) {
        err += cmdline_add_option(argc_p, argv_p, "--provider-name");
        err += cmdline_add_option(argc_p, argv_p, string_value);
        free(string_value);
    }
    if (windows_service_registry_read_string
        ("ResolverAddress", &string_value) == 0) {
        err += cmdline_add_option(argc_p, argv_p, "--resolver-address");
        err += cmdline_add_option(argc_p, argv_p, string_value);
        free(string_value);
    }
    if (windows_service_registry_read_dword
        ("EDNSPayloadSize", &dword_value) == 0) {
        evutil_snprintf(dword_string, sizeof dword_string, "%ld",
                        (long) dword_value);
        err += cmdline_add_option(argc_p, argv_p, "--edns-payload-size");
        err += cmdline_add_option(argc_p, argv_p, dword_string);
    }
    if (windows_service_registry_read_dword
        ("MaxActiveRequests", &dword_value) == 0) {
        evutil_snprintf(dword_string, sizeof dword_string, "%ld",
                        (long) dword_value);
        err += cmdline_add_option(argc_p, argv_p, "--max-active-requests");
        err += cmdline_add_option(argc_p, argv_p, dword_string);
    }
    if (windows_service_registry_read_dword
        ("TCPOnly", &dword_value) == 0 && dword_value > (DWORD) 0) {
        err += cmdline_add_option(argc_p, argv_p, "--tcp-only");
    }
    if (windows_service_registry_read_dword
        ("EphemeralKeys", &dword_value) == 0 && dword_value > (DWORD) 0) {
        err += cmdline_add_option(argc_p, argv_p, "--ephemeral-keys");
    }
    if (windows_service_registry_read_dword
        ("IgnoreTimestamps", &dword_value) == 0 && dword_value > (DWORD) 0) {
        err += cmdline_add_option(argc_p, argv_p, "--ignore-timestamps");
    }
    if (windows_service_registry_read_string
        ("ClientKeyFile", &string_value) == 0) {
        err += cmdline_add_option(argc_p, argv_p, "--client-key");
        err += cmdline_add_option(argc_p, argv_p, string_value);
        free(string_value);
    }
    if (windows_service_registry_read_string
        ("LogFile", &string_value) == 0) {
        err += cmdline_add_option(argc_p, argv_p, "--logfile");
        err += cmdline_add_option(argc_p, argv_p, string_value);
        free(string_value);
    }
    if (windows_service_registry_read_dword
        ("LogLevel", &dword_value) == 0) {
        evutil_snprintf(dword_string, sizeof dword_string, "%ld",
                        (long) dword_value);
        err += cmdline_add_option(argc_p, argv_p, "--loglevel");
        err += cmdline_add_option(argc_p, argv_p, dword_string);
    }
    windows_service_registry_read_multi_sz
        ("Plugins", & (WindowsServiceParseMultiSzCb) {
            .cb = windows_service_parse_multi_sz_cb,
            .argc_p = argc_p,
            .argv_p = argv_p,
            .err_p = &err
        });
    if (err != 0) {
        return -1;
    }
    return 0;
}

char *
get_windows_service_name(void)
{
    assert(windows_service_name != NULL);

    return windows_service_name;
}

static void
set_windows_service_name(char *name)
{
    windows_service_name = name;
}

static int
init_windows_service_name(int argc, char *argv[])
{
    int i;

    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--service-name") == 0) {
            if (i < argc - 1) {
                windows_service_name = argv[i + 1];
                return 0;
            } else {
                logger_noformat(NULL, LOG_ERR,
                                "Missing service name after --service-name");
                exit(1);
            }
        }
        if (strncmp(argv[i], "--service-name=", sizeof "--service-name=" - 1) == 0) {
            windows_service_name = argv[i] + sizeof "--service-name=" - 1;
            if (*windows_service_name != 0) {
                return 0;
            } else {
                logger_noformat(NULL, LOG_ERR,
                                "Missing service name after --service-name");
                exit(1);
            }
        }
    }
    if ((windows_service_name = strdup(WINDOWS_SERVICE_NAME)) == NULL) {
        return -1;
    }
    return 0;
}

static void WINAPI
service_main(DWORD argc_, LPTSTR *argv_)
{
    char **argv = (char **) argv_;
    int    argc = (int) argc_;

    if (argc != 0) {
        set_windows_service_name(argv[0]);
    }
    assert(argc_ < INT_MAX);
    if (windows_build_command_line_from_registry(&argc, &argv) != 0) {
        logger_noformat(NULL, LOG_ERR,
                        "Unable to build a command line from the registry");
        return;
    }
    memset(&service_status, 0, sizeof service_status);
    service_status.dwServiceType = SERVICE_WIN32;
    service_status.dwCurrentState = SERVICE_START_PENDING;
    service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    service_status_handle =
        RegisterServiceCtrlHandler(get_windows_service_name(), control_handler);
    if (service_status_handle == 0) {
        return;
    }
    service_status.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(service_status_handle, &service_status);

    dnscrypt_proxy_main(argc, argv);
}

static int
windows_main(int argc, char *argv[])
{
    static SERVICE_TABLE_ENTRY service_table[2];

    if (init_windows_service_name(argc, argv) != 0) {
        return -1;
    }
    memcpy(service_table, (SERVICE_TABLE_ENTRY[2]) {
        { .lpServiceName = get_windows_service_name(), .lpServiceProc = service_main },
        { .lpServiceName = NULL, .lpServiceProc = (void *) NULL }
    }, sizeof service_table);
    if (StartServiceCtrlDispatcher(service_table) == 0) {
        return dnscrypt_proxy_main(argc, argv);
    }
    app_is_running_as_a_service = 1;

    return 0;
}

int
windows_service_uninstall(void)
{
    SC_HANDLE      scm_handle;
    SC_HANDLE      service_handle;
    SERVICE_STATUS service_status;
    int            ret = 0;

    scm_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm_handle == NULL) {
        return -1;
    }
    service_handle = OpenService(scm_handle, get_windows_service_name(),
                                 DELETE | SERVICE_STOP);
    if (service_handle == NULL) {
        CloseServiceHandle(scm_handle);
        return 0;
    }
    ControlService(service_handle, SERVICE_CONTROL_STOP, &service_status);
    if (DeleteService(service_handle) == 0) {
        ret = -1;
    }
    CloseServiceHandle(service_handle);
    CloseServiceHandle(scm_handle);

    return ret;
}

static int
windows_registry_install_with_config_file(const char *config_file)
{
    char *config_file_path;
    int   ret;

    if ((config_file_path = path_from_app_folder(config_file)) == NULL) {
        return -1;
    }
    ret = windows_service_registry_write_string("ConfigFile",
                                                config_file_path);
    free(config_file_path);

    return ret;
}

static int
windows_registry_install(ProxyContext * const proxy_context)
{
    if (proxy_context->resolvers_list != NULL) {
        windows_service_registry_write_string("ResolversList",
                                              proxy_context->resolvers_list);
    }
    if (proxy_context->resolver_name != NULL) {
        windows_service_registry_write_string("ResolverName",
                                              proxy_context->resolver_name);
    }
    if (proxy_context->local_ip != NULL) {
        windows_service_registry_write_string("LocalAddress",
                                              proxy_context->local_ip);
    }
    if (proxy_context->client_key_file != NULL) {
        windows_service_registry_write_string("ClientKeyFile",
                                              proxy_context->client_key_file);
    }
    if (proxy_context->log_file != NULL) {
        windows_service_registry_write_string("LogFile",
                                              proxy_context->log_file);
    }
    return 0;
}

static int
register_service(void)
{
    char      self_path[MAX_PATH];
    char      self_path_quoted[2U + MAX_PATH];
    SC_HANDLE scm_handle;
    SC_HANDLE service_handle;

    if (GetModuleFileName(NULL, self_path, MAX_PATH) <= (DWORD) 0) {
        return -1;
    }
    if (strchr(self_path, '"') != NULL) {
        return -1;
    }
    evutil_snprintf(self_path_quoted, sizeof self_path_quoted,
                    "\"%s\"", self_path);
    scm_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm_handle == NULL) {
        return -1;
    }
    service_handle = CreateService
        (scm_handle, get_windows_service_name(),
         get_windows_service_name(), SERVICE_ALL_ACCESS,
         SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START,
         SERVICE_ERROR_NORMAL, self_path_quoted, NULL, NULL, NULL, NULL, NULL);
    if (service_handle == NULL) {
        CloseServiceHandle(scm_handle);
        return -1;
    }
    StartService(service_handle, (DWORD) 0, NULL);
    CloseServiceHandle(service_handle);
    CloseServiceHandle(scm_handle);

    return 0;
}

int
windows_service_install(ProxyContext * const proxy_context)
{
    if (windows_registry_install(proxy_context) != 0) {
        logger_noformat(proxy_context, LOG_ERR,
                        "Unable to set up registry keys");
        return -1;
    }
    return register_service();
}

int
windows_service_install_with_config_file(ProxyContext * const proxy_context,
                                         const char * const config_file)
{
    if (windows_registry_install_with_config_file(config_file) != 0) {
        logger_noformat(proxy_context, LOG_ERR,
                        "Unable to set up registry keys for the config file");
        return -1;
    }
    return register_service();
}

int
main(int argc, char *argv[])
{
    return windows_main(argc, argv);
}

#endif
