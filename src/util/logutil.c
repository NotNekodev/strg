#include <strg/util/logutil.h>

#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "strg/strg.h"

static char stderr_buf[STDERR_BUF_SIZE];
static size_t stderr_buf_len = 0;

static int stderr_pipe[2];
static int stderr_orig_fd;

void strg_wlr_log_callback(enum wlr_log_importance importance, const char *fmt, va_list args) {
    if (server.logfile_fd == -1) return;

    char buf[512];
    vsnprintf(buf, sizeof(buf), fmt, args);
    const char *level = (importance == WLR_ERROR) ? "ERROR" :
                        (importance == WLR_INFO) ? "INFO" :
                        (importance == WLR_DEBUG) ? "DEBUG" : "SILENT";
    strg_log_timestamped_fd(level, buf);
}

int strg_stderr_log_callback(int fd, uint32_t mask, void *data) {
    (void)data;
    if (!(mask & WL_EVENT_READABLE)) return 0;

    char read_buf[512];
    ssize_t n = read(fd, read_buf, sizeof(read_buf));
    if (n <= 0) return 0;

    for (ssize_t i = 0; i < n; ++i) {
        char c = read_buf[i];
        if (stderr_buf_len < STDERR_BUF_SIZE - 1) {
            stderr_buf[stderr_buf_len++] = c;
        }

        if (c == '\n') {
            stderr_buf[stderr_buf_len - 1] = '\0';
            strg_log_timestamped_fd("STDERR", stderr_buf);
            stderr_buf_len = 0;
        }
    }

    if (stderr_buf_len == STDERR_BUF_SIZE - 1) {
        stderr_buf[stderr_buf_len] = '\0';
        strg_log_timestamped_fd("STDERR", stderr_buf);
        stderr_buf_len = 0;
    }

    return 0;
}

void strg_log_timestamped_fd(const char *level, const char *msg) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    time_t sec = now.tv_sec - server.start.tv_sec;
    long nsec = now.tv_nsec - server.start.tv_nsec;
    if (nsec < 0) {
        sec -= 1;
        nsec += 1000000000L;
    }

    dprintf(server.logfile_fd, "[%02ld:%02ld:%02ld.%03ld] [%s] %s\n",
            sec / 3600, (sec % 3600) / 60, sec % 60, nsec / 1000000,
            level, msg);
}

int strg_init_logging(const char *logfile) {
    server.logfile_fd = open(logfile, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (server.logfile_fd == -1) {
        perror("Failed to open log file");
        return 1;
    }

    wlr_log_init(WLR_DEBUG, strg_wlr_log_callback);

    pipe(stderr_pipe);
    stderr_orig_fd = dup(STDERR_FILENO);
    dup2(stderr_pipe[1], STDERR_FILENO);
    close(stderr_pipe[1]);
    fcntl(stderr_pipe[0], F_SETFL, O_NONBLOCK);

    return 0;
}

void strg_init_stderr_ev_loop_wl(struct wl_event_loop *event_loop) {
	wl_event_loop_add_fd(event_loop, stderr_pipe[0], WL_EVENT_READABLE, strg_stderr_log_callback, NULL);
}

void strg_shutdown_logging(void) {
    if (stderr_buf_len > 0) {
        stderr_buf[stderr_buf_len] = '\0';
        strg_log_timestamped_fd("STDERR", stderr_buf);
    }

    close(server.logfile_fd);
    close(stderr_pipe[0]);
    dup2(stderr_orig_fd, STDERR_FILENO);
    close(stderr_orig_fd);
}
