#include "tasks.h"

#include <algorithm>
#include <dirent.h>
#include <sys/param.h>
#include <unistd.h>

#include "log.h"
#include "process.h"
#include "duration.h"

struct ProcessStat {
    // https://man7.org/linux/man-pages/man5/proc_pid_stat.5.html
    int pid;
    char comm[16];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned int flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    long cutime;
    long cstime;
    long priority;
    long nice;
    long num_threads;
    long itrealvalue;
    unsigned long long starttime;
    unsigned long vsize;
    long rss;
    unsigned long rsslim;
    unsigned long startcode;
    unsigned long endcode;
    unsigned long startstack;
    unsigned long kstkesp;
    unsigned long kstkeip;
    unsigned long signal;
    unsigned long blocked;
    unsigned long sigignore;
    unsigned long sigcatch;
    unsigned long wchan;
    unsigned long nswap;
    unsigned long cnswap;
    int exit_signal;
    int processor;
    unsigned int rt_priority;
    unsigned int policy;
    unsigned long long delayacct_blkio_ticks;
    unsigned long guest_time;
    long cguest_time;
    unsigned long start_data;
    unsigned long end_data;
    unsigned long start_brk;
    unsigned long arg_start;
    unsigned long arg_end;
    unsigned long env_start;
    unsigned long env_end;
    int exit_code;

    // https://man7.org/linux/man-pages/man5/proc_pid_statm.5.html
    unsigned long size;
    unsigned long resident;
    unsigned long shared;
    unsigned long text;
    unsigned long lib;
    unsigned long data;
    unsigned long dt;
};

static constexpr const char *stat_pattern =
        "%d "    // pid
        "(%15[^)]) " // comm
        "%c "    // state
        "%d "    // ppid
        "%d "    // pgrp
        "%d "    // session
        "%d "    // tty_nr
        "%d "    // tpgid
        "%u "    // flags
        "%lu "   // minflt
        "%lu "   // cminflt
        "%lu "   // majflt
        "%lu "   // cmajflt
        "%lu "   // utime
        "%lu "   // stime
        "%ld "   // cutime
        "%ld "   // cstime
        "%ld "   // priority
        "%ld "   // nice
        "%ld "   // num_threads
        "%ld "   // itrealvalue
        "%llu "  // starttime
        "%lu "   // vsize
        "%ld "   // rss
        "%lu "   // rsslim
        "%lu "   // startcode
        "%lu "   // endcode
        "%lu "   // startstack
        "%lu "   // kstkesp
        "%lu "   // kstkeip
        "%lu "   // signal
        "%lu "   // blocked
        "%lu "   // sigignore
        "%lu "   // sigcatch
        "%lu "   // wchan
        "%lu "   // nswap
        "%lu "   // cnswap
        "%d "    // exit_signal
        "%d "    // processor
        "%u "    // rt_priority
        "%u "    // policy
        "%llu "  // delayacct_blkio_ticks
        "%lu "   // guest_time
        "%ld "   // cguest_time
        "%lu "   // start_data
        "%lu "   // end_data
        "%lu "   // start_brk
        "%lu "   // arg_start
        "%lu "   // arg_end
        "%lu "   // env_start
        "%lu "   // env_end
        "%d"     // exit_code
;

static constexpr const char *statm_pattern = "%lu %lu %lu %lu %lu %lu %lu";

static long get_total_system_memory() {
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == nullptr) {
        return -1;
    }

    char line[256];
    long total_memory = 0;

    constexpr char mem_total[] = "MemTotal:";
    while (fgets(line, sizeof(line), file) != nullptr) {
        if (strncmp(line, mem_total, 9) == 0) {
            if (sscanf(line + 9, "%ld", &total_memory) == 1) {
                break;
            }
        }
    }

    fclose(file);
    return total_memory;
}

