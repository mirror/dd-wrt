#include <stdio.h>  
#include <stdlib.h> 
#include <unistd.h>
#include <string.h> 
#include <network.h>
#include <tftp_support.h>
#include <dirent.h>
#include <errno.h>
#include <cyg/kernel/kapi.h>            /* All the kernel specific stuff */
#include <cyg/infra/diag.h>        

#define MWINCLUDECOLORS
#include "nano-X.h"

static int show_time = 5;

//
// Component interfaces
//

static void
fdprintf(int fd, char *fmt, ...)
{
    char msg[256];
    int ret;
    va_list ap;

    // Ignore if no 'fd'
    va_start(ap, fmt);
    ret = vsprintf(msg, fmt, ap);
    va_end(ap);
    diag_printf(msg);
    if (fd <= 0) return;
    write(fd, msg, strlen(msg));
}

void
show_jpeg(int client, char *file, int show_time)
{
    GR_EVENT        event;          /* current event */
    GR_IMAGE_ID     id = 0;
    GR_SIZE         w = -1;
    GR_SIZE         h = -1;
    GR_IMAGE_INFO   info;
    GR_WINDOW_ID    w1;             /* id for large window */
    GR_GC_ID        gc1;            /* graphics context for text */
    GR_SCREEN_INFO  si;

    int time_left;
    bool ever_exposed = false;

#if !defined(HAVE_JPEG_SUPPORT) && !defined(HAVE_BMP_SUPPORT) && !defined(HAVE_GIF_SUPPORT)
    printf("Sorry, no image support compiled in\n");
    exit(1);        
#endif

    GrGetScreenInfo(&si);
    printf("Loading image: %s\n", file);
    if (access(file, F_OK) < 0) {
        fdprintf(client, "Can't access \"%s\": %s\n", file, strerror(errno));
        return;
    }
    id = GrLoadImageFromFile(file, 0);
    if (id) {
        GrGetImageInfo(id, &info);
    } else {
        // File exists, so why the error?
        int fd, len;
        char buf[64];
        fdprintf(client, "Can't load %s\n", file);
        if ((fd = open(file, O_RDONLY)) >= 0) {
            len = read(fd, buf, 64);
            if (len != 64) {
                diag_printf("Short read? len = %d\n", len);
            } else {
                diag_dump_buf(buf, len);
            }
            close(fd);
        } else {
            diag_printf("Can't oopen \"%s\": %s\n",  file, strerror(errno));
        }
        return;
    }

    w = info.width;
    h = info.height;
    if ((si.rows < info.height) || (si.cols < info.width)) {
        // Preserve aspect ratio
        if (si.cols < info.width) {
            w = si.cols;
            h = (si.cols * info.height) / info.width;
        }
        if (si.rows < h) {
            w = (si.rows * w) / h;
            h = si.rows;
        }
    }
    printf("Create window - orig %dx%d => %dx%d\n", info.width, info.height, w, h);
    fdprintf(client, "<INFO> Display \"%s\" - orig %dx%d => %dx%d\n", file, info.width, info.height, w, h);
    w1 = GrNewWindow(GR_ROOT_WINDOW_ID, 10, 10, w, h, 4, BLACK, WHITE);
    GrSelectEvents(w1, GR_EVENT_MASK_CLOSE_REQ|GR_EVENT_MASK_EXPOSURE);
    GrMapWindow(w1);
    gc1 = GrNewGC();
    GrSetGCForeground(gc1, WHITE);

#define TO_MS 50
    time_left = show_time * 1000;
    while (time_left > 0) {
        GrGetNextEventTimeout(&event, TO_MS);  // milliseconds
        switch(event.type) {
        case GR_EVENT_TYPE_CLOSE_REQ:
            GrDestroyWindow(w1);
            GrFreeImage(id);
            return;
            /* no return*/
        case GR_EVENT_TYPE_EXPOSURE:
            /*GrDrawImageFromFile(w1, gc1, 0, 0, w, h, argv[1],0);*/
            GrDrawImageToFit(w1, gc1, 0, 0, w, h, id);
            ever_exposed = true;
            break;
        default:
        case GR_EVENT_TYPE_NONE:
        case GR_EVENT_TYPE_TIMEOUT:
            time_left -= TO_MS;
            if ((time_left < 0) && !ever_exposed) {
                // Things get real cranky if we delete the window too fast!
                time_left = TO_MS;
            }
            break;
        }
    }    
    GrUnmapWindow(w1);
    GrDestroyWindow(w1);
    GrDestroyGC(gc1);
    GrFreeImage(id);
}

