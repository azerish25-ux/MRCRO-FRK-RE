# Android UI message transport

`UiMessageQueue.initialize()` runs on the main thread before native scene setup.
The native `post_write` / `post_message` APIs own typed envelopes and schedule
them through the real Android main `Handler`. A native worker obtains its own
JNI environment and detaches only when the transport attached it. The cached
queue class is a process-lifetime global reference, with no Activity/View held.
See [Android JNI tips](https://developer.android.com/ndk/guides/jni-tips) and the
[Android JNI interface](https://android.googlesource.com/platform/libnativehelper/+/master/include_jni/jni.h).

Pending messages use monotonically assigned IDs rather than addresses. Delivery
removes the ID before callbacks, allowing nested posts and preventing replay.
Cancellation releases owned payloads outside the registry lock. Unknown or
cancelled runnable IDs are ignored. Expired scene/backend owners follow the
recovered weak-listener delivery behavior.

An application includes the `ui` Gradle library and adds `native/CMakeLists.txt`
to its native build. That CMake target builds `mfrk_ui_messages`; the Java module
does not build a duplicate copy. Consumer rules retain the JNI queue entrypoints.
The renderer comparison host now packages and initializes this actual transport.

Debug-only instrumentation covers worker FIFO ordering, expired owner/listener,
cancellation, non-reentrant nested posts and a full 24-byte heap payload on the
main thread. These are transport checks, not an installed synth scene, device
MIDI test or sound/pixel parity claim. Android build/runtime verification occurs
only on GitHub Actions. Actual controller factories, handlers, scene and audio
application integration remain unfinished.
