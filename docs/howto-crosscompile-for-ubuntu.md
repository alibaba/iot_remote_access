# 概述
本文介绍在Ubuntu下如何编译远程终端代码。
# 环境准备
## Mac电脑
系统信息如下:
```plain
yuehu@iot:~/github/remote_access$ uname -a
Linux iot 4.15.0-36-generic #39~16.04.1-Ubuntu SMP Tue Sep 25 08:59:23 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux
```
## 下载设备侧代码
假设我们的代码目录位于:  `/home/yuehu/github/`
```plain
cd /home/yuehu/github/

git clone https://github.com/xiaowenhuyuehu/remote_access.git
```



![image.png | left | 720x994](https://cdn.nlark.com/lark/0/2018/png/16055/1543648691373-c588ece1-a2af-49dc-8185-61a7e51a6e39.png "")

# 编译
## 安装SCONS
```plain
brew install -y scons
```
## 编译
```plain
scons
```


![image.png | left | 747x371](https://cdn.nlark.com/lark/0/2018/png/16055/1543648742222-63b5275a-99e1-4f3b-8dc9-1384919a43f9.png "")

## 查看生成物


![image.png | left | 747x58](https://cdn.nlark.com/lark/0/2018/png/16055/1543648764228-487070fc-3649-4f83-ad81-4a303c83bad6.png "")


