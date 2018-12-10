# 概述
本文介绍在Macos下如何编译远程终端代码。
# 环境准备
## Mac电脑
系统信息如下:


![image.png | left | 827x500](https://cdn.nlark.com/lark/0/2018/png/16055/1543493720861-370a4674-3642-4793-bd39-f2d1c7dc137d.png "")

## 下载设备侧代码
假设我们的代码目录位于:  `/home/yuehu/github/`
```plain
cd /home/yuehu/github/

git clone https://github.com/alibaba/iot_remote_access.git
```



![image.png | left | 827x708](https://cdn.nlark.com/lark/0/2018/png/16055/1543493818343-cc895cb1-86ee-4567-9928-843c0ea608e1.png "")

# 编译
## 安装SCONS
```plain
brew install -y scons
```
## 编译
```plain
scons board=macos

```


![image.png | left | 827x471](https://cdn.nlark.com/lark/0/2018/png/16055/1543493913894-6ef9ecac-fe8c-4207-ac5d-de8feea33061.png "")

## 查看生成物


![image.png | left | 827x107](https://cdn.nlark.com/lark/0/2018/png/16055/1543493962015-aba2fb5d-e354-44c7-af2b-f36ef713f831.png "")


