## 代码移植指南

### 交叉编译第三方库

1. 在scripts目录下，创建一个新的support脚本，建议直接拷贝`support_armv7.sh`, 假设重命名为`support_myboard.sh`.

2. 根据实际情况，修改`support_myboard.sh`里面的以下几项: 

```
_ToolChainRootDirectory:        指的是ToolChain解压的根目录，一般情况下，ToolChain的根目录包含include/bin/lib等目录
_CrossPrefix:                   指的所采用的Gcc的前缀，一般在ToolChain的根目录bin下，有*-gcc，比如arm-linux-gnueabi-gcc等。
_Host:                          指的是交叉编译器所运行的OS信息，一般情况下，可以通过*-gcc -v获取其中的--host值。
_Target:                        指的是交叉编译器所运行的Target信息，一般情况下，可以通过*-gcc -v获取其中的--target值。
_TargetBit:                     值的是交叉编译的目标平台的bit数，一般为 32位。
_NewBoardName:                  新的平台的名称，最好跟脚本名称里包含的board名称一致，假如为myboard。
```

3. 在scripts目录下，运行新建的support脚本:

`./support_myboard.sh`

4. 执行完成后，返回到仓库所在目录，确认新建的board符合一下目录格式:
```
board/
├── alios
│   ├── include
│   │   ├── nopoll
│   │   └── openssl
│   └── lib
│       ├── engines
│       ├── libcrypto.a
│       ├── libcrypto.so -> libcrypto.so.1.0.0
│       ├── libcrypto.so.1.0.0
│       ├── libnopoll.a
│       ├── libnopoll.la
│       ├── libssl.a
│       ├── libssl.so -> libssl.so.1.0.0
│       ├── libssl.so.1.0.0
│       └── pkgconfig
├── armv7
│   ├── include
│   │   ├── nopoll
│   │   └── openssl
│   └── lib
│       ├── engines
│       ├── libcrypto.a
│       ├── libcrypto.so -> libcrypto.so.1.0.0
│       ├── libcrypto.so.1.0.0
│       ├── libnopoll.a
│       ├── libnopoll.la
│       ├── libssl.a
│       ├── libssl.so -> libssl.so.1.0.0
│       ├── libssl.so.1.0.0
│       └── pkgconfig
├── centos
│   ├── include
│   │   ├── nopoll
│   │   └── openssl
│   └── lib
│       ├── engines
│       ├── libcrypto.a
│       ├── libcrypto.so -> libcrypto.so.1.0.0
│       ├── libcrypto.so.1.0.0
│       ├── libnopoll.a
│       ├── libnopoll.la
│       ├── libssl.a
│       ├── libssl.so -> libssl.so.1.0.0
│       ├── libssl.so.1.0.0
│       └── pkgconfig
└── macos
    ├── include
    │   ├── nopoll
    │   └── openssl
    └── lib
        ├── engines
        ├── libcrypto.1.0.0.dylib
        ├── libcrypto.a
        ├── libcrypto.dylib -> libcrypto.1.0.0.dylib
        ├── libnopoll.a
        ├── libnopoll.la
        ├── libssl.1.0.0.dylib
        ├── libssl.a
        ├── libssl.dylib -> libssl.1.0.0.dylib
        └── pkgconfig
```

### 核心代码编译

1. 在顶层目录执行以下命令:
```shell
make board=armv7 CC={实际的toolchain目录}/bin/arm-linux-gnueabi-gcc STRIP={实际的toolchain目录}/bin/arm-linux-gnueabi-strip
```

如有问题，请参考[howto-crosscompile-for-arm.md](https://github.com/alibaba/iot_remote_access/tree/master/docs/howto-crosscompile-for-arm-linux-step-by-step.md)手动完成第三方库的的交叉编译.

