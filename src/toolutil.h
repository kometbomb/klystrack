#ifndef TOOLUTIL_H
#define TOOLUTIL_H

#include <stdio.h>

FILE *open_dialog(const char *mode, wchar_t *title, wchar_t *filter);
int confirm(const char *msg);
int confirm_ync(const char *msg);

#endif
