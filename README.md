# 阿里云IoT远程运维套装之远程访问设备端源码

## 功能

* Link IoT Edge Lite版环境搭建指南，请点击[链接](https://github.com/alibaba/iot_remote_access/wiki/Link-IoT-Lite%E7%89%88%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA%E6%8C%87%E5%8D%97).
* 跨公网SSH到你的设备上，并提供基于浏览器方案的免安装web shell.
* 无需公网IP，直接浏览设备上的文件，并提供下载/上传功能.
* 内网穿透，支持跨公网访问Windows远程桌面.
* Android ADB不再局限于局域网调试.
* 免费，开源，稳定，安全的远程运维神器。
* 技术及产品架构介绍，请浏览 [WIKI](https://github.com/alibaba/iot_remote_access/wiki)
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

```
RemoteTerminalDaemon: 动态链接库版本的可执行程序，运行需要将nopoll和openssl库放到系统库里或者手动指定LD_LIBRARY_PATH.
RemoteTerminalDaemon_static: 静态库链接版本的可执行程序，可直接运行. 
```

## 运行
两种运行方式的差异在于设备三元组在配置文件里面传递还是通过命令行参数传递。
### 配置文件方式

* 配置文件说明

```shell
{
	"cloud_ip": "backend-iotx-remote-debug.aliyun.com",	//连接云端的ip或者url，无需修改。
	"cloud_port": "443",					//连接云端的端口，无需修改。
	"cert_path": "root.pem",				//TSL握手需要的根证书存储路径，无需修改。
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
	"product_key": "AAAAAAAAAAA",		            //阿里云IoT物联网平台上的ProductKey，具体请[参考](https://help.aliyun.com/document_detail/73729.html?spm=a2c4g.11174283.6.584.5fd91668DmxBzt)
	"device_name": "BBBBBBBBBBBBBBBBBBBB",	                  //阿里云IoT物联网平台上的DeviceName
	"device_secret": "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"        //阿里云IoT物联网平台上的DeviceSecret
}

```
* 修改配置文件里的product_key, device_name, device_secret三项.

* 运行

```shell
./RemoteTerminalDaemon_static
```

### 命令行方式 
```shell
./RemoteTerminalDaemon_static ProductKey DeviceName DeviceSecret

直接运行，后面跟上设备三要素即可。
```

**注意**: 必须把board/xxx/prebuilt下面所有的.so，顶层目录的root.pem，顶层目录的remote_terminal.json及RemoteTerminalDaemon同时安装在设备上
才能运行成功。

PS: 如有任何问题，请钉钉扫描咨询。
![asdf](https://camo.githubusercontent.com/bc61a578aa686d36c550ee657498786a0afdffdf/68747470733a2f2f63646e2e6e6c61726b2e636f6d2f6c61726b2f302f323031382f706e672f31363035352f313534333838383432313239332d36643638663830632d376261362d343363362d383737372d6331636365623035643834642e706e67)

