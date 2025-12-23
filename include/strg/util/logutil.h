#ifndef STRG_LOGUTIL_H
#define STRG_LOGUTIL_H

#include <stdarg.h>
#include <wlr/util/log.h>
#include <stdint.h>
#include <wayland-server-core.h>

#define STDERR_BUF_SIZE 4096

extern struct strg_server server; // acceptable here since the logger is basically a singleton or sum

void strg_wlr_log_callback(enum wlr_log_importance importance, const char *fmt, va_list args);
int strg_stderr_log_callback(int fd, uint32_t mask, void *data);

void strg_log_timestamped_fd(const char *level, const char *msg);

int strg_init_logging(const char *logfile);
void strg_init_stderr_ev_loop_wl(struct wl_event_loop *event_loop);
void strg_shutdown_logging(void);

#endif //STRG_LOGUTIL_H