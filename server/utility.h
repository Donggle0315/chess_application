#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <unistd.h>
#include <stdio.h>
#include "structures_constants.h"

int readall(int fd, char *buf, int count);
int writeall(int fd, char *buf, int count);
void wrappedWriteAll(SendInfo * si);

#endif