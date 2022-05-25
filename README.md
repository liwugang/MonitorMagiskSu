# Monitor Magisk Su

## summary
This tool is to monitor the magisk su invoked, and logging the command and result.


## Usage


* Push the compiled fake-sh to /data/local/tmp of the target device

```bash
adb push fake-sh  /data/local/tmp/
```

* Modify the source of magisk su, Change the default shell from "/system/bin/sh" to "/data/local/tmp/fake-sh"

https://github.com/topjohnwu/Magisk/blob/master/native/jni/su/su.hpp#L9

```C++
--- a/native/jni/su/su.hpp
+++ b/native/jni/su/su.hpp
@@ -6,7 +6,7 @@
 
 #include <db.hpp>
 
-#define DEFAULT_SHELL "/system/bin/sh"
+#define DEFAULT_SHELL "/data/local/tmp/fake-sh"

```

* Get the result to use the keyword 'fake-sh' which to filter the log info

```bash

adb shell logcat | grep fake-sh

```

## End

I provided a modified Magisk-v24.3.apk by using IDA to patch magisk instead of compiling from source.



# Magisk su 监控工具

## 介绍

该工具用于监控 magisk su 的使用，输出执行 su 的命令和结果。

## 使用

* 将编译生成的 fake-sh，push 到手机的 /data/local/tmp/目录，即：

```bash
adb push fake-sh  /data/local/tmp/
```

* 修改 magisk su，将调用的 sh 默认地址从 /system/bin/sh 修改为 /data/local/tmp/fake-sh

https://github.com/topjohnwu/Magisk/blob/master/native/jni/su/su.hpp#L9

```C++
--- a/native/jni/su/su.hpp
+++ b/native/jni/su/su.hpp
@@ -6,7 +6,7 @@
 
 #include <db.hpp>
 
-#define DEFAULT_SHELL "/system/bin/sh"
+#define DEFAULT_SHELL "/data/local/tmp/fake-sh"

```

* 查看 log 日志，使用 fake-sh 关键字过滤

```bash

adb shell logcat | grep fake-sh

```


## 最后

我提供了修改后的 Magisk-v24.3.apk，修改的方式是使用 IDA 对 magisk 进行 patch，而不是从源码编译。

