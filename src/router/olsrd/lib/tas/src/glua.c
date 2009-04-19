
/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * Copyright (c) 2004, Thomas Lopatic (thomas@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include "link.h"
#include "plugin.h"
#include "lib.h"
#include "os_unix.h"
#include "http.h"
#include "glua.h"
#include "glua_ext.h"

#include <stdio.h>
#include <string.h>

static const char infoKey;
static const char keepFlagKey;
static const char argListKey;

int
lspToLua(const char *rootDir, const char *lspFileName, const char *workDir, const char *luaFileName)
{
  FILE *file;
  int lspLen;
  unsigned char *buff;
  int start, code, i, k;
  char *lspPath = fullPath(rootDir, lspFileName);
  char *luaPath = fullPath(workDir, luaFileName);

  if (fileIsNewer(lspPath, luaPath) == 0) {
    freeMem(lspPath);
    freeMem(luaPath);
    return 0;
  }

  file = fopen(lspPath, "r");

  if (file == NULL) {
    error("cannot open %s\n", lspPath);
    freeMem(lspPath);
    freeMem(luaPath);
    return -1;
  }

  if (fseek(file, 0, SEEK_END) < 0) {
    error("cannot seek to end of %s\n", lspPath);
    fclose(file);
    freeMem(lspPath);
    freeMem(luaPath);
    return -1;
  }

  lspLen = ftell(file);

  if (lspLen < 0) {
    error("cannot determine length of %s\n", lspPath);
    fclose(file);
    freeMem(lspPath);
    freeMem(luaPath);
    return -1;
  }

  if (fseek(file, 0, SEEK_SET) < 0) {
    error("cannot seek to beginning of %s\n", lspPath);
    fclose(file);
    freeMem(lspPath);
    freeMem(luaPath);
    return -1;
  }

  buff = allocMem(lspLen);

  if (fread(buff, lspLen, 1, file) != 1) {
    error("cannot read %s\n", lspPath);
    fclose(file);
    freeMem(lspPath);
    freeMem(luaPath);
    freeMem(buff);
    return -1;
  }

  fclose(file);

  if (createAllDirs(luaPath) < 0) {
    error("cannot create required directories for %s\n", luaPath);
    freeMem(lspPath);
    freeMem(luaPath);
    freeMem(buff);
    return -1;
  }

  file = fopen(luaPath, "w");

  if (file == NULL) {
    error("cannot open %s\n", luaPath);
    freeMem(lspPath);
    freeMem(luaPath);
    freeMem(buff);
    return -1;
  }

  start = 0;
  code = 0;

  i = 0;

  for (;;) {
    if (code == 0 && (i == lspLen || strncmp((char *)(buff + i), "<?lua", 5) == 0)) {
      fprintf(file, "tas.write(\"");

      for (k = start; k < i; k++) {
        if (buff[k] == 13)
          continue;

        if (buff[k] == '\\' || buff[k] == '"' || buff[k] == 10)
          fputc('\\', file);

        fputc(buff[k], file);
      }

      fprintf(file, "\")\n");

      if (i == lspLen)
        break;

      if (buff[i + 5] == '=') {
        i += 6;
        code = 2;
      }

      else {
        i += 5;
        code = 1;
      }

      start = i;

      continue;
    }

    if (code > 0 && (i == lspLen || strncmp((char *)(buff + i), "?>", 2) == 0)) {
      if (code > 1)
        fprintf(file, "tas.write(");

      for (k = start; k < i; k++)
        if (buff[k] != 13)
          fputc(buff[k], file);

      if (code > 1)
        fputc(')', file);

      fputc('\n', file);

      if (i == lspLen)
        break;

      i += 2;
      start = i;

      code = 0;

      continue;
    }

    i++;
  }

  fclose(file);
  freeMem(lspPath);
  freeMem(luaPath);
  freeMem(buff);
  return 0;
}

static int
luaWriter(lua_State * lua __attribute__ ((unused)), const void *buff, int len, FILE * file)
{
  return fwrite(buff, len, 1, file) == 1;
}

int
luaToLex(char **errMsg, const char *workDir, const char *luaFileName, const char *lexFileName)
{
  lua_State *lua;
  int res;
  FILE *file;
  char *luaPath = fullPath(workDir, luaFileName);
  char *lexPath = fullPath(workDir, lexFileName);

  *errMsg = NULL;

  if (fileIsNewer(luaPath, lexPath) == 0) {
    freeMem(luaPath);
    freeMem(lexPath);
    return 0;
  }

  lua = lua_open();

  res = luaL_loadfile(lua, luaPath);

  if (res != 0) {
    *errMsg = myStrdup(lua_tostring(lua, -1));
    error("cannot load %s: %s\n", luaPath, *errMsg);
    lua_close(lua);
    freeMem(luaPath);
    freeMem(lexPath);
    return -1;
  }

  file = fopen(lexPath, "wb");

  if (file == NULL) {
    error("cannot open %s\n", lexPath);
    lua_close(lua);
    freeMem(luaPath);
    freeMem(lexPath);
    return -1;
  }

  lua_dump(lua, (lua_Chunkwriter) luaWriter, file);

  fclose(file);

  lua_close(lua);
  freeMem(luaPath);
  freeMem(lexPath);
  return 0;
}

static int
tasWrite(lua_State * lua)
{
  int numArg = lua_gettop(lua);
  const char *strConv;
  int i;
  struct connInfo *info;

  lua_pushlightuserdata(lua, &infoKey);
  lua_gettable(lua, LUA_REGISTRYINDEX);

  info = lua_touserdata(lua, -1);

  lua_getglobal(lua, "tostring");

  for (i = 1; i <= numArg; i++) {
    lua_pushvalue(lua, -1);
    lua_pushvalue(lua, i);

    lua_call(lua, 1, 1);

    strConv = lua_tostring(lua, -1);

    if (strConv == NULL)
      return luaL_error(lua, "cannot convert value to string");

    writeBuff(&info->write[2], (const unsigned char *)strConv, strlen(strConv));

    lua_pop(lua, 1);
  }

  return 0;
}

static int
tasAddHeaderLine(lua_State * lua)
{
  struct connInfo *info;
  char *line;

  lua_pushlightuserdata(lua, &infoKey);
  lua_gettable(lua, LUA_REGISTRYINDEX);

  info = lua_touserdata(lua, -1);

  line = myStrdup(luaL_checkstring(lua, 1));

  chomp(line, strlen(line));

  writeBuff(&info->write[1], (const unsigned char *)line, strlen(line));
  writeBuff(&info->write[1], (const unsigned char *)"\r\n", 2);

  freeMem(line);

  return 0;
}

static int
tasSetContentType(lua_State * lua)
{
  struct connInfo *info;
  const char *contType;
  char *s;

  lua_pushlightuserdata(lua, &infoKey);
  lua_gettable(lua, LUA_REGISTRYINDEX);

  info = lua_touserdata(lua, -1);

  contType = luaL_checkstring(lua, 1);

  s = allocBuff(info, strlen(contType) + 1);
  strcpy(s, contType);

  return 0;
}

static int
tasKeepState(lua_State * lua)
{
  int *keepFlag;

  lua_pushlightuserdata(lua, &keepFlagKey);
  lua_gettable(lua, LUA_REGISTRYINDEX);

  keepFlag = lua_touserdata(lua, -1);

  *keepFlag = 1;

  return 0;
}

static int
tasGetParameters(lua_State * lua)
{
  int i;
  char **argList;

  lua_pushlightuserdata(lua, &argListKey);
  lua_gettable(lua, LUA_REGISTRYINDEX);

  argList = lua_touserdata(lua, -1);

  lua_newtable(lua);

  if (argList == NULL)
    return 1;

  for (i = 0; argList[i] != NULL; i += 2) {
    lua_pushstring(lua, argList[i]);
    lua_pushstring(lua, argList[i + 1]);
    lua_settable(lua, -3);
  }

  return 1;
}

static const struct luaL_reg tasLib[] = {
  {"write", tasWrite},
  {"set_content_type", tasSetContentType},
  {"add_header_line", tasAddHeaderLine},
  {"keep_state", tasKeepState},
  {"get_parameters", tasGetParameters},
#ifdef TAS_EXTRA_FUNCTIONS
  TAS_EXTRA_FUNCTIONS
#endif
  {NULL, NULL}
};

static int
luaopen_tas(lua_State * lua)
{
  luaL_openlib(lua, "tas", tasLib, 0);
  return 1;
}

int
runLua(char **errMsg, struct connInfo *info, const char *workDir, const char *lexFileName, char **argList, void **session)
{
  lua_State *lua;
  int res;
  char *lexPath = fullPath(workDir, lexFileName);
  int keepFlag = 0;

  *errMsg = NULL;

  if (*session == NULL) {
    lua = lua_open();

    luaopen_base(lua);
    luaopen_table(lua);
    luaopen_io(lua);
    luaopen_string(lua);
    luaopen_math(lua);
    luaopen_debug(lua);
    luaopen_loadlib(lua);

    luaopen_tas(lua);
  }

  else
    lua = *session;

  lua_pushlightuserdata(lua, &infoKey);
  lua_pushlightuserdata(lua, info);
  lua_settable(lua, LUA_REGISTRYINDEX);

  lua_pushlightuserdata(lua, &argListKey);
  lua_pushlightuserdata(lua, argList);
  lua_settable(lua, LUA_REGISTRYINDEX);

  lua_pushlightuserdata(lua, &keepFlagKey);
  lua_pushlightuserdata(lua, &keepFlag);
  lua_settable(lua, LUA_REGISTRYINDEX);

  res = luaL_loadfile(lua, lexPath);

  if (res != 0) {
    *errMsg = myStrdup(lua_tostring(lua, -1));
    error("cannot load %s: %s\n", lexPath, *errMsg);
    lua_close(lua);
    freeMem(lexPath);
    return -1;
  }

  res = lua_pcall(lua, 0, 0, 0);

  if (res != 0) {
    *errMsg = myStrdup(lua_tostring(lua, -1));
    error("cannot run %s: %s\n", lexPath, *errMsg);
    lua_close(lua);
    freeMem(lexPath);
    return -1;
  }

  if (keepFlag == 0) {
    lua_close(lua);
    *session = NULL;
  }

  else
    *session = lua;

  freeMem(lexPath);
  return 0;
}

void
freeLuaSession(void *session)
{
  lua_close(session);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
