/*
 * Copyright 2025 Morse Micro
 */

#include <errno.h>

#include "morsectrl.h"
#include "utilities.h"
#include "transport/transport.h"

static struct {
    struct arg_str *ssid;
    struct arg_rex *auth;
    struct arg_str *pwd;
} args;

int connect_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Connect to a network (like `iw connect`) (nl80211 transport only)",
        args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID"),
        args.auth = arg_rex1("a", "auth-type", "(open|sae|owe)", "{open|sae|owe}",
                             0, "Authentication type"),
        args.pwd = arg_str0("p", "sae-password", "<password>", "SAE password"));

    return 0;
}

int connect_handler(struct morsectrl *mors, int argc, char *argv[])
{
    int ssid_len = 0;
    int pwd_len = 0;
    const char *auth = args.auth->sval[0];
    bool secure = true;

    if (!args.ssid->count || !args.auth->count)
    {
        return -EINVAL;
    }

    ssid_len = strlen(args.ssid->sval[0]);

    if (args.pwd->count)
    {
        pwd_len = strlen(args.pwd->sval[0]);
    }

    if (!strcmp(auth, "sae"))
    {
        if (!pwd_len)
        {
            mctrl_err("SAE requires a password\n");
            return -EINVAL;
        }
    }
    else if (!strcmp(auth, "owe"))
    {
        if (pwd_len)
        {
            mctrl_err("OWE doesn't have a password\n");
            return -EINVAL;
        }
    }
    else
    {
        if (pwd_len)
        {
            mctrl_err("Open network doesn't have a password\n");
            return -EINVAL;
        }
        secure = false;
    }

    return morsectrl_transport_connect(mors->transport, args.ssid->sval[0],
                                       ssid_len, args.pwd->sval[0], pwd_len, secure);
}

MM_CLI_HANDLER_CUSTOM_FUNC(connect, connect_handler, MM_INTF_REQUIRED,
                           MM_DIRECT_CHIP_NOT_SUPPORTED);
