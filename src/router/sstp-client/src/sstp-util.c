/*!
 * @brief Utility Functions
 *
 * @file sstp-util.c
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <config.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include "sstp-private.h"


status_t sstp_set_nonbl(int sock, int state)
{
    int ret  = -1;
    int flag = fcntl(sock, F_GETFL);

    flag = (state == 1) 
        ? (flag | O_NONBLOCK)
        : (flag & ~O_NONBLOCK);

    ret = fcntl(sock, F_SETFL, flag);
    if (ret != 0)
    {
        return SSTP_FAIL;
    }

    return SSTP_OKAY;
}


char *sstp_get_guid(char *buf, int len)
{
    uint32_t data1, data4;
    uint16_t data2, data3;
    unsigned int seed;
    int ret;

    seed = time(NULL) | getpid();
    srand (seed);

    data1 = (rand() + 1);
    data2 = (rand() + 1);
    data3 = (rand() + 1);
    data4 = (rand() + 1);

    /* Create the GUID string */
    ret = snprintf(buf, len, "{%.4X-%.2X-%.2X-%.4X}", data1, data2, 
            data3, data4);
    if (ret <= 0 || ret > len)
    {
        return NULL;
    }

    return buf;    
}


status_t sstp_set_sndbuf(int sock, int size)
{
    int ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
    if (ret != 0)
    {
        return SSTP_FAIL;
    }

    return SSTP_OKAY;
}


status_t sstp_url_parse(sstp_url_st **url, const char *path)
{
    char *ptr = NULL;
    char *ptr1 = NULL;

    /* Allocate url context */
    sstp_url_st *ctx = calloc(1, sizeof(sstp_url_st));
    if (!ctx)
    {
        goto errout;
    }
    
    /* Copy to working buffer */
    ctx->ptr = strdup(path);
    ptr = ctx->ptr;
    
    /* Look for the protocol string */
    ptr1 = strstr(ptr, "://");
    if (ptr1 != NULL)
    {
        ctx->schema= ptr;
        *ptr1 = '\0';
        ptr1  += 3;
        ptr    = ptr1;
    }

    /* Username & Password? */
    ptr1 = strchr(ptr, '@');
    if (ptr1 != NULL)
    {
        ctx->user = ptr;
        *ptr1++ = '\0';
        ptr = ptr1;
    }

    /* Set the site pointer */
    ctx->host = ptr;
    
    /* Extract the password */
    if (ctx->user)
    {
        ptr1 = strchr(ctx->user, ':');
        if (ptr1)
        {
            *ptr1++ = '\0';
            ctx->password = ptr1;
        }
    }

    /* Look for the optional port component */
    ptr1 = strchr(ptr, ':');
    if (ptr1 != NULL)
    {
        /* Get the site */
        *ptr1++ = '\0';
        ptr     = ptr1;
        ctx->port = ptr1;
    }

    /* Look for the path component */
    ptr1 = strchr(ptr, '/');
    if (ptr1 != NULL)
    {
        *ptr1++ = '\0';
        ctx->path = ptr1;
    }

    /* Either must be specified */
    if (!ctx->schema && !ctx->port)
    {
        ctx->port = "443";
    }

    /* Success */
    *url = ctx;
    return SSTP_OKAY;

errout:
    
    if (ctx)
    {
        sstp_url_free(ctx);
    }

    return SSTP_FAIL;
}


void sstp_url_free(sstp_url_st *url)
{
    if (!url)
    {
        return;
    }

    if (url->ptr)
    {
        free(url->ptr);
        url->ptr = NULL;
    }

    free(url);
}


const char *sstp_norm_data(unsigned long long count, char *buf, int len)
{
    float b = count;
    char v [] = { 'K', 'M', 'G', 'T' };
    int i = 0;

    if (count <= 1024) 
    {
        snprintf(buf, len, "%llu bytes", count);
        return buf;
    }

    while (b > 1024)
    {
        b /= 1024;
        i++;
    }

    snprintf(buf, len, "%.02f %cb", b, v[i-1]);
    return buf;
}


/*!
 * @brief Normilize into hour, min or sec.
 */
const char *sstp_norm_time(unsigned long t, char *buf, int len)
{
    if (t > 3600)
    {
        snprintf(buf, len, "%.02f hour(s)", (float)t/3600);
        return buf;
    }

    if (t > 60)
    {
        snprintf(buf, len, "%.02f minute(s)", (float)t/60);
        return buf;
    }

    snprintf(buf, len, "%lu seconds", t);
    return buf;
}


