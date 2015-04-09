#ifndef CMD_H_
#define CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct cmd_t cmd_t;

typedef int (*cmd_handle_t)(const char* param);

// must be singleton
// not thread safe

cmd_t* cmd_create(const char* history, const char* prompt);
void cmd_release(cmd_t* cmd);

bool cmd_eof(cmd_t* cmd);
bool cmd_closed(cmd_t* cmd);
void cmd_set_closed(cmd_t* cmd);

char* cmd_readline(cmd_t* cmd);
void cmd_register(cmd_t* cmd, const char* sentence, cmd_handle_t);
int cmd_handle(cmd_t* cmd, const char* sentence);

typedef void (*cmd_callback_t)(const char* commands, int result);
void cmd_traverse(cmd_t* cmd, const char* param, cmd_callback_t cb);

#ifdef __cplusplus
}
#endif

#endif