static void
do_ls(int client, char *buf)
{
    int err;
    DIR *dirp;
    int num=0;
    char name[256];

    buf += 2;  // Skip over command
    while (*buf && (*buf == ' ')) buf++;
    if (*buf) {
        // Name provided
        strcpy(name, buf);
    } else {
        strcpy(name, ".");
    }
    
    fdprintf(client, "<INFO> reading directory %s\n",name);
    dirp = opendir( name );    
    if( dirp == NULL ) {
        fdprintf(client, "Can't open directory \"%s\"\n", name);
        return;
    }
    for(;;) {
        struct dirent *entry = readdir( dirp );
        struct stat sbuf;
        char fullname[PATH_MAX];
        
        if( entry == NULL )
            break;
        num++;
        diag_printf("<INFO> entry %s",entry->d_name);
        fdprintf(client, "<INFO> entry %14s",entry->d_name);
        if( name[0] ) {
            strcpy(fullname, name );
            if( !(name[0] == '/' && name[1] == 0 ) )
                strcat(fullname, "/" );
        }
        else fullname[0] = 0;
        strcat(fullname, entry->d_name );
        err = stat( fullname, &sbuf );
        if( err < 0 ) {
            if( errno == ENOSYS ) {
                fdprintf(client, " <no status available>");
            } else {
                diag_printf(" - error %s\n", strerror(errno));
                fdprintf(client, " - error %s\n", strerror(errno));
            }
        } else {
            diag_printf(" [mode %08x ino %08x nlink %d size %d]\n",
                        sbuf.st_mode,sbuf.st_ino,sbuf.st_nlink,sbuf.st_size);
            fdprintf(client, " [mode %08x ino %08x nlink %d size %d]\n",
                     sbuf.st_mode,sbuf.st_ino,sbuf.st_nlink,sbuf.st_size);
        }
    }
    err = closedir( dirp );
}

static void
do_show(int client, char *buf)
{
    char name[256];

    buf += 4;  // Skip over command
    while (*buf && (*buf == ' ')) buf++;
    if (*buf) {
        // Name provided
        strcpy(name, buf);
    } else {
        fdprintf(client, "usage: show <file>\n");
        return;
    }
    show_jpeg(client, name, show_time);
}

static void
do_show_all(int client, char *buf)
{
    int err;
    DIR *dirp;
    char *c, name[256];

    buf += 8;  // Skip over command
    while (*buf && (*buf == ' ')) buf++;
    if (*buf) {
        // Name provided
        strcpy(name, buf);
    } else {
        strcpy(name, ".");
    }
    
    fdprintf(client, "<INFO> show .jpg files in directory %s\n",name);
    dirp = opendir( name );    
    if( dirp == NULL ) {
        fdprintf(client, "Can't open directory \"%s\"\n", name);
        return;
    }
    for(;;) {
        struct dirent *entry = readdir( dirp );
        struct stat sbuf;
        char fullname[PATH_MAX];
        
        if( entry == NULL )
            break;
        if( name[0] ) {
            strcpy(fullname, name );
            if( !(name[0] == '/' && name[1] == 0 ) )
                strcat(fullname, "/" );
        }
        else fullname[0] = 0;
        strcat(fullname, entry->d_name );
        err = stat( fullname, &sbuf );
        if( err < 0 ) {
            fdprintf(client, "<ERROR> Can't access \"%s\":", fullname);
            if( errno == ENOSYS ) {
                fdprintf(client, " <no status available>");
            } else {
                fdprintf(client, "%s\n", strerror(errno));
            }
        } else {
#if 0
            if (/* hack: !S_ISREG(sbuf.st_mode)*/ (sbuf.st_mode & 0x8000) == 0) {
                continue;
            }
#endif
            if ((c = rindex(fullname, '.')) != (char *)NULL) {
                if (strcmp(c, ".jpg") == 0) {
                    show_jpeg(client, fullname, show_time);
                }
            }
        }
    }
    err = closedir( dirp );
}

static void
do_time(int client, char *buf)
{
    char *cp;
    int val;

    buf += 4;  // Skip over command
    while (*buf && (*buf == ' ')) buf++;    
    if (*buf) {
        // value provided
        val = strtol(buf, &cp, 0);
        if (val > 0) {
            show_time = val;
            fdprintf(client, "<INFO> time set to %d seconds\n", val);
            return;
        }
    }
    fdprintf(client, "usage: time <value>\n");
}

