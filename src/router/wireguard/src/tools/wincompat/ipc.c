// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>

static FILE *userspace_interface_file(const char *interface)
{
	char fname[MAX_PATH];
	HANDLE thread_token, process_snapshot, winlogon_process, winlogon_token, duplicated_token, pipe_handle;
	PROCESSENTRY32 entry = { .dwSize = sizeof(PROCESSENTRY32) };
	BOOL ret;
	DWORD pid = 0, last_error;
	TOKEN_PRIVILEGES privileges = {
		.PrivilegeCount = 1,
		.Privileges = {{ .Attributes = SE_PRIVILEGE_ENABLED }}
	};
	int fd;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &privileges.Privileges[0].Luid)) {
		fprintf(stderr, "Error: LookupPrivilegeValue: 0x%lx\n", GetLastError());
		return NULL;
	}
	if (!ImpersonateSelf(SecurityImpersonation)) {
		fprintf(stderr, "Error: ImpersonateSelf: 0x%lx\n", GetLastError());
		return NULL;
	}
	if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES, FALSE, &thread_token)) {
		fprintf(stderr, "Error: OpenThreadToken: 0x%lx\n", GetLastError());
		RevertToSelf();
		return NULL;
	}
	if (!AdjustTokenPrivileges(thread_token, FALSE, &privileges, sizeof(privileges), NULL, NULL)) {
		fprintf(stderr, "Error: AdjustTokenPrivileges: 0x%lx\n", GetLastError());
		CloseHandle(thread_token);
		RevertToSelf();
		return NULL;
	}
	CloseHandle(thread_token);

	process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (process_snapshot == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error: CreateToolhelp32Snapshot: 0x%lx\n", GetLastError());
		RevertToSelf();
		return NULL;
	}
	for (ret = Process32First(process_snapshot, &entry); ret; ret = Process32Next(process_snapshot, &entry)) {
		//TODO: This isn't very smart matching. An attacker can DoS us by just opening processes
		// with the same name. Instead we should be looking for tokens that we can actually use.
		if (!strcasecmp(entry.szExeFile, "winlogon.exe")) {
			pid = entry.th32ProcessID;
			break;
		}
	}
	CloseHandle(process_snapshot);
	if (!pid) {
		fprintf(stderr, "Error: unable to find winlogon.exe\n");
		RevertToSelf();
		return NULL;
	}

	winlogon_process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!winlogon_process) {
		fprintf(stderr, "Error: OpenProcess: 0x%lx\n", GetLastError());
		RevertToSelf();
		return NULL;
	}

	if (!OpenProcessToken(winlogon_process, TOKEN_IMPERSONATE | TOKEN_DUPLICATE, &winlogon_token)) {
		fprintf(stderr, "Error: OpenProcessToken: 0x%lx\n", GetLastError());
		CloseHandle(winlogon_process);
		RevertToSelf();
		return NULL;
	}
	CloseHandle(winlogon_process);

	if (!DuplicateToken(winlogon_token, SecurityImpersonation, &duplicated_token)) {
		fprintf(stderr, "Error: DuplicateToken: 0x%lx\n", GetLastError());
		CloseHandle(winlogon_token);
		RevertToSelf();
		return NULL;
	}
	CloseHandle(winlogon_token);

	if (!SetThreadToken(NULL, duplicated_token)) {
		fprintf(stderr, "Error: SetThreadToken: 0x%lx\n", GetLastError());
		CloseHandle(duplicated_token);
		RevertToSelf();
		return NULL;
	}
	CloseHandle(duplicated_token);

	snprintf(fname, sizeof(fname), "\\\\.\\pipe\\WireGuard\\%s", interface);
	pipe_handle = CreateFile(fname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	last_error = GetLastError();
	RevertToSelf();
	if (pipe_handle == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error: CreateFile: 0x%lx\n", last_error);
		return NULL;
	}
	fd = _open_osfhandle((intptr_t)pipe_handle, _O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "Error: _open_osfhandle: 0x%lx\n", GetLastError());
		CloseHandle(pipe_handle);
		return NULL;
	}
	return _fdopen(fd, "r+");
}

static int userspace_get_wireguard_interfaces(struct inflatable_buffer *buffer)
{
	WIN32_FIND_DATA find_data;
	HANDLE find_handle;
	int ret = 0;

	find_handle = FindFirstFile("\\\\.\\pipe\\*", &find_data);
	if (find_handle == INVALID_HANDLE_VALUE)
		return -GetLastError();
	do {
		if (strncmp("WireGuard\\", find_data.cFileName, 10))
			continue;
		buffer->next = strdup(find_data.cFileName + 10);
		buffer->good = true;
		ret = add_next_to_inflatable_buffer(buffer);
		if (ret < 0)
			goto out;
	} while (FindNextFile(find_handle, &find_data));

out:
	FindClose(find_handle);
	return ret;
}
