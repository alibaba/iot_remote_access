# 概述
本文介绍如何基于Ubuntu将远程终端移植到Arm v7架构上的裁剪Linux上。
# 环境准备
## 交叉工具链
* 解压交叉工具链
假设当前目录位于`/home/yuehu/project/toolchain/Demo`
```plain
tar zxvf CrossCompiler.tar.gz
```


![image.png | left | 827x648](https://cdn.nlark.com/lark/0/2018/png/16055/1543461768463-fc78624c-7e8d-4a09-8ff1-d5591354b92f.png "")

* 进入解压目录，确保编译器可用
```plain
# 进入目录
cd /home/yuehu/project/toolchain/Demo/ql-oe/sysroots/x86_64-linux/usr/bin/arm-oe-linux-gnueabi

# 查看编译器
./arm-oe-linux-gnueabi-gcc -v
```


![image.png | left | 827x326](https://cdn.nlark.com/lark/0/2018/png/16055/1543461874027-6580527b-5aa8-4706-ad19-b6a5685893b9.png "")

## 下载设备侧代码
假设我们的代码目录位于:  `/home/yuehu/github/`
```plain
cd /home/yuehu/github/

git clone https://github.com/alibaba/iot_remote_access.git
```


![image.png | left | 637x536](https://cdn.nlark.com/lark/0/2018/png/16055/1543461460332-e773cfaa-2e12-426e-8210-91fa5236d657.png "")

详细的代码目录介绍，请参考github。
# 交叉编译
## 新建board
假设我们的板子叫 Demo，我们执行以下命令来完成board的创建。
```plain
cd /home/yuehu/github/remote_access

mkdir -p board/demo/prebuilt/nopoll && mkdir -p board/demo/prebuilt/openssl
```
## 编译openssl
* 先在工具链中查找是否已经包含openssl的库
```plain
cd /home/yuehu/project/toolchain/Demo/ql-oe

find -name libssl.so
find -name libcrypto.so
find -name libcrypto.a
find -name libssl.a
```


![image.png | left | 827x920](https://cdn.nlark.com/lark/0/2018/png/16055/1543480744941-d355e74d-bd47-44b7-8a79-e88a51e755b0.png "")


如上图的结果，该工具链里已经包含了openssl库，我们就不需要再编译，只需将头文件和库拷贝到board目录即可。
* 拷贝头文件和库到board
```plain
拷贝库文件 
cp sysroots/mdm9607/usr/lib/libssl.* /home/yuehu/github/remote_access/board/demo/prebuilt/openssl/lib/
cp sysroots/mdm9607/lib/libcrypto.* /home/yuehu/github/remote_access/board/demo/prebuilt/openssl/lib/

拷贝头文件
cp -rf ./sysroots/mdm9607/usr/include/openssl/ /home/yuehu/github/remote_access/board/demo/prebuilt/openssl/include
```
## 编译nopoll
* 先在工具链中查找是否已经包含nopoll的库
```plain
find -name libnopoll*
```
找不到这个库，于是我们需要手动交叉编译。
* 解压软件包
```plain
cd /home/yuehu/github/remote_access/dependencies

tar zxvf nopoll-0.4.6.tar.gz
```

* 配置
```plain
./configure --prefix=/home/yuehu/github/remote_access/board/demo/prebuilt/nopoll CFLAGS=-L/home/yuehu/github/remote_access/board/demo/prebuilt/openssl/include LDFLAGS=-L/home/yuehu/github/remote_access/board/demo/prebuilt/openssl/lib --host=arm-oe-linux-gnueabi --build=arm-oe-linux-gnueabi

```
* 修改libtool
```plain
vim libtool +318

把 318 行的CC替换成: /home/yuehu/project/toolchain/Demo/ql-oe/sysroots/x86_64-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-gcc
```


![image.png | left | 827x170](https://cdn.nlark.com/lark/0/2018/png/16055/1543485168354-63c7a1bb-2638-4e65-b2f8-c4d5572281cf.png "")

* 修改 nopoll\_config.h
```plain
vim ./src/nopoll_config.h +63

注释掉对 NOPOLL_64BIT_PLATFORM 的定义。
```



![image.png | left | 827x228](https://cdn.nlark.com/lark/0/2018/png/16055/1543485246670-c0d3e532-cf29-4651-a8e0-af150288507f.png "")

* 编译
```plain
make CFLAGS="--sysroot=/home/yuehu/project/toolchain/Demo/ql-oe/sysroots/mdm9607/" CC=/home/yuehu/project/toolchain/Demo/ql-oe/sysroots/x86_64-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-gcc
```

* 安装
```plain
make install
```


![image.png | left | 827x219](https://cdn.nlark.com/lark/0/2018/png/16055/1543485315746-31215f1a-9003-4d52-a877-7346fd1b29d5.png "")

* 手动拷贝头文件
```plain
cd /home/yuehu/github/remote_access/board/demo/prebuilt/nopoll/include

此时可以看到所有头文件都在nopoll这个目录了
mv nopoll/* ./ && rm -rf nopoll
```

至此，所有的第三方库的依赖已经编译完成，下面开始编译端侧代码。
## 编译端侧代码
### 使用SCONS
* 安装scons
```plain
sudo apt-get install -y scons
```
* 编译
```plain
scons STRIP=/home/yuehu/project/toolchain/Demo/ql-oe/sysroots/x86_64-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-strip LINKFLAGS="--sysroot=/home/yuehu/project/toolchain/Demo/ql-oe/sysroots/mdm9607/"  board=demo CFLAGS="--sysroot=/home/yuehu/project/toolchain/Demo/ql-oe/sysroots/mdm9607/" CC=/home/yuehu/project/toolchain/Demo/ql-oe/sysroots/x86_64-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-gcc
```
* 查看生成物


![image.png | left | 747x74](https://cdn.nlark.com/lark/0/2018/png/16055/1543492499841-f36e6863-ed66-4565-af4e-389a44bbc8d1.png "")


### 使用Makefile
* 编译
```plain
make board=demo CFLAGS="--sysroot=/home/yuehu/project/toolchain/Demo/ql-oe/sysroots/mdm9607/" CC=/home/yuehu/project/toolchain/Demo/ql-oe/sysroots/x86_64-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-gcc
```
* 查看生成物


![image.png | left | 747x37](https://cdn.nlark.com/lark/0/2018/png/16055/1543491535118-3a8a3cbe-8384-4553-a9d3-f3fd55ba8742.png "")