static void
do_get(int client, char *buf)
{
    char *fn, *sn, *data;
#ifdef CYGPKG_FS_RAM
    char _fn[PATH_MAX];
#endif
    int fd, len, err;
    struct sockaddr_in srvr_addr;

    buf += 3;  // Skip over command
    fn = strtok(buf, " ,");
    sn = strtok(NULL, " ,");
    if ((fn == (char *)NULL) || (sn == (char *)NULL)) {
        fdprintf(client, "usage: get <file> <server>\n");
        return;
    }
    // For now, only numeric IP addresses
    if (!inet_aton(sn, &srvr_addr.sin_addr)) {
        fdprintf(client, "Can't get host info: %s\n", sn);
        return;
    }
    srvr_addr.sin_port = 0;
    if ((data = (char *)malloc(0x100000)) == (char *)NULL) {
        fdprintf(client, "Can't allocate temp buffer\n");
        return;
    }
    if ((len = tftp_get(fn, &srvr_addr, data, 0x100000, TFTP_OCTET, &err)) > 0) {
        fdprintf(client, "Read %d bytes\n", len);
        fd = open(fn, O_RDWR|O_CREAT);
        if (fd > 0) {
            err = write(fd, data, len);
            if (err != len) {
                fdprintf(client, "Error writing data\n");
            }
            close(fd);
        } else {
            fdprintf(client, "Can't create \"%s\"\n", fn);
        }
#ifdef CYGPKG_FS_RAM
        sprintf(_fn, "/%s", fn);
        fd = open(_fn, O_RDWR|O_CREAT);
        if (fd > 0) {
            err = write(fd, data, len);
            if (err != len) {
                fdprintf(client, "Error writing data\n");
            }
            close(fd);
        } else {
            fdprintf(client, "Can't create \"%s\"\n", _fn);
        }
#endif
    } else {
        fdprintf(client, "Error reading data\n");
    }
    free(data);
}

static void
do_rm(int client, char *buf)
{
    char *fn;

    buf += 2;  // Skip over command
    fn = strtok(buf, " ,");
    if (fn == (char *)NULL) {
        fdprintf(client, "usage: rm <file>\n");
        return;
    }
    if (unlink(fn) < 0) {
        fdprintf(client, "Can't remove \"%s\": %s\n", fn, strerror(errno));
    }
}

static void
do_cmd(int client, char *buf)
{
    char *cp = buf+strlen(buf)-1;
    while ((*cp == '\n') || (*cp == '\r')) {
        *cp-- = '\0';  // Remove trailing terminators
    }
    printf("Command: %s\n", buf);
    if (strncmp(buf, "ls", 2) == 0) {
        do_ls(client, buf);
    } else
    if (strncmp(buf, "show_all", 8) == 0) {
        do_show_all(client, buf);
    } else
    if (strncmp(buf, "show", 4) == 0) {
        do_show(client, buf);
    } else
    if (strncmp(buf, "time", 4) == 0) {
        do_time(client, buf);
    } else
    if (strncmp(buf, "get", 3) == 0) {
        do_get(client, buf);
    } else
    if (strncmp(buf, "rm", 2) == 0) {
        do_rm(client, buf);
    } else
    {
        fdprintf(client, "Unknown command: %s\n", buf);
    }
}

static void
do_copy_file(char *src, char *dst)
{
    int err, srcfd, dstfd, len;
    char *buf;

#define COPY_SIZE 0x1000
    if ((srcfd = open(src, O_RDONLY)) >= 0) {
        if ((dstfd = open(dst, O_RDWR|O_CREAT)) >= 0) {
            if ((buf = (char *)malloc(COPY_SIZE)) != (char *)NULL) {
                while ((len = read(srcfd, buf, COPY_SIZE)) > 0) {
                    write(dstfd, buf, len);
                }
                free(buf);
            } else {
                diag_printf("Can't allocate working buffer\n");
            }
            close(srcfd);
            close(dstfd);
        } else {                        
            diag_printf("Can't create \"%s\": %s\n", dst, strerror(errno));
            close(srcfd);
        }
    } else {
        diag_printf("Can't open \"%s\": %s\n", src, strerror(errno));
    }
}

