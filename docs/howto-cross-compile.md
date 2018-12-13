## 代码移植指南

### 创建board及其子目录

参考board/demo/创建子目录。目录必须符合以下格式:

![image.png](https://cdn-pri.nlark.com/lark/0/2018/png/16055/1544666677758-b674111e-3a70-4b69-9a1c-fe126fcac771.png)

### 第三方库编译

#### 编译 OpenSSL

* 首先确认toolchain里是否已经提供OpenSSL库，如果已提供则无需再交叉编译此库。
* 参考[OpenSSL Compilation and Installation](https://wiki.openssl.org/index.php/Compilation_and_Installation#ARM)完成OpenSSL的交叉编译.
* **注意** 编译完成后需要拷贝到新创建的board目录.

#### 编译 nopoll

* nopoll 目前只能支持OpenSSL库，请务必先编译OpenSSL并拷贝到新创建的board目录.
* 参考[howto-crosscompile-for-arm.md](https://github.com/alibaba/iot_remote_access/tree/master/docs/howto-crosscompile-for-arm.md)完成nopoll的交叉编译.

### 核心代码编译

在顶层目录执行以下命令:
```shell
make board=XXXX CFLAGS="" CC=""
注意，XXX需要换成真实的新创建的board子目录的名称
CFLAGS用来指定sysroot及toolchain其他头文件的路径
CC指定当前所使用的gcc的绝对路径.
```

如有问题，请参考[howto-crosscompile-for-arm.md](https://github.com/alibaba/iot_remote_access/tree/master/docs/howto-crosscompile-for-arm.md)完成nopoll的交叉编译.