/*!
 * @brief Convert sockaddr structure to an ip-string
 */
const char *sstp_ipaddr(struct sockaddr *addr, char *buf, int len)
{
    const char *retval = NULL;

    switch (addr->sa_family)
    {
    case AF_INET:
    {
        struct sockaddr_in *in = (struct sockaddr_in*) addr;
        if (inet_ntop(AF_INET, &in->sin_addr, buf, len))
        {
            retval = buf;
        }
        break;
    }
    case AF_INET6:
    {
        struct sockaddr_in6 *in = (struct sockaddr_in6*) addr;
        if (inet_ntop(AF_INET6, &in->sin6_addr, buf, len))
        {
            retval = buf;
        }
        break;
    }
    default:
        break;
    }

    return retval;
}


int sstp_get_uid(const char *name)
{
    struct passwd pwd;
    struct passwd *res = NULL;
    char *buff = NULL;
    int blen = 0;

    blen = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (blen == -1)
    {
        blen = 1024;
    }

    /* Allocate the memory */
    buff = alloca(blen);
    if (!buff)
    {
        return -1;
    }

    /* Get the password entry */
    if (!getpwnam_r(name, &pwd, buff, blen, &res) && res)
    {
        return pwd.pw_uid;
    }

    return -1;
}


int sstp_get_gid(const char *name)
{
    struct group grp;
    struct group *res = NULL;
    char *buff = NULL;
    int blen = 0;

    blen = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (blen == -1)
    {
        blen = 1024;
    }

    /* Allocate the memory */
    buff = alloca(blen);
    if (!buff)
    {
        return -1;
    }

    /* Get the password entry */
    if (!getgrnam_r(name, &grp, buff, blen, &res) && res)
    {
        return grp.gr_gid;
    }

    return -1;
}


int sstp_sandbox(const char *path, const char *user, const char *group)
{
    int gid = -1;
    int uid = -1;
    int retval = -1;

    if (user)
    {
        uid = sstp_get_uid(user);
    }
    
    if (group)
    {
        gid = sstp_get_gid(group);
    }

    /* Change the root directory */
    if (path)
    {
        if (chdir(path) != 0)
        {
            log_warn("Could not change working directory, %s (%d)", 
                strerror(errno), errno);
            goto done;
        }

        if (chroot(path) != 0)
        {
            log_warn("Could not change root directory, %s (%d)", 
                strerror(errno), errno);
            goto done;
        }
    }

    /* Set the group id (before setting user id) */
    if (gid >= 0 && gid != getgid())
    {
        if (setgid(gid) != 0)
        {
            log_warn("Could not set process group id, %s (%d)", 
                strerror(errno), errno);
            goto done;
        }
    }

    /* Setting the user id */
    if (uid >= 0 && uid != getuid())
    {
        if (setuid(uid) != 0)
        {
            log_warn("Could not set process user id, %s (%d)", 
                strerror(errno), errno);
            goto done;
        }
    }

    retval = 0;

done:
    
    return (retval);
}


int sstp_create_dir(const char *path, const char *user, const char *group, mode_t mode)
{
    int ret = -1;
    int gid = -1;
    int uid = -1;
    int retval = (-1);

    /* Create the directory if it doesn't exists */
    ret = mkdir(path, mode);
    if (ret != 0 && errno != EEXIST)
    {
        log_err("Could not create directory: %s, %s (%d)", 
            path, strerror(errno), errno);
        goto done;
    }
    
    /* Get the user */
    if (user)
    {
        uid = sstp_get_uid(user);
    }

    /* Get the group */
    if (group)
    {
        gid = sstp_get_gid(group);
    }

    /* Change user/group permissions */
    ret = chown(path, uid, gid);
    if (ret != 0)
    {
        log_warn("Could not change permissions on %s, %s (%d)", 
            path, strerror(errno), errno);
    }

    /* Success */
    retval = 0;

done:

    return retval;
}


#if 0
int main(int argc, char *argv[])
{
    sstp_url_st *url;

    sstp_split_url(&url, argv[1]);

    printf("protocol: %s\n", url->protocol);
    printf("site:     %s\n", url->site);
    printf("port:     %s\n", url->port);
    printf("path:     %s\n", url->path);
}

#endif
