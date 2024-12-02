# Fishnet

Fishnet is an Android library that dumps Java and native crashes,
format of the log file is almost the same as the one generated by Android.

## Compatibility

- Architectures: arm64-v8a, armeabi-v7a, x86, x86_64, risc-v64
- Android: 5.0 and above
- Log types: Native, Java, ANR

### Additional abort message detection

- [ ] [FORTIFY](https://android-developers.googleblog.com/2017/04/fortify-in-android.html)
- [ ] [fdsan](https://android.googlesource.com/platform/bionic/+/master/docs/fdsan.md) (Android 10+)
- [x] [Scudo error](https://source.android.com/docs/security/test/scudo) (Android 11+)
- [ ] [GWP-ASan](https://developer.android.com/ndk/guides/gwp-asan) (Android 14 +)
- [ ] [MTE](https://developer.android.com/ndk/guides/arm-mte) (Android 14 QPR3 +)

## Usage

1. Download the [aar](https://github.com/Kyant0/Fishnet/releases) and include it in your project.

2. In your `Application` class, add the following code,

```kotlin
import com.kyant.fishnet.Fishnet

class App : Application() {
    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)
        val logPath = File(filesDir, "logs").apply { mkdirs() }.absolutePath
        Fishnet.init(this, logPath)
    }
}
```

3. Run application and make a testing crash,
   the log file will be generated in the path you specified.

## Demo

See the `app` module,
the pre-built APK can be found in the [GitHub releases](https://github.com/Kyant0/Fishnet/releases).

## Example logs

TBD

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

## Submodule maintain info

| Repository           | Link                                                                                                                              | Update time                |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------|----------------------------|
| capstone             | [capstone-engine/capstone](https://github.com/capstone-engine/capstone/commit/e46838ed974948b4446c7c478508409e0000aeaa)           | -                          |
| art                  | [art](https://android.googlesource.com/platform/art/+/54cd7bf45839805d7f0f2708a6c9a65c70ca154a)                                   | 2024 Nov 25 21:04:05       |
|                      | [libartbase](https://android.googlesource.com/platform/art/+log/refs/heads/main/libartbase)                                       | 2024 Nov 22 16:37:17       |
|                      | [libartpalette](https://android.googlesource.com/platform/art/+log/refs/heads/main/libartpalette)                                 | 2024 Aug 29 12:15:41       |
|                      | [libdexfile](https://android.googlesource.com/platform/art/+log/refs/heads/main/libdexfile)                                       | 2024 Nov 18 10:53:22       |
| system/libbase       | [system/libbase](https://android.googlesource.com/platform/system/libbase/+/72bf9612eacb940beaefbabc116c9f5b293d5ad1)             | 2024 Nov 07 20:40:11       |
| system/libprocinfo   | [system/libprocinfo](https://android.googlesource.com/platform/system/libprocinfo/+/b6b71fd97d7015a59175936a9b3824623470a1a7)     | 2024 Aug 14 22:08:40       |
| system/libziparchive | [system/libziparchive](https://android.googlesource.com/platform/system/libziparchive/+/63f00d1ade29315dc0adac08fceae9984305205d) | 2024 Jun 04 23:06:47       |
| system/logging       | [system/logging](https://android.googlesource.com/platform/system/logging/+/b34720941c1a18249615ebed5a25d05ff12bfad1)             | 2024 Sep 16 16:54:16       |
|                      | [liblog](https://android.googlesource.com/platform/system/logging/+log/refs/heads/main/liblog)                                    | 2024 Sep 11 14:28:59       |
| system/unwinding     | [system/unwinding](https://android.googlesource.com/platform/system/unwinding/+/c7b83a52e8dbe29735e74f0abee1fe565a7ced5b)         | 2024 Sep 12 14:09:02 -0700 |
|                      | [libunwindstack](https://android.googlesource.com/platform/system/unwinding/+log/refs/heads/main/libunwindstack)                  | 2024 Sep 12 14:09:02 -0700 |
| external/lzma        | [external/lzma](https://android.googlesource.com/platform/external/lzma/+/1c87526f4839d9276d0dfdcaceea8a6c8dcd100d)               | 2024 Sep 09 13:20:46       |
| external/zstd        | [external/zstd](https://android.googlesource.com/platform/external/zstd/+/d06a29da1edcc46177e774010cc2ef6158366b52)               | 2024 Jul 11 02:24:39       |
