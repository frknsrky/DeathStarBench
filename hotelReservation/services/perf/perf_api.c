#define _GNU_SOURCE
#include "perf_api.h"
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sched.h>

//_Thread_local static int initialized = 0;
//_Thread_local static int leader_fd = -1;
//_Thread_local static int instructions_fd = -1;
//_Thread_local static int l1_misses_fd = -1;
_Thread_local static char result_buffer[256];
_Thread_local static char error_buffer[256];

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

struct read_format {
    uint64_t nr;            // number of events
    struct {
        uint64_t value;     // counter value
        uint64_t id;        // event ID
    } values[3];
};


struct perf_handles perf_start() {
    struct perf_handles handles = { .leader_fd = -1, .instructions_fd = -1, .l1_misses_fd = -1 };
    int initialized = 0;
    if (!initialized) {
        struct perf_event_attr pe = {0};

        int cpu = sched_getcpu();

        // Leader: cycles
        pe.type = PERF_TYPE_HARDWARE;
        pe.size = sizeof(struct perf_event_attr);
        pe.config = PERF_COUNT_HW_CPU_CYCLES;
        pe.disabled = 1;
        pe.exclude_kernel = 1;
        pe.exclude_hv = 1;

        handles.leader_fd = perf_event_open(&pe, 0, cpu, -1, PERF_FLAG_FD_CLOEXEC);
        if (handles.leader_fd == -1) {
            perror("perf_event_open (cycles)");
            return handles;
        }

        // Instructions
        pe.disabled = 0;
        pe.config = PERF_COUNT_HW_INSTRUCTIONS;
        handles.instructions_fd = perf_event_open(&pe, 0, cpu, handles.leader_fd, PERF_FLAG_FD_CLOEXEC);
        if (handles.instructions_fd == -1) {
            perror("perf_event_open (instructions)");
            close(handles.leader_fd);
            return handles;
        }

        // L1 cache misses
        pe.type = PERF_TYPE_HW_CACHE;
        pe.config = PERF_COUNT_HW_CACHE_L1D |
                    (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                    (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
        handles.l1_misses_fd = perf_event_open(&pe, 0, cpu, handles.leader_fd, PERF_FLAG_FD_CLOEXEC);
        if (handles.l1_misses_fd == -1) {
            perror("perf_event_open (l1_misses)");
            close(handles.leader_fd);
            close(handles.instructions_fd);
            return handles;
        }

        initialized = 1;
    }

    // Reset and enable only if already initialized
    ioctl(handles.leader_fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(handles.leader_fd, PERF_EVENT_IOC_ENABLE, 0);
    return handles;
}

const char* perf_stop(int leader_fd, int instructions_fd, int l1_misses_fd) {
    int initialized = 1;
    if (!initialized) return "not_initialized";

    ioctl(leader_fd, PERF_EVENT_IOC_DISABLE, 0);

    //ioctl(leader_fd, PERF_EVENT_IOC_REFRESH, 0);
    
    long long cycles = -1, instructions = -1, l1_misses = -1;
    int bytes_read = read(leader_fd, &cycles, sizeof(long long));
    read(instructions_fd, &instructions, sizeof(long long));
    read(l1_misses_fd, &l1_misses, sizeof(long long));

    if (bytes_read == -1) {
        snprintf(error_buffer, sizeof(error_buffer), "read failed: %s", strerror(errno));
        return error_buffer;
    }
    
    //struct read_format rf={0};
    //int bytes_read = read(leader_fd, &rf, sizeof(rf));

    //if (bytes_read == -1) {
    //  snprintf(error_buffer, sizeof(error_buffer), "read failed: %s", strerror(errno));
    //  return error_buffer;
    //}
    //if (bytes_read < sizeof(rf)) {
    //  snprintf(error_buffer, sizeof(error_buffer), "read too small: %d bytes", bytes_read);
    //  return error_buffer;
    //}

    //if (rf.nr < 3) {
    //    snprintf(error_buffer, sizeof(error_buffer), "not all events read: %d events", rf.nr);
    //    return error_buffer;
    //}

    //long long cycles = rf.values[0].value;
    //long long instructions = rf.values[1].value;
    //long long l1_misses = rf.values[2].value;

    snprintf(result_buffer, sizeof(result_buffer),
             "cycles=%lld, instructions=%lld, l1_misses=%lld",
             cycles, instructions, l1_misses);

    close(leader_fd);
    close(instructions_fd);
    close(l1_misses_fd);

    leader_fd = instructions_fd = l1_misses_fd = -1;

    initialized = 0;

    return result_buffer;
}
