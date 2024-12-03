#ifndef FISHNET_DUMP_H
#define FISHNET_DUMP_H

#include "debugger_thread_info.h"

void fishnet_dump_native(const DebuggerThreadInfo *info);

void fishnet_dump_java(const char *java_stack_traces);

void fishnet_dump_anr(const char *java_stack_traces, const DebuggerThreadInfo *info);

#endif //FISHNET_DUMP_H
