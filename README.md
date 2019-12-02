# 阿里云IoT远程运维套装之远程访问设备端源码

## 功能

* Link IoT Edge Lite版环境搭建指南，请点击[链接](https://github.com/alibaba/iot_remote_access/wiki/Link-IoT-Lite%E7%89%88%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA%E6%8C%87%E5%8D%97).
* 跨公网SSH到你的设备上，并提供基于浏览器方案的免安装web shell.
* 无需公网IP，直接浏览设备上的文件，并提供下载/上传功能.
* 内网穿透，支持跨公网访问Windows远程桌面.
* Android ADB不再局限于局域网调试.
* 免费，开源，稳定，安全的远程运维神器。
* 技术及产品架构介绍，请浏览 [WIKI](https://github.com/alibaba/iot_remote_access/wiki)
* 支持从云端控制台上手动开启/关闭运维通道，控制协议详见: [云端控制协议](https://github.com/alibaba/iot_remote_access/blob/master/docs/protocol-cloud.md)
## 编译

### 编译CentOS版本

默认已经支持64bit的CentOS/Ubuntu。

`make board=centos`

### 编译MacOS版本

默认已经支持 OSX 10.11.6:

`make board=macos`

### 编译Arm V7 32位软浮点版本

注意，需要下载[ToolChain](https://releases.linaro.org/components/toolchain/binaries/latest-7/arm-linux-gnueabi/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi.tar.xz) 并解压到合适目录。

`make board=armv7 CC=/home/yuehu/toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc STRIP=/home/yuehu/toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/arm-linux-gnueabi-strip`

### 编译其他平台

1. 请参考scripts/support_armv7.sh在scripts下面创建一个新的support_xxx.sh，指定以下几个参数:

```
_ToolChainRootDirectory:        指的是ToolChain解压的根目录，一般情况下，ToolChain的根目录包含include/bin/lib等目录
_CrossPrefix:                   指的所采用的Gcc的前缀，一般在ToolChain的根目录bin下，有*-gcc，比如arm-linux-gnueabi-gcc等。
_Host:                          指的是交叉编译器所运行的OS信息，一般情况下，可以通过*-gcc -v获取其中的--host值。
_Target:                        指的是交叉编译器所运行的Target信息，一般情况下，可以通过*-gcc -v获取其中的--target值。
_TargetBit:                     值的是交叉编译的目标平台的bit数，一般为 32位。
_NewBoardName:                  新的平台的名称，比如armv8等。

```
2. 在scripts下面运行这个新的脚本:

`./support_xxx.sh`

3. 编译

`make board=XXXX CC=XXXXX/bin/arm-linux-gnueabi-gcc STRIP=XXXX/bin/arm-linux-gnueabi-strip`

### 生成物介绍

编译完成后,在当前目录下生成"build"文件夹, 其中包含可执行文件和依赖库等.其中:

```
RemoteTerminalDaemon_dynamic: 动态链接库版本的可执行程序，运行需要将nopoll和openssl库放到系统库里或者手动指定LD_LIBRARY_PATH.
RemoteTerminalDaemon_static:  静态库链接版本的可执行程序，可直接运行. 
start_for_dynamic.sh:         启动RemoteTerminalDaemon_dynamic 的shell脚本(依赖环境已经配置好)
remote_terminal.json:         配置文件,下面有具体的说明
```
### 配置文件方式

* 配置文件说明

```shell
{
	"cloud_ip": "backend-iotx-remote-debug.aliyun.com",	//连接云端的ip或者url，无需修改。
	"cloud_port": "443",					//连接云端的端口，无需修改。
	"is_tls_on": 1,						//是否支持TLS，默认为1，无需修改。
	"is_debug_on": 0,					//是否打开调试模式，可以填0或者1，调试模式下，有更丰富的打印信息.
	"services": [{
			"type": "FTP", 				//远程服务的类型，支持FTP/SSH/SFTP/RDP/ADB/HTTP等常用协议
			"name": "ftp_localhost",		//用户可以为该服务自定义一个别名，支持最多32个字符，支持utf-8编码的中文
			"ip": "127.0.0.1",			//服务的IP地址，如果需要远程访问本地的ip的地址，则填入127.0.0.1，如果是需要远程访问局域网中其他设备的服务，则填其他设备的ip地址，比如192.168.1.138
			"port": 21				//服务的实际端口，比如SSH服务，默认为22，比如HTTP端口默认为80等。
		}, {
			"type": "SFTP",
			"name": "sftp_localhost",
			"ip": "127.0.0.1",
			"port": 22
		}, {
			"type": "SSH",
			"name": "ssh_localhost",
			"ip": "127.0.0.1",
			"port": 22
		},
		{
			"type": "TELNET",
			"name": "telnet_local",
			"ip": "127.0.0.1",
			"port": 23
		}, {
			"type": "HTTP",
			"name": "http_localhost",
			"ip": "127.0.0.1",
			"port": 80
		}, {
			"type": "RDP",
			"name": "rdp_localhost",
			"ip": "127.0.0.1",
			"port": 3389
		}, {
			"type": "HTTP",
			"name": "openapi",
			"ip": "100.69.166.91",
			"port": 26999
		}

	],
    "product_key":   "此处填写pk值",    //阿里云IoT物联网平台上的ProductKey，具体请[参考](https://help.aliyun.com/document_detail/73729.html?spm=a2c4g.11174283.6.584.5fd91668DmxBzt)
    "device_name":   "此处填写dn值",   //阿里云IoT物联网平台上的DeviceName
    "device_secret": "此处填写ds值"    //阿里云IoT物联网平台上的DeviceSecret
}

```

### 运行


* 静态链接的可执行文件的运行

```shell
cd ./build/bin
./RemoteTerminalDaemon_static <product_key> <device_name> <device_secret> //若在配置文件中已经填写了product_key,device_name,device_secret 则此处无需重复填写
```


* 动态链接的可执行文件的运行

```shell
cd ./build/bin
./start_for_dynamic.sh <product_key> <device_name> <device_secret> //若在配置文件中已经填写了product_key,device_name,device_secret 则此处无需重复填写
```

PS: 如有任何问题，请钉钉扫描咨询。

![asdf](https://cdn.nlark.com/yuque/0/2019/png/209889/1557195802207-24b3bc61-de22-45ab-ae91-afecd400f0eb.png)