double get_uptime_from_clock() {
    struct timespec ts;
    if (clock_gettime(CLOCK_BOOTTIME, &ts) == -1) {
        return -1;
    }
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static std::unique_ptr<ProcessStat> get_task_info(pid_t pid, pid_t tid) {
    char path[33];
    snprintf(path, sizeof(path), "/proc/%d/task/%d/stat", pid, tid);
    FILE *file = fopen(path, "r");
    if (!file) return nullptr;

    char stat[512];
    if (fgets(stat, sizeof(stat), file) == nullptr) {
        fclose(file);
        return nullptr;
    }
    fclose(file);

    std::unique_ptr<ProcessStat> process = std::make_unique<ProcessStat>();
    int result = sscanf(stat, stat_pattern,
                        &process->pid,
                        process->comm,
                        &process->state,
                        &process->ppid,
                        &process->pgrp,
                        &process->session,
                        &process->tty_nr,
                        &process->tpgid,
                        &process->flags,
                        &process->minflt,
                        &process->cminflt,
                        &process->majflt,
                        &process->cmajflt,
                        &process->utime,
                        &process->stime,
                        &process->cutime,
                        &process->cstime,
                        &process->priority,
                        &process->nice,
                        &process->num_threads,
                        &process->itrealvalue,
                        &process->starttime,
                        &process->vsize,
                        &process->rss,
                        &process->rsslim,
                        &process->startcode,
                        &process->endcode,
                        &process->startstack,
                        &process->kstkesp,
                        &process->kstkeip,
                        &process->signal,
                        &process->blocked,
                        &process->sigignore,
                        &process->sigcatch,
                        &process->wchan,
                        &process->nswap,
                        &process->cnswap,
                        &process->exit_signal,
                        &process->processor,
                        &process->rt_priority,
                        &process->policy,
                        &process->delayacct_blkio_ticks,
                        &process->guest_time,
                        &process->cguest_time,
                        &process->start_data,
                        &process->end_data,
                        &process->start_brk,
                        &process->arg_start,
                        &process->arg_end,
                        &process->env_start,
                        &process->env_end,
                        &process->exit_code);

    if (result < 52) {
        return nullptr;
    }

    snprintf(path, sizeof(path), "/proc/%d/task/%d/statm", pid, tid);
    file = fopen(path, "r");
    if (!file) {
        return nullptr;
    }

    if (fgets(stat, sizeof(stat), file) == nullptr) {
        fclose(file);
        return nullptr;
    }
    fclose(file);

    result = sscanf(stat, statm_pattern,
                    &process->size,
                    &process->resident,
                    &process->shared,
                    &process->text,
                    &process->lib,
                    &process->data,
                    &process->dt);

    if (result < 7) {
        return nullptr;
    }

    return process;
}

void print_tasks(pid_t pid) {
    char task_dir[19];
    snprintf(task_dir, sizeof(task_dir), "/proc/%d/task", pid);

    DIR *dir = opendir(task_dir);
    if (!dir) {
        perror("Failed to open task directory");
        return;
    }

    std::vector<ProcessStat> tasks;

    dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        // We skip non-numeric directories (non-thread entries)
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            const pid_t tid = atoi(entry->d_name);
            const std::unique_ptr<ProcessStat> stat = get_task_info(pid, tid);
            if (stat) {
                tasks.emplace_back(*stat);
            }
        }
    }

    closedir(dir);

    const size_t total = tasks.size();
    const size_t running = std::count_if(tasks.begin(), tasks.end(), [](const ProcessStat &task) {
        return task.state == 'R';
    });
    const size_t sleeping = std::count_if(tasks.begin(), tasks.end(), [](const ProcessStat &task) {
        return task.state == 'S';
    });
    const size_t stopped = std::count_if(tasks.begin(), tasks.end(), [](const ProcessStat &task) {
        return task.state == 'T';
    });
    const size_t zombie = std::count_if(tasks.begin(), tasks.end(), [](const ProcessStat &task) {
        return task.state == 'Z';
    });
    LOG_FISHNET("Threads: %zu total, %zu running, %zu sleeping, %zu stopped, %zu zombie",
                total, running, sleeping, stopped, zombie);

    const uint64_t total_uptime = get_process_uptime(pid);
    const long total_memory = get_total_system_memory();

    /*
top - 19:59:27 up  4:29,  1 user,  load average: 0.00, 0.00, 0.00
Threads:   1 total,   0 running,   1 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
MiB Mem :   7808.3 total,   7160.9 free,    664.2 used,    214.2 buff/cache
MiB Swap:   2048.0 total,   2048.0 free,      0.0 used.   7144.1 avail Mem

    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
    174 message+  20   0    9532   5120   4488 S   0.0   0.1   0:00.20 dbus-daemon
     */

    //      PID   PR  NI        VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
    //    12345 -100 -20 12345678901 123456 123456 S 100.0 100.0 123:56:89 123456789012345
    LOG_FISHNET("      PID   PR  NI        VIRT    RES    SHR S  %%CPU  %%MEM     TIME+ COMMAND");

    const int page_size_kb = getpagesize() / 1024;
    const double hz = (double) HZ;
    const double sys_uptime = get_uptime_from_clock();

    for (const ProcessStat &task: tasks) {
        double cpu_usage = 0.0;
        if (total_uptime > 0) {
            const double total_time = (task.utime + task.stime + task.cutime + task.cstime) / hz;
            cpu_usage = (total_time / total_uptime) * 100;
        }
        double memory_usage = 0.0;
        if (total_memory > 0) {
            memory_usage = 100.0 * (task.rss * page_size_kb) / total_memory;
        }
        LOG_FISHNET("    %5d %4ld %3ld %11lu %6ld %6ld %c %5.1f %5.1f %9s %s",
                    task.pid, task.priority, task.nice, task.vsize, task.rss * page_size_kb,
                    task.shared * page_size_kb, task.state, cpu_usage, memory_usage,
                    seconds_to_human_readable_time(sys_uptime - task.starttime / hz).c_str(),
                    task.comm);
    }

    LOG_FISHNET("");
}
