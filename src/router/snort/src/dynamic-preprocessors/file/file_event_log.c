/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2013-2013 Sourcefire, Inc.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License Version 2 as
 ** published by the Free Software Foundation.  You may not use, modify or
 ** distribute this program under any other version of the GNU General
 ** Public License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  4.11.2013 - Initial Source Code.
 **  Log file events
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#include "event.h"
#include "sf_snort_packet.h"
#include "snort_debug.h"
#include "sf_textlog.h"
#include "output_api.h"
#include "output_lib.h"
#include "sf_dynamic_common.h"
#include "file_sha.h"
#include "file_event_log.h"
#include "sf_dynamic_preprocessor.h"


#define OUTPUT_MOD_VERSION  1
#define OUTPUT_NAME "filelog"
#define OUTPUT_TYPE (DYNAMIC_OUTPUT_TYPE_FLAG__ALERT)


/* full buf was chosen to allow printing max size packets
 * in hex/ascii mode:
 * each byte => 2 nibbles + space + ascii + overhead
 */
#define FULL_BUF  (4*65535)
#define FAST_BUF  (4*K_BYTES)

/*
 * not defined for backwards compatibility
 * (default is produced by OpenAlertFile()
#define DEFAULT_FILE  "alert.fast"
 */
#define DEFAULT_LIMIT (128*M_BYTES)

typedef struct
{
    TextLog* log;
    uint8_t packet_flag;
} FileLogData;

static void file_log_init(struct _SnortConfig *, char* args);
static void file_log_term(int, void* arg);
static int file_log_parse (void** config, char* args, const char* default_output_file);
static void file_log_exec(void* packet, char* msg, void* arg, void* eventInfo);

OUTPUT_SO_PUBLIC Output_Module_t OUTPUT_MODULE_DATA =
{
        .api_major_version = OUTPUT_API_MAJOR_VERSION,
        .api_minor_version = OUTPUT_API_MINOR_VERSION,
        .module_version = OUTPUT_MOD_VERSION,
        .name = OUTPUT_NAME,
        .type = OUTPUT_TYPE,
        .default_file = "file",
        .load = file_log_init,
        .parse_args = file_log_parse,
        .postconfig = NULL,
        .alert_output = file_log_exec,
        .log_output = NULL,
        .rotate = NULL,
        .shutdown = file_log_term,
        .next = NULL
};

