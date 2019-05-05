# 概述

本文介绍设备端接收物联网平台下发指令完成按需开启/关闭运维通道的协议内容。

设备端默认集成两个通道: mqtt 通道和 websocket 通道，前者即为数据通道，用来接收云端的下行控制命令完成与云端的信息交互，后者即为运维通道，用来与云端建立一个透传的
隧道，完成诸如SSH/Telnet/RDP/SFTP/FTP等网络服务报文的转发。

所以，本文介绍的协议实际是数据通道上的协议，用来接收云端控制指令。数据通道详细介绍，请参考: [阿里云物联网平台MQTT协议规范](https://help.aliyun.com/document_detail/30540.html) 

# 协议内容

云端通过mqtt数据通道下发开启/关闭指令，关于mqtt通道的topic规范，请参考: [阿里云物联网平台基本概念-topic](https://help.aliyun.com/document_detail/73731.html)
## Topic 
运维通道的开启/关闭由同一个topic来实现, 根据payload的不同完成不同的功能，topic格式如下:

`/sys/${YourProductKey}/${YourDeviceName}/edge/debug/switch`

${YourProductKey}表示产品的标识符ProductKey；${YourDeviceName}表示设备名称。比如: `/sys/a1wJ2pERo8x/yuehu_demo/edge/debug/switch`

## 开启运维通道

payload的内容如下:

```
{"status":1}
```
设备收到此topic消息后，需要开启运维通道，即运行RemoteTerminalDaemon进程。

## 关闭运维通道

payload的内容如下:

```
{"status":0}
```

设备收到此topic消息后，需要关闭运维通道，即停止RemoteTerminalDaemon进程。

# 参考实现

为便于开发者使用，我们基于[c版本的物联网SDK](https://help.aliyun.com/document_detail/97568.html)实现了开启/关闭运维功能的Sample Code.

解析下行控制指令源码如下:
```c
#define MSG_START_REMOTE_SERVICE "{\"status\":1}"
#define MSG_STOP_REMOTE_SERVICE  "{\"status\":0}"

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt     ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("----");
            EXAMPLE_TRACE("PacketId: %d", ptopic_info->packet_id);
            EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                          ptopic_info->topic_len,
                          ptopic_info->ptopic,
                          ptopic_info->topic_len);
            EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                          ptopic_info->payload_len,
                          ptopic_info->payload,
                          ptopic_info->payload_len);
            EXAMPLE_TRACE("----");
			if(strncmp(ptopic_info->payload, MSG_START_REMOTE_SERVICE, ptopic_info->payload_len) == 0) {
				EXAMPLE_TRACE("starting remote access daemon...\n");
				open_remote_access_daemon(g_pk, g_dn, g_ds);
			} else if(strncmp(ptopic_info->payload, MSG_STOP_REMOTE_SERVICE, ptopic_info->payload_len) == 0) {
				EXAMPLE_TRACE("stopping remote access daemon...\n");
				close_remote_access_daemon();
			} else {
				EXAMPLE_TRACE("topic ignored...\n");
			} 
            break;
        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

```

完整源码请参考: [开启/关闭运维通道Sample Code](https://github.com/xiaowenhuyuehu/iotkit-embedded/blob/master/examples/mqtt/mqtt_example.c)

