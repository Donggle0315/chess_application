#ifndef WLOG
#define WLOG

#include <stdio.h>
inline void print_error(const char *file_path, const int line_num,
                        const char *function_name, const char *error) {
    fprintf(stderr, "file: %s\nline %d, function:%s : %s\n", file_path,
            line_num, function_name, error);
}

#ifdef __DEBUG__
#define ERROR(err_msg)                                                         \
    fprintf(stderr, "file: %s\nline %d, function:%s : %s\n", __FILE__,         \
            __LINE__, __FUNCTION__, err_msg);

#define DEBUG(dbg_fmt, ...)                                                         \
    fprintf(stdout, "file: %s\nline %d, function:%s\n", __FILE__,         \
            __LINE__, __FUNCTION__); \
    fprintf(stdout, dbg_fmt, ##__VA_ARGS__); \
    fprintf(stdout, "\n"); 
#else
#define ERROR(err_msg)
#define DEBUG(dbg_fmt, ...)
#endif

#endif
