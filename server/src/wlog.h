#ifndef WLOG
#define WLOG

#include <stdio.h>
inline void print_error(const char *file_path, const int line_num,
                        const char *function_name, const char *error) {
    fprintf(stderr, "file: %s\nline %d, function:%s : %s\n", file_path,
            line_num, function_name, error);
}

#ifdef DEBUG
#define ERROR(err_msg)                                                         \
    fprintf(stderr, "file: %s\nline %d, function:%s : %s\n", __FILE__,         \
            __LINE__, __FUNC__, err_msg);

#define DEBUG(dbg_msg)                                                         \
    fprintf(stdout, "file: %s\nline %d, function:%s : %s\n", __FILE__,         \
            __LINE__, __FUNC__, dbg_msg);
#else
#define ERROR(err_msg)
#define DEBUG(dbg_msg)
#endif

#endif
