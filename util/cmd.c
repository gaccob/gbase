#include <assert.h>
#include <signal.h>
#include <setjmp.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "core/os_def.h"
#include "cmd.h"

#define MAX_WORD_LEN 32
#define MAX_SENTENCE_LEN 1024
#define MAX_WORD_OPTION 32 

const char* _sep = " \n";

typedef struct word_t {
    char word[MAX_WORD_LEN];
    struct word_t* children[MAX_WORD_OPTION];
    cmd_handle_t handle;
} word_t;

struct cmd_t {
    char history[1024];
    char prompt[32];
    bool eof;
    bool closed;
    int max_history;
    word_t* word;
    char* complete[MAX_WORD_OPTION];
};

cmd_t* _cmd = NULL;

sigjmp_buf _ctl_c_buf;

static void
_signal(int sig) {
    rl_initialize();
    switch (sig) {
        case SIGINT:
            puts(""); // clear tty
            break;
        case SIGTTIN:
            puts("tty IN to background");
            if (_cmd) {
                _cmd->eof = true;
            }
            signal(SIGTTIN, SIG_IGN);
            break;
        case SIGTTOU:
            puts("tty OUT to background");
            if (_cmd) {
                _cmd->eof = true;
            }
            signal(SIGTTOU, SIG_IGN);
            break;
        case SIGTSTP:
        case SIGQUIT:
        case SIGTERM:
            printf("tty stop\n");
            if (_cmd) {
                _cmd->eof = true;
                _cmd->closed = true;
            }
            break;
        default:
            printf("catch signal %d\n", sig);
            break;
    }
    rl_cleanup_after_signal();
    // continue read
    siglongjmp(_ctl_c_buf, 1);
}

static word_t*
_word_create() {
    word_t* c = (word_t*) MALLOC(sizeof(*c));
    memset(c, 0, sizeof(*c));
    return c;
}

static void
_word_release(word_t* c) {
    if (c) {
        for (int i = 0; i < MAX_WORD_OPTION; ++ i) {
            if (c->children[i]) {
                _word_release(c->children[i]);
            }
        }
        FREE(c);
    }
}

static void
_word_add_complete(const char* word) {
    if (_cmd && word) {
        for (int i = 0; i < MAX_WORD_OPTION; ++ i) {
            if (!_cmd->complete[i]) {
                _cmd->complete[i] = (char*) MALLOC(MAX_WORD_LEN);
                snprintf(_cmd->complete[i], MAX_WORD_LEN, "%s", word);
                // printf("auto complete[%s]\n", word);
                return;
            }
        }
    }
}

static bool
_word_has_complete() {
    return _cmd && _cmd->complete[0];
}

static void
_word_add_complete_prefix(const char* prefix) {
    if (_cmd && prefix && _cmd->complete[0] && !_cmd->complete[1]) {
        char* concat = _cmd->complete[0];
        _cmd->complete[0] = (char*) MALLOC(MAX_SENTENCE_LEN);
        snprintf(_cmd->complete[0], MAX_SENTENCE_LEN, "%s %s", prefix, concat);
        FREE(concat);
    }
}

static void
_word_clear_complete() {
    if (_cmd) {
        for (int i = 0; i < MAX_WORD_OPTION; ++ i) {
            if (_cmd->complete[i]) {
                FREE(_cmd->complete[i]);
            } else {
                break;
            }
        }
    }
}

static char*
_auto_complete(const char* text, int state) {
    (void)(text);
    (void)(state);
    if (!_cmd || !_cmd->complete[0]) {
        return NULL;
    }
    char* ret = _cmd->complete[0];
    for (int i = 0; i < MAX_WORD_OPTION - 1; ++ i) {
        _cmd->complete[i] = _cmd->complete[i + 1];
        if (!_cmd->complete[i]) {
            break;
        }
    }
    _cmd->complete[MAX_WORD_OPTION - 1] = NULL;
    return ret;
}

static char**
_completed(const char* text, int start, int end) {
    if (!_cmd || !_cmd->word) {
        return rl_completion_matches(text, rl_filename_completion_function);
    }

    int line_size = end + 1;
    char line[line_size];
    snprintf(line, line_size, "%s", rl_line_buffer);
    // printf("\nline[%s] text[%s]\n", line, text);
    char* word = strtok(line, _sep);

    // find parent
    word_t* parent = _cmd->word;
    for (; word; word = strtok(NULL, _sep)) {
        bool found = false;
        for (int i = 0; i < MAX_WORD_OPTION; ++ i) {
            if (!parent->children[i]) {
                break;
            }
            if (0 == strncmp(word, parent->children[i]->word, MAX_WORD_LEN)) {
                parent = parent->children[i];
                found = true;
                break;
            }
        }
        if (!found) {
            break;
        }
    }

    // auto complete
    for (int i = 0; i < MAX_WORD_OPTION; ++ i) {
        if (parent->children[i]) {
            if (!word || 0 == strncmp(word, parent->children[i]->word, strlen(word))) {
                _word_add_complete(parent->children[i]->word);
            }
        } else {
            break;
        }
    }
    if (!word && text[0] != 0 && parent != _cmd->word) {
        _word_add_complete_prefix(text);
    }
    if (_word_has_complete()) {
        return rl_completion_matches(text, _auto_complete);
    }
    return rl_completion_matches("", rl_filename_completion_function);
}

