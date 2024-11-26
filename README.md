# Fishnet

Fishnet is an Android library that dumps Java and native crashes to a file,
whose format is almost the same as the one generated by Android `libdebuggerd`,
and it is powerful than `libdebuggerd`.

## Compatibility

- Architectures: arm64-v8a, armeabi-v7a, x86, x86_64
- Android: 6.0 and above

### Abort message detection

- [ ] [FORTIFY](https://android-developers.googleblog.com/2017/04/fortify-in-android.html)
- [ ] [fdsan](https://android.googlesource.com/platform/bionic/+/master/docs/fdsan.md) (Android 10+)
- [x] [Scudo error](https://source.android.com/docs/security/test/scudo) (Android 11+)
- [ ] [GWP-ASan](https://developer.android.com/ndk/guides/gwp-asan) (Android 14 +)
- [ ] [MTE](https://developer.android.com/ndk/guides/arm-mte) (Android 14 QPR3 +)

## Example logs

### Java crash

TBD.

### Native crash

```
* * * * * * * * * * * * * * * *
Build fingerprint: 'google/coral/coral:13/TP1A.221005.002.B2/9382335:user/release-keys'
Revision: 'MP1.0'
ABI: 'arm64'
Timestamp: 2024-11-19 18:13:36.306697361+0800
Process uptime: 5s
Page size: 16384 bytes
Cmdline: com.kyant.fishnet.demo
pid: 22575, tid: 22575, name: nt.fishnet.demo  >>> com.kyant.fishnet.demo <<<
uid: 10565
tagged_addr_ctrl: 0000000000000001 (PR_TAGGED_ADDR_ENABLE)
signal 6 (SIGABRT), code -1 (SI_QUEUE), fault addr 0x000029450000582f
Abort message: 'FORTIFY: memcpy: prevented 10-byte write into 8-byte buffer'

registers:
    x0  0000000000000000  x1  000000000000582f  x2  0000000000000006  x3  0000007fce91d580
    x4  0000008080808080  x5  0000008080808080  x6  0000008080808080  x7  8080808080808080
    x8  00000000000000f0  x9  0000007300645a00  x10 0000000000000001  x11 0000007300683ce4
    x12 0101010101010101  x13 0000075b4423a35b  x14 000c77d0edd643a2  x15 0000000000000020
    x16 00000073006e8d60  x17 00000073006c5b70  x18 00000073200ca000  x19 000000000000582f
    x20 000000000000582f  x21 00000000ffffffff  x22 0000000000000002  x23 0000007fce91d900
    x24 000000730d409840  x25 0000000000000002  x26 000000730d4098c0  x27 0000000000000002
    x28 0000007fce91d7d0  x29 0000007fce91d600
    lr  0000007300675868  sp  0000007fce91d560  pc  0000007300675894  pst 0000000000000000

63 total frames
backtrace:
      #00 pc 0000000000051894  /apex/com.android.runtime/lib64/bionic/libc.so (abort+164) (BuildId: 058e3ec96fa600fb840a6a6956c6b64e)
             0000000000051894: ldr  q0, [x9]
             0000000000051898: ldr  q1, [x9, #0x10]
             000000000005189c: mov  x1, sp
             00000000000518a0: mov  w0, #6
             00000000000518a4: mov  x2, xzr
             00000000000518a8: stp  q0, q1, [sp]
             00000000000518ac: bl  0xbf850
             00000000000518b0: add  x1, x29, #0x18
             ...
      #01 pc 000000000005353c  /apex/com.android.runtime/lib64/bionic/libc.so (__fortify_fatal(char const*, ...)+124) (BuildId: 058e3ec96fa600fb840a6a6956c6b64e)
      #02 pc 000000000007b540  /apex/com.android.runtime/lib64/bionic/libc.so (__check_buffer_access(char const*, char const*, unsigned long, unsigned long)+40) (BuildId: 058e3ec96fa600fb840a6a6956c6b64e)
      #03 pc 000000000007b6b4  /apex/com.android.runtime/lib64/bionic/libc.so (__memcpy_chk_fail+68) (BuildId: 058e3ec96fa600fb840a6a6956c6b64e)
      #04 pc 000000000007c490  /apex/com.android.runtime/lib64/bionic/libc.so (__memcpy_chk+16) (BuildId: 058e3ec96fa600fb840a6a6956c6b64e)
      #05 pc 000000000002cb8c  /data/app/~~QjObwZNEQ7CnsLBrqrlwWQ==/com.kyant.fishnet.demo-JEwVFt5y6o5F1BXdcx9B1g==/lib/arm64/libcom.kyant.fishnet.demo.so (Java_com_kyant_fishnet_demo_CrashFragment_nativeCrash+60)
      #06 pc 0000000000384370  /apex/com.android.art/lib64/libart.so (art_quick_generic_jni_trampoline+144) (BuildId: 3f7d5a016e08d528f129bdd336d81168)
      #07 pc 000000000036db74  /apex/com.android.art/lib64/libart.so (art_quick_invoke_stub+612) (BuildId: 3f7d5a016e08d528f129bdd336d81168)
      #08 pc 00000000003671e4  /apex/com.android.art/lib64/libart.so (bool art::interpreter::DoCall<false>(art::ArtMethod*, art::Thread*, art::ShadowFrame&, art::Instruction const*, unsigned short, bool, art::JValue*)+1928) (BuildId: 3f7d5a016e08d528f129bdd336d81168)
      ...

...

memory near x9 (/apex/com.android.runtime/lib64/bionic/libc.so):
    00000073006459e0 646165725f797472 6361626c6c61635f  rty_read_callbac
    00000073006459f0 72206f742029286b 0000000000646165  k() to read.....
    0000007300645a00 0000000010000000 0000000000000000  ................
    0000007300645a10 0000000000000000 0000000000000000  ................
    0000007300645a20 0000000000000000 0000000000000001  ................
    0000007300645a30 0000000000000000 0000000000000000  ................
    0000007300645a40 100d000c01414d4c 2825221f1c191613  LMA.........."%(
    0000007300645a50 00000010000000f8 0000008400000078  ........x.......
    0000007300645a60 000000c000000090 0000009c000000cc  ................
    0000007300645a70 000000a8000000d8 000000b4000000e4  ................
    0000007300645a80 00000010000000c4 0000005000000044  ........D...P...
    0000007300645a90 0000008c0000005c 0000006800000098  \...........h...
    0000007300645aa0 00000074000000a4 00000080000000b0  ....t...........
    0000007300645ab0 00000010000002b0 0000023c00000230  ........0...<...
    0000007300645ac0 0000025400000248 0000026c00000260  H...T...`...l...
    0000007300645ad0 0000028400000278 0000029c00000290  x...............

...

memory map (2220 entries): (fault address prefixed with --->)
    00000000'14000000-00000000'33ffffff rw-         0  20000000  [anon:dalvik-main space (region space)]
    00000000'6f220000-00000000'705bbfff rw-         0   139c000  [anon:dalvik-/data/misc/apexdata/com.android.art/dalvik-cache/boot.art]
    00000000'705bc000-00000000'708bbfff r--         0    300000  /data/misc/apexdata/com.android.art/dalvik-cache/arm64/boot.oat
    00000000'708bc000-00000000'713bafff r-x    300000    aff000  /data/misc/apexdata/com.android.art/dalvik-cache/arm64/boot.oat
    00000000'713bb000-00000000'713bbfff ---         0      1000  [anon:dalvik-Boot image reservation]
    00000000'713bc000-00000000'713bcfff rw-         0      1000  [anon:.bss]
    00000000'713bd000-00000000'713bffff ---         0      3000  [anon:dalvik-Boot image reservation]
    00000000'713c0000-00000000'7147ffff rw-         0     c0000  /data/misc/apexdata/com.android.art/dalvik-cache/arm64/boot.vdex
    ...
    0000007f'ce123000-0000007f'ce123fff ---         0      1000
    0000007f'ce124000-0000007f'ce922fff rw-         0    7ff000  [stack]
--->Fault address falls at 00002945'00005bb4 after any mapped regions

Threads: 20 total, 1 running, 19 sleeping, 0 stopped, 0 zombie
Total uptime: 5
Total memory: 11718828
    PID     PR  NI     VIRT      RES    SHR   S  %CPU  %MEM   TIME+
    15270   10  -10 14875426816  38043  23802 S   5.2   1.3  8573695
    15278    0  -20 14875426816  38043  23802 S   0.2   1.3  8573701
    15279    0  -20 14875426816  38043  23802 S   0.2   1.3  8573701
    15280    0  -20 14875426816  38043  23802 S   0.2   1.3  8573701
    15283   29    9 14875426816  38043  23802 S  16.5   1.3  8573702
    15284   24    4 14875426816  38043  23802 S   0.2   1.3  8573702
    ...

--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
Cmdline: com.kyant.fishnet.demo
pid: 11377, tid: 11389, name: Metrics Backgro  >>> com.kyant.fishnet.demo <<<
uid: 10572
tagged_addr_ctrl: 0000000000000001 (PR_TAGGED_ADDR_ENABLE)

registers:
    x0  b40000789aa94480  x1  0000000000000080  x2  0000000000000002  x3  0000000000000000
    x4  0000000000000000  x5  0000000000000000  x6  0000000000000000  x7  00000000000000ff
    x8  0000000000000062  x9  76cc194a13e40754  x10 431bde82d7b634db  x11 76cc194a13e40754
    x12 ffffffffc4653600  x13 00000284bbc064ec  x14 ffffffff98befc10  x15 000a27b98539e703
    x16 00000077b9c22bb0  x17 0000007a64813e40  x18 00000077a33ba000  x19 b40000789aa94470
    x20 b40000792aa9df50  x21 0000000000000002  x22 00000077a7559000  x23 00000077a7558ba0
    x24 00000077a7559000  x25 0000000000000001  x26 00000077a7558ff8  x27 00000000000fc000
    x28 00000000000fe000  x29 00000077a7558ae0
    lr  00000077b942a224  sp  00000077a7558ad0  pc  0000007a64813e5c  pst 0000000060001000

7 total frames
backtrace:
      #00 pc 000000000004de5c  /apex/com.android.runtime/lib64/bionic/libc.so (syscall+28) (BuildId: 058e3ec96fa600fb840a6a6956c6b64e)
             000000000004de5c: svc  #0
             000000000004de60: cmn  x0, #1, lsl #12
             000000000004de64: cneg  x0, x0, hi
             000000000004de68: b.hi  0xa1b24
             000000000004de6c: ret
             ...
      ...
...

open files:
    fd 0: /dev/null (unowned)
    fd 1: /dev/null (unowned)
    fd 2: /dev/null (unowned)
    fd 3: socket:[2757246] (unowned)
    fd 4: /sys/kernel/tracing/trace_marker (unowned)
    fd 5: /proc/22575/cmdline (owned by FILE* 0x71adc6ed18)
    fd 6: pipe:[2757248] (unowned)
    fd 7: pipe:[2757248] (unowned)
    fd 8: /apex/com.android.art/javalib/core-oj.jar (owned by RandomAccessFile 0xbd0fb7)
    fd 9: /apex/com.android.art/javalib/core-libart.jar (owned by RandomAccessFile 0x9d3a324)
    fd 10: /apex/com.android.art/javalib/okhttp.jar (owned by RandomAccessFile 0xca99e8d)
    ...

--------- log main
...
11-19 18:12:56.920 22575 22591 W AdrenoUtils: <ReadGpuID:221>: Failed to read chip ID from gpu_model. Fallback to use the GSL path
11-19 18:12:56.964 22575 22591 W Parcel  : Expecting binder but got null!
11-19 18:13:29.466 22575 22582 I nt.fishnet.demo: Background concurrent copying GC freed 2774KB AllocSpace bytes, 7(164KB) LOS objects, 89% free, 2907KB/26MB, paused 279us,140us total 111.835ms
11-19 18:13:31.176 22575 22575 I Choreographer: Skipped 900 frames!  The application may be doing too much work on its main thread.
11-19 18:13:31.209 22575 22588 I OpenGLRenderer: Davey! duration=10022ms; Flags=0, FrameTimelineVsyncId=16439050, IntendedVsync=133005930754777, Vsync=133015930754677, InputEventId=0, HandleInputStart=133015933859342, AnimationStart=133015933884290, PerformTraversalsStart=133015935786426, DrawStart=133015936235280, FrameDeadline=133005962379799, FrameInterval=133015932864551, FrameStartTime=11125022, SyncQueued=133015938177312, SyncStart=133015938399603, IssueDrawCommandsStart=133015939532312, SwapBuffers=133015945022573, FrameCompleted=133015953151740, DequeueBufferDuration=89583, QueueBufferDuration=924583, GpuCompleted=133015953151740, SwapBuffersCompleted=133015947115958, DisplayPresentTime=133011647126572, CommandSubmissionCompleted=133015945022573, 
11-19 18:13:36.210 22575 22575 F libc    : FORTIFY: memcpy: prevented 10-byte write into 8-byte buffer

```

### ANR

TBD.

## Build

### Clone the repository

```shell
git clone https://github.com/Kyant0/Fishnet.git
cd Fishnet
git submodule init
git submodule update

git apply --directory disasm/src/main/cpp/external/capstone disasm_external.patch
git apply fishnet_external.patch
```

### Build the library

You should build twice, the first time is expected to fail.

### Run the demo app

When you modified the c++ code in `fishnet` module, run the demo app twice to see the changes.

## Submodule maintain info

| Repository           | Link                                                                                                                              | Update time                |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------|----------------------------|
| capstone             | [capstone-engine/capstone](https://github.com/capstone-engine/capstone/commit/e46838ed974948b4446c7c478508409e0000aeaa)           | -                          |
| art                  | [art](https://android.googlesource.com/platform/art/+/54cd7bf45839805d7f0f2708a6c9a65c70ca154a)                                   | 2024 Nov 25 21:04:05       |
|                      | [libartbase](https://android.googlesource.com/platform/art/+log/refs/heads/main/libartbase)                                       | 2024 Nov 22 16:37:17       |
|                      | [libartpalette](https://android.googlesource.com/platform/art/+log/refs/heads/main/libartpalette)                                 | 2024 Aug 29 12:15:41       |
|                      | [libdexfile](https://android.googlesource.com/platform/art/+log/refs/heads/main/libdexfile)                                       | 2024 Nov 18 10:53:22       |
| system/core          | [system/core](https://android.googlesource.com/platform/system/core/+/f7b584086c464f04f764677df56c8a2644109523)                   | 2024 Nov 25 22:25:24       |
| system/libbase       | [system/libbase](https://android.googlesource.com/platform/system/libbase/+/72bf9612eacb940beaefbabc116c9f5b293d5ad1)             | 2024 Nov 07 20:40:11       |
| system/libprocinfo   | [system/libprocinfo](https://android.googlesource.com/platform/system/libprocinfo/+/b6b71fd97d7015a59175936a9b3824623470a1a7)     | 2024 Aug 14 22:08:40       |
| system/libziparchive | [system/libziparchive](https://android.googlesource.com/platform/system/libziparchive/+/63f00d1ade29315dc0adac08fceae9984305205d) | 2024 Jun 04 23:06:47       |
| system/logging       | [system/logging](https://android.googlesource.com/platform/system/logging/+/b34720941c1a18249615ebed5a25d05ff12bfad1)             | 2024 Sep 16 16:54:16       |
|                      | [liblog](https://android.googlesource.com/platform/system/logging/+log/refs/heads/main/liblog)                                    | 2024 Sep 11 14:28:59       |
| system/unwinding     | [system/unwinding](https://android.googlesource.com/platform/system/unwinding/+/c7b83a52e8dbe29735e74f0abee1fe565a7ced5b)         | 2024 Sep 12 14:09:02 -0700 |
|                      | [libunwindstack](https://android.googlesource.com/platform/system/unwinding/+log/refs/heads/main/libunwindstack)                  | 2024 Sep 12 14:09:02 -0700 |
| external/lzma        | [external/lzma](https://android.googlesource.com/platform/external/lzma/+/1c87526f4839d9276d0dfdcaceea8a6c8dcd100d)               | 2024 Sep 09 13:20:46       |
| external/zstd        | [external/zstd](https://android.googlesource.com/platform/external/zstd/+/d06a29da1edcc46177e774010cc2ef6158366b52)               | 2024 Jul 11 02:24:39       |