/*********************Output plugin for file log*******************************/
static char* getFullName (char* dir, const char* filespec)
{
    char *filename = NULL;
    char buffer[STD_BUF + 1];

    if(filespec == NULL)
    {
        _dod.fatalMsg("no argument in this file option, "
                "remove extra ':' at the end of the alert option\n");
    }

    buffer[STD_BUF] = '\0';

    if(filespec[0] == '/')
    {
        /* absolute filespecs are saved as is */
        filename = strdup(filespec);
    }
    else
    {
        /* relative filespec is considered relative to the log directory */
        /* or /var/log if the log directory has not been set */
        /* Make sure this function isn't called before log dir is set */
        if (dir != NULL)
        {
            strncpy(buffer, dir, sizeof(buffer) - 1);
        }
        else
        {
            strncpy(buffer, "/var/log/snort", sizeof(buffer) - 1);
        }

        strncat(buffer, "/", STD_BUF - strlen(buffer));
        strncat(buffer, filespec, STD_BUF - strlen(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        filename = strdup(buffer);
    }

    return filename;
}
static void file_log_init (struct _SnortConfig *sc, char* args)
{
    init_output_module(sc, &OUTPUT_MODULE_DATA, args);
    _dod.logMsg("%s: version %d\n", __FUNCTION__, OUTPUT_MOD_VERSION);

}

static void file_log_term (int unused, void* arg)
{
    FileLogData *data = (FileLogData *)arg;
    DEBUG_WRAP(_dod.debugMsg(DEBUG_LOG, "%s\n", "FileLog_Term"););

    /*free memory from FileLogData */
    if ( data->log ) _dod.textLog_Term(data->log);
    free(data);

    /*free memory from FileLogData */
}

static void file_log_exec(void* packet, char* msg, void* arg, void* eventInfo)
{
    FileLogData *data = (FileLogData *)arg;
    SFSnortPacket *p = (SFSnortPacket*)packet;
    Event *event = (Event *)eventInfo;

    uint8_t filename[STD_BUF];
    uint8_t *file_name;
    uint32_t file_name_len = 0;

    if ( !event || ((event->sig_generator != GENERATOR_FILE_TYPE)
            && (event->sig_generator != GENERATOR_FILE_SIGNATURE)))
        return;

    _dod.logTimeStamp(data->log, p);

    /*Get the file name captured*/
    if(_dpd.fileAPI->get_file_name(p->stream_session, &file_name, &file_name_len))
    {
        FileCharEncoding encoding = SNORT_CHAR_ENCODING_ASCII;
        if(file_name_len > 0)
            encoding = _dpd.fileAPI->get_character_encoding(file_name, file_name_len);
        if(SNORT_CHAR_ENCODING_ASCII == encoding)
        {
            if (file_name_len >= sizeof(filename))
                file_name_len = sizeof(filename) - 1;
            memcpy(filename, file_name, file_name_len);
            filename[file_name_len] = '\0';
            _dod.textLog_Puts(data->log, " [**] ");
            _dod.textLog_Print(data->log, " [File: %s, size: %d bytes]", filename,
                    _dpd.fileAPI->get_file_size(p->stream_session));
        }
        else if(SNORT_CHAR_ENCODING_UTF_16LE == encoding)
        {
            if(file_name_len > (sizeof(filename) - 2))
                file_name_len = sizeof(filename) - 2;//Last 2 bytes reserved for NULL termination
            memcpy(filename, file_name, file_name_len);
            filename[file_name_len] = '\0';
            filename[file_name_len + 1] = '\0';
            _dod.textLog_Puts(data->log, " [**]  [File: ");
            _dod.textLog_Flush(data->log);
            _dod.textLog_PrintUnicode(data->log, filename + UTF_16_LE_BOM_LEN, file_name_len, true);
            _dod.textLog_Print(data->log, ", size: %d bytes]", _dpd.fileAPI->get_file_size(p->stream_session));
        }
        else
        {
            _dod.textLog_Puts(data->log, " [**] ");
            _dod.textLog_Print(data->log, " [File: %s, size: %d bytes]", "<<Filename Encoding not supported>>",
                    _dpd.fileAPI->get_file_size(p->stream_session));
        }
    }

    if (event->sig_generator == GENERATOR_FILE_SIGNATURE)
    {
        char sha256[SHA256_HASH_STR_SIZE + 1];
        sha_to_str((char *)_dpd.fileAPI->get_sig_sha256(p->stream_session),
                sha256, SHA256_HASH_STR_SIZE + 1);
        sha256[SHA256_HASH_STR_SIZE] = '\0';
        _dod.textLog_Print(data->log, " [signature: %s]", sha256);
    }

    if( p != NULL && _dod.active_PacketWasDropped() )
    {
        _dod.textLog_Puts(data->log, " [Drop]");
    }
    else if( p != NULL && _dod.active_PacketWouldBeDropped() )
    {
        _dod.textLog_Puts(data->log, " [WDrop]");
    }

    _dod.textLog_Puts(data->log, " [**] ");
    _dod.textLog_Print(data->log, "[%lu:%lu:%lu] ",
            (unsigned long) event->sig_generator,
            (unsigned long) event->sig_id,
            (unsigned long) event->sig_rev);

    if (_dod.ScAlertInterface())
    {
        _dod.textLog_Print(data->log, " <%s> ", _dod.getDAQinterface());
    }

    /*Ignore event message for file signature events*/
    if ((msg != NULL) && (event->sig_generator == GENERATOR_FILE_TYPE))
    {
        _dod.textLog_Puts(data->log, msg);
    }

    _dod.textLog_Puts(data->log, " [**] ");

    /* print the packet header to the alert file */
    if ((p != NULL) && IPH_IS_VALID(p))
    {
        _dod.textLog_Print(data->log, "{%s} ", _dod.getProtocolName(p));
        _dod.logIpAddrs(data->log, p);
    }

    _dod.textLog_NewLine(data->log);
    _dod.textLog_Flush(data->log);

}

static int file_log_parse (void** config, char* args, const char* default_output_file)
{
    char *tok;
    FileLogData *data;
    char* filename = NULL;
    unsigned long limit = DEFAULT_LIMIT;
    unsigned int bufSize = FAST_BUF;
    int i = 0;

    DEBUG_WRAP(_dod.debugMsg(DEBUG_LOG, "FileLog_Parse: %s\n", args););
    data = (FileLogData *)calloc(1, sizeof(*data));

    if ( !data )
    {
        _dod.fatalMsg("file_log: unable to allocate memory!\n");
        return OUTPUT_ERROR;
    }
    if ( !args ) args = "";

    tok = strtok( args, " \t");

    while ( tok )
    {
        char *end;

        switch (i)
        {
        case 0:
            if ( !strcasecmp(tok, "stdout") )
            {
                filename = strdup(tok);
                if (!filename)
                    _dod.fatalMsg("file_log error in %s(%i): %s\n",
                            *(_dod.config_file), *(_dod.config_line), tok);
            }
            else
            {
                char *dir = _dod.getLogDirectory();
                filename = getFullName(dir, tok);
            }
            break;

        case 1:
            if ( !strcasecmp("packet", tok) )
            {
                data->packet_flag = 1;
                bufSize = FULL_BUF;
                break;
            }
            /* in this case, only 2 options allowed */
            else i++;
            /* fall thru so "packet" is optional ... */

        case 2:
            limit = strtol(tok, &end, 10);

            if ( tok == end )
                _dod.fatalMsg("file_log error in %s(%i): %s\n",
                        *(_dod.config_file), *(_dod.config_line), tok);

            if ( end && toupper(*end) == 'G' )
                limit <<= 30; /* GB */

            else if ( end && toupper(*end) == 'M' )
                limit <<= 20; /* MB */

            else if ( end && toupper(*end) == 'K' )
                limit <<= 10; /* KB */
            break;

        case 3:
            _dod.fatalMsg("file_log: error in %s(%i): %s\n",
                    *(_dod.config_file), *(_dod.config_line), tok);
            break;
        }
        tok = strtok( NULL, " " );
        i++;
    }


#ifdef DEFAULT_FILE
    if ( !filename ) filename = getFullName(_dod.getLogDirectory(), DEFAULT_FILE);
#endif

    DEBUG_WRAP(_dod.debugMsg(
            DEBUG_INIT, "file_log: '%s' %d %ld\n",
            filename ? filename:"alert", data->packet_flag, limit ););

    if ((filename == NULL) && (_dod.getAlertFile() != NULL))
        filename = _dod.SnortStrdup(_dod.getAlertFile());

    data->log = _dod.textLog_Init(filename, bufSize, limit);

    if (filename != NULL)
        free(filename);

    *config = data;

    return OUTPUT_SUCCESS;
}