cmd_t*
cmd_create(const char* history, const char* prompt) {
    // cmd singleton existed
    if (_cmd) {
        return NULL;
    }
    if (!history || !prompt) {
        return NULL;
    }
    cmd_t* cmd = (cmd_t*) MALLOC(sizeof(cmd_t));
    snprintf(cmd->history, sizeof(cmd->history), "%s", history);
    snprintf(cmd->prompt, sizeof(cmd->prompt), "%s", prompt);
    cmd->eof = false;
    cmd->closed = false;
    cmd->word = NULL;

    // reuse history file as readline name
    rl_readline_name = cmd->history;
    rl_attempted_completion_function = _completed;
    using_history();
    read_history(cmd->history);
    stifle_history(1024);

    // signal handler
    if (signal(SIGINT, _signal) == SIG_ERR
        || signal(SIGTSTP, _signal) == SIG_ERR
        || signal(SIGTERM, _signal) == SIG_ERR
        || signal(SIGQUIT, _signal) == SIG_ERR
        || signal(SIGTTIN, _signal) == SIG_ERR
        || signal(SIGTTOU, _signal) == SIG_ERR) {
        FREE(cmd);
        return NULL;
    }

    _cmd = cmd;
    return cmd;
}

void
cmd_release(cmd_t* cmd) {
    if (cmd) {
        write_history(cmd->history);
        if (cmd->word) {
            _word_release(cmd->word);
            _word_clear_complete();
        }
        if (_cmd == cmd) {
            _cmd = NULL;
        }
        FREE(cmd);
    }
}

bool
cmd_eof(cmd_t* cmd) {
    return cmd->eof;
}

bool
cmd_closed(cmd_t* cmd) {
    return cmd->closed;
}

void
cmd_set_closed(cmd_t* cmd) {
    cmd->closed = true;
}

char*
cmd_readline(cmd_t* cmd) {
    if (!cmd || cmd->eof) {
        return NULL;
    }
    char* rd = 0;
    while (sigsetjmp(_ctl_c_buf, 1) != 0);
    while ((rd = readline(cmd->prompt)) == NULL || *rd == '\0') {
        if (rd == NULL) {
            cmd->eof = true;
            return NULL;
        }
        if (*rd == '\0') {
            free(rd);
        }
    }
    add_history(rd);
    return rd;
}

void
cmd_register(cmd_t* cmd, const char* sentence, cmd_handle_t handle) {
    if (cmd && sentence) {
        word_t* host = NULL;
        if (!cmd->word) {
            cmd->word = _word_create();
            host = cmd->word;
        }
        char input[MAX_SENTENCE_LEN];
        snprintf(input, sizeof(input), "%s", sentence);
        char* word = strtok(input, _sep);
        word_t* c = cmd->word;
        for (; word; word = strtok(NULL, _sep)) {
            // find
            for (int i = 0; i < MAX_WORD_OPTION; ++ i) {
                if (c->children[i] == NULL) {
                    c->children[i] = _word_create();
                    host = c->children[i];
                    snprintf(c->children[i]->word, MAX_WORD_LEN, "%s", word);
                    c = c->children[i];
                    break;
                }
                else if (0 == strncmp(c->children[i]->word, word, MAX_WORD_LEN)) {
                    c = c->children[i];
                    host = c;
                    break;
                }
                else if (i == MAX_WORD_OPTION - 1) {
                    // exeed max option size
                    assert(0);
                }
            }
        }
        host->handle = handle;
    }
}

int
cmd_handle(cmd_t* cmd, const char* sentence) {
    if (cmd && sentence) {
        char input[MAX_SENTENCE_LEN];
        snprintf(input, sizeof(input), "%s", sentence);
        char* word = strtok(input, _sep);
        word_t* host = cmd->word;
        for (; word; word = strtok(NULL, _sep)) {
            bool found = false;
            for (int i = 0; host->children[i] && i < MAX_WORD_OPTION; ++ i) {
                if (0 == strncmp(host->children[i]->word, word, MAX_WORD_LEN)) {
                    found = true;
                    host = host->children[i];
                    break;
                } else if (i == MAX_WORD_OPTION - 1) {
                    break;
                }
            }
            if (!found) {
                break;
            }
        }
        if (host && host->handle) {
            return host->handle(word);
        }
    }
    return -1;
}

