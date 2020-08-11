#ifndef __ayyi_debug_h__
#define __ayyi_debug_h__

#include "glib.h"

#define dbg(A, B, ...) debug_printf(__func__, A, B, ##__VA_ARGS__)
#define gerr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)
#define perr(A, ...) errprintf2(__func__, A"\n", ##__VA_ARGS__)
#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define pwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__)
#define PF0 {printf("%s()...\n", __func__);}
#define PF {if(_debug_) printf("%s()...\n", __func__);}
#define PF2 {if(_debug_ > 1) printf("%s()...\n", __func__);}
#define PF_DONE printf("%s(): done.\n", __func__);
#define P_GERR if(error){ gerr("%s\n", error->message); g_error_free(error); error = NULL; }
#define GERR_INFO if(error){ printf("%s\n", error->message); g_error_free(error); error = NULL; }
#define GERR_WARN if(error){ gwarn("%s", error->message); g_error_free(error); error = NULL; }

void        debug_printf             (const char* func, int level, const char* format, ...);
void        warnprintf               (char* format, ...);
void        warnprintf2              (const char* func, char* format, ...);
void        errprintf                (char* fmt, ...);
void        errprintf2               (const char* func, char* format, ...);

void        log_handler              (const gchar* log_domain, GLogLevelFlags, const gchar* message, gpointer);

#ifdef __debug_c__
char ayyi_warn[32] = "\x1b[1;33mwarning:\x1b[0;39m";
char ayyi_err [32] = "\x1b[1;31merror!\x1b[0;39m";
#else
extern int _debug_;                  // debug level. 0=off.

extern char ayyi_warn[32];
extern char ayyi_err [32];
#endif

#endif //__ayyi_debug_h__
