/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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

#ifdef _WIN32

#include <windows.h>

int
main(int argc, char *argv[])
{
  STARTUPINFO StartInfo;
  PROCESS_INFORMATION ProcInfo;
  int i;
  char *CmdLine;
  char *Walker;
  char NewCmdLine[MAX_PATH + 500];
  HANDLE Handles[2];
  unsigned long Res;
  int Quotes;

  Handles[0] = OpenEvent(EVENT_ALL_ACCESS, FALSE, "TheOlsrdShimEvent");

  if (Handles[0] == NULL) {
    MessageBox(NULL, "Cannot open event.", "Shim Error", MB_ICONERROR | MB_OK);
    ExitProcess(1);
  }

  CmdLine = GetCommandLine();

  Quotes = 0;

  while (*CmdLine != 0) {
    if (*CmdLine == '"')
      Quotes = !Quotes;

    else if (*CmdLine == ' ' && !Quotes)
      break;

    CmdLine++;
  }

  if (*CmdLine == 0) {
    MessageBox(NULL, "Missing arguments.", "Shim Error", MB_ICONERROR | MB_OK);
    ExitProcess(1);
  }

  GetModuleFileName(NULL, NewCmdLine, MAX_PATH);

  for (Walker = NewCmdLine; *Walker != 0; Walker++);

  while (*Walker != '\\')
    Walker--;

  Walker[1] = 'o';
  Walker[2] = 'l';
  Walker[3] = 's';
  Walker[4] = 'r';
  Walker[5] = 'd';
  Walker[6] = '.';
  Walker[7] = 'e';
  Walker[8] = 'x';
  Walker[9] = 'e';

  Walker[10] = ' ';

  Walker += 11;

  while ((*Walker++ = *CmdLine++) != 0);

  for (i = 0; i < sizeof(STARTUPINFO); i++)
    ((char *)&StartInfo)[i] = 0;

  StartInfo.cb = sizeof(STARTUPINFO);

  if (!CreateProcess(NULL, NewCmdLine, NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &StartInfo, &ProcInfo)) {
    MessageBox(NULL, "Cannot execute OLSR server.", "Shim Error", MB_ICONERROR | MB_OK);
    ExitProcess(1);
  }

  Handles[1] = ProcInfo.hProcess;

  Res = WaitForMultipleObjects(2, Handles, FALSE, INFINITE);

  if (Res == WAIT_OBJECT_0) {
    GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, ProcInfo.dwProcessId);
    WaitForSingleObject(ProcInfo.hProcess, INFINITE);
  }

  CloseHandle(ProcInfo.hThread);
  CloseHandle(ProcInfo.hProcess);

  ExitProcess(0);
}

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
