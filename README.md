# CODM ESP Box
Source project for the current CODM Garena libunity build.

Before GitHub Actions build, add:
- jni/include/zygisk.hpp
- jni/include/dobby.h
- jni/libs/arm64-v8a/libdobby.a

Use the same Zygisk header and Dobby static library from your already-working Zygisk/Dobby project.

Runtime log:
`logcat -s CODM-ESP`

Offsets are version-specific.