static void
do_copy_all(char *src, char *dst)
{
    int err;
    DIR *dirp;
    char *c, *buf;

    diag_printf("Copy all files from %s to %s\n", src, dst);
    dirp = opendir(src);    
    if(dirp == NULL) {
        diag_printf("Can't open directory \"%s\"\n", src);
        return;
    }
    for(;;) {
        struct dirent *entry = readdir( dirp );
        char srcname[PATH_MAX];
        char dstname[PATH_MAX];
        
        if( entry == NULL )
            break;
        strcpy(srcname, src);
        strcpy(dstname, dst);
        if( !(src[0] == '/' && src[1] == 0 ) )
            strcat(srcname, "/" );
        if( !(dst[0] == '/' && dst[1] == 0 ) )
            strcat(dstname, "/" );
        strcat(srcname, entry->d_name );
        strcat(dstname, entry->d_name );
        if ((c = rindex(srcname, '.')) != (char *)NULL) {
            if (strcmp(c, ".jpg") == 0) {
                diag_printf("Copy %s => %s\n", srcname, dstname); 
                do_copy_file(srcname, dstname);
            }
        }
    }
    closedir(dirp);
}

static void
pexit(char *why)
{
    printf("Image demo thread exiting - %s\n", why);
    GrClose();
    cyg_thread_exit();
}

int 
img_demo_thread(CYG_ADDRWORD data)
{
    int err, s, client, client_len;
    struct sockaddr client_addr;
    struct sockaddr_in my_addr;
    struct addrinfo *ai, *addrs, hints;
    char buf[256], addr_buf[256];
    int one = 1;
    fd_set in_fds, src_fds;
    int num, len;
    struct timeval tv;

    INIT_PER_THREAD_DATA();

    printf("Image demo here\n");
#ifdef CYGPKG_FS_RAM
    // Stage files from JFFS2 image into RAM
    err = mount("", "/ramfs", "ramfs");        
    if (err >= 0) {
        do_copy_all("/", "/ramfs");
    } else {
        pexit("Can't mount RAMfs\n");
    }
    chdir("/ramfs");
#endif
    if(GrOpen() < 0) {
        fprintf(stderr, "Couldn't connect to Nano-X server\n");
        exit(1);
    }
    // Set up as a generic server, listening on TCP/7734
#if 0
    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;b
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((err = getaddrinfo(NULL, "7734", &hints, &addrs)) != EAI_NONE) {
        diag_printf("can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    s = socket(ai->ai_family, ai->ai_socktype, 0);
    if (s < 0) {
        pexit("stream socket");
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        pexit("setsockopt SO_REUSEADDR");
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        pexit("setsockopt SO_REUSEPORT");
    }
    if(bind(s, ai->ai_addr, ai->ai_addr->sa_len) < 0) {
        pexit("bind error");
    }
    listen(s, SOMAXCONN);
#else
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        pexit("stream socket");
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        pexit("setsockopt SO_REUSEADDR");
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        pexit("setsockopt SO_REUSEPORT");
    }
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_len = sizeof(my_addr);
    my_addr.sin_port = htons(7734);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(s, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) {
        pexit("bind error");
    }
    listen(s, SOMAXCONN);
#endif
    while (true) {
        // Wait for a connection
        FD_ZERO(&src_fds);
        FD_SET(s, &src_fds);
        tv.tv_sec = 15;
        tv.tv_usec = 0;
        num = select(s+1, &src_fds, 0, 0, &tv);
        if (num > 0) {
            client_len = sizeof(client_addr);
            if ((client = accept(s, (struct sockaddr *)&client_addr, &client_len)) < 0) {
                pexit("accept");
            }
            client_len = sizeof(client_addr);
            getpeername(client, &client_addr, &client_len);
            _inet_ntop(&client_addr, addr_buf, sizeof(addr_buf));
            diag_printf("connection from %s(%d)\n", addr_buf, ntohs(_inet_port(&client_addr)));
            fdprintf(client, "Hello %s(%d)\n", addr_buf, ntohs(_inet_port(&client_addr)));
            while (true) {
                fdprintf(client, "// Ready\n");
                tv.tv_sec = 5;
                tv.tv_usec = 0;
                FD_ZERO(&in_fds);
                FD_SET(client, &in_fds);
                num = select(client+1, &in_fds, 0, 0, &tv);
                if (num > 0) {
                    len = read(client, buf, sizeof(buf)-1);
                    if (len <= 0) {
                        diag_printf("Client read error: %s\n", strerror(errno));
                        break;
                    }
                    buf[len-1] = '\0';
                    do_cmd(client, buf);
                } else if (num == 0) {
                    fdprintf(client, "<IDLE> show_all\n");
                    do_show_all(client, "show_all .");
                } else {
                    perror("select");
                }
            }
            close(client);
            diag_printf("Connection with %s closed\n", addr_buf);
        } else if (num == 0) {
            // No connection within 15 seconds
            do_show_all(0, "show_all .");
        } else {
            diag_printf("select returned: %d\n", num);
            pexit("bad select");
        }
    }
}
