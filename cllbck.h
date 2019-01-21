
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "bblio.h"
#include "bblls.h"
#include "bblht.h"

extern void fv_getcnf(char *, char **);

extern int  gi_cps;
extern char gr_lsn[16];
