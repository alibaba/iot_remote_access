## 编译openssl库

* 解压源码包
```
tar zxvf openssl-1.0.2l.tar.gz
```

* 进入目录进行配置并编译
```
cd openssl-1.0.2l/ && ./config shared && make

如果是在Mac下编译
./Configure darwin64-x86_64-cc

```
* 拷贝静态库
```
ls -alh *.a
```

## 编译nopoll库

* 解压
```
tar zxvf nopoll-0.4.6.tar.gz
```

* 配置
```
cd nopoll/ && ./autogen.sh --prefix=$(OUTPUT_DIR) CFLAGS=-L${OUTPUT_DIR}/include LDFLAGS=-L${OUTPUT_DIR}/lib && GCC_VERSION=$(shell $(CC) -dumpversion) make CFLAGS="-DNOPOLL_HAVE_TLSv11_ENABLED -DNOPOLL_HAVE_TLSv12_ENABLED -DNOPOLL_HAVE_TLSv10_ENABLED" 
```
