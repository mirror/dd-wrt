/*
 * Copyright (C) 2019 Trond Myklebust <trond.myklebust@hammerspace.com>
 */
#ifndef XSTAT_H
#define XSTAT_H

struct stat;

int xlstat(const char *pathname, struct stat *statbuf);
int xstat(const char *pathname, struct stat *statbuf);
#endif
