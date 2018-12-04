# 阿里云IoT远程运维套装之远程访问设备端源码


## 编译

以下在ubuntu上演示如何编译。
### 下载Scons

```shell
sudo apt-get install -y scons
```

### 编译

```shell
scons
```

### 生成物介绍

```
RemoteTerminalDaemon: 动态链接库版本的可执行程序，运行需要将nopoll和openssl库放到系统库里或者手动指定LD_LIBRARY_PATH.
RemoteTerminalDaemon_static: 静态库链接版本的可执行程序，可直接运行. 
```

## 运行

### 配置文件方式

* 配置文件说明

```shell
{
	"cloud_ip": "backend-iotx-remote-debug.aliyun.com",       //连接云端的ip或者url，无需修改。
	"cloud_port": "443",              //连接云端的端口，无需修改。
	"cert_path": "root.pem",            //TSL握手需要的根证书存储路径，无需修改。
	"is_tls_on": 1,                     //是否支持TLS，默认为1，无需修改。
	"is_debug_on": 0,                   //是否打开调试模式，可以填0或者1，调试模式下，有更丰富的打印信息.
	"listen_port": 22,                  //监听本地的端口号，一般为需要代理的服务端口号，比如SSH为22，telnet为21，ftp为23等。
	"listen_ip": "127.0.0.1",           //监听代理服务的IP号，一般为localhost的IP地址.
	"product_key": "aaaaaaaaaaa",       //阿里云IoT物联网平台上的ProductKey，具体请参考: https://help.aliyun.com/document_detail/73729.html?spm=a2c4g.11174283.6.584.5fd91668DmxBzt 
	"device_name": "bbbbbbbbbbbbb",     //阿里云IoT物联网平台上的DeviceName
	"device_secret": "cccccccccccccccccccccccccccccccc"   //阿里云IoT物联网平台上的DeviceSecret
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

