#ifndef STRG_SIGUTIL_H
#define STRG_SIGUTIL_H

// this exists simply because i wanted to use a GNU extension without using GNU extensions (the function is sigabbrev_np)

struct sig_name {
    int sig;
    const char *name;
};

const char *sig_to_string(int sig);

#endif //STRG_SIGUTIL_H