# <a name="htukaw"></a>1 通讯协议
以下介绍的协议均通过websocket通道传递，包括前端与云端的通讯协议，边缘端与云端的通讯协议。
## <a name="khynbp"></a>1.1 报文格式
每条报文均由消息头和负载组成，两者之间通过2组\r\n\r\n分割，具体格式如下:

```
| 消息头 |\r\n\r\n| 负载 | 消息头 |\r\n\r\n| 负载 | 消息头 |\r\n\r\n| 负载 |
```

注意，所有报文均以__<span data-type="color" style="color:#CF1322">二进制方式</span>__传输。不同的消息类型，消息头均为JSON格式，负载格式略有不同，具体分以下两种:

* 非服务原始报文
```plain
{
    "code": int,            //响应错误码，0表示无错误，其他值参考错误码表。
    "message": string,      //仅在code非0时有效，用来描述详细的错误信息。
}
\r\n\r\n                    //分隔符，若二进制不存在，可以不填。若填，则计入负载长度中。
二进制                        //二进制内容，用来返回对端需要响应的内容，可以不填。若填，则计入负载长度中。
```

* 服务原始报文
```plain
二进制                    //若为服务的原始报文，则直接透传二进制，无具体的格式规范。
```

## <a name="kg9grp"></a>1.2 消息头(Header)
所有消息均需要携带合法的消息头以便于接收方正确解析，消息头部的格式是固定的JSON串，具体内容如下:

### <a name="iixrrg"></a>协议字段

<div class="bi-table">
  <table>
    <colgroup>
      <col width="111px" />
      <col width="70px" />
      <col width="59px" />
      <col width="506px" />
    </colgroup>
    <tbody>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">参数</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">类型</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">必选</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">描述</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">msg_type</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">int</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">消息类型，具体取值参考表格 1。</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">service_type</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">int</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p"></div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">待访问的服务类型，具体取值参考表格 2。</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">payload_len</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">int</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">负载内容的长度，单位是字节。注: 不包含消息头和负载之间的\r\n\r\n。</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">msg_id</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">必须保证当前Session内全局唯一。<span data-type="color" style="color:#CF1322"><strong>请求方发送的消息id不为空的话，响应方也要将此id原封不动的回复给请求方，最长64位。</strong></span></div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">timestamp</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">long</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">Unix timestamp, ms作为单位。从格林威治时间1970年01月01日00时00分00秒起至现在的总秒数。</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">token</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p"></div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">1、边缘侧和云端之间新增session时由边缘端生成返回。在后续session通讯时携带。最长64位。</div>
          <div data-type="p">2、前端在和云端连接成功后，需要携带云端返回的Session。</div>
        </td>
      </tr>
    </tbody>
  </table>
</div>

#### <a name="r8egpl"></a>表格1:
目前支持的消息类型:

| 名称 | 值 | 含义 |
| --- | --- | --- |
| RESERVED | 1 | 保留 |
| MSG\_SERVICE\_PROVIDER\_CONN\_REQ | 2 | 服务提供者(即边缘设备)向云端发起(握手)连接请求 |
| MSG\_SERVICE\_CONSUMER\_CONN\_REQ | 3 | 服务消费者(即前端)向云端发起(握手)连接请求 |
| MSG\_SERVICE\_PROVIDER\_RAW\_PROTOCOL | 4 | 服务提供者(即边缘设备)向云端发送的原始服务协议 |
| MSG\_SERVICE\_CONSUMER\_RAW\_PROTOCOL | 5 | 服务消费者(即前端)向云端发送的原始服务协议 |
| RESERVED | 6 | 保留 |
| RESERVED | 7 | 保留 |
| MSG\_SERVICE\_CONSUMER\_NEW\_SESSION | 8 | 云端向边缘端请求新增一条会话 |
| MSG\_SERVICE\_CONSUMER\_RELEASE\_SESSION | 9 | 云端向边缘端请求新增一条会话 |
| RESERVED | 10 | RESERVED |
| MSG\_WEB\_LOGIN | 20 | 前端向云端发送登录请求 |
| MSG\_RESP | 255 | 响应报文 |

#### <a name="29i9dn"></a>表格2:
目前支持的服务类型:

| 名称 | 值 | 含义 |
| :--- | :--- | :--- |
| SERVICE\_TYPE\_SSH | 1 | SSH 服务,端口默认 22，__<span data-type="color" style="color:#CF1322">注: 当前仅支持SSH服务。</span>__ |
| SERVICE\_TYPE\_HTTP | 2 | HTTP 服务，端口默认 80 |
| SERVICE\_TYPE\_REMOTE\_DESKTOP | 3 | windows远程桌面服务，端口默认 3389 |
| SERVICE\_TYPE\_FTP | 4 | FTP服务， 默认端口为21 |
| SERVICE\_TYPE\_LINKEDGE\_EWEB | 5 | link iot edge 的 eweb和openApi 服务， 默认端口 9999 |
| SERVICE\_TYPE\_TELNET | 6 | telnet 服务， 默认端口 23 |
| SERVICE\_TYPE\_NONE | 255 | 无服务 |

### <a name="rtz2yg"></a>举例
```plain
{
	"msg_type": 255,
	"payload_len": 115,
	"msg_id": "f470f123-8ab1-405f-95cf-8748e8e4f2ed",
	"timestamp": 1541064891,
	"token": "Heie1uRRQyx3ICvBcwkXzYBYkTLBVoWqs3thlaZ1wkV4MetOBBz0n1NyKmZtAKH"
}
```
## <a name="99u6ae"></a>1.3 负载(Payload)
### <a name="oughxm"></a>1.3.1 设备上线请求
由边缘端远程终端服务向云端服务发起连接请求。消息头里的msg\_type为 2.

* 参数列表

<div class="bi-table">
  <table>
    <colgroup>
      <col width="150px" />
      <col width="76px" />
      <col width="56px" />
      <col width="466px" />
    </colgroup>
    <tbody>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">参数</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">类型</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">必选</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">描述</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">uuid</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p"></div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">设备的唯一标示，厂商自定义提供，可以为设备别名等</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">product_key</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">阿里云物联网平台上的网关ProductKey.</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">device_name</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">阿里云物联网平台上的网关DeviceName.</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">version</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">20181030之前的版本为 1.0，当前为2.0</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">IP</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p"></div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">设备的联网IP，非必须</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">MAC</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p"></div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">设备的MAC地址，非必须</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">service_supported</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">边缘设备目前支持的服务列表，参考表格1，多个服务则以逗号分割</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">signmethod</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">列出支持的签名算法，目前仅支持hmacsha256。</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">sign</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">签名字符串格式为:</div>
          <div data-type="p">clientId{uuid}deviceName{deviceName}productKey{productKey}timestamp{timestamp}</div>
          <div data-type="p">其中{}替换为真实的值，待签名的key为网关的DeivceSecret详见下面的说明。</div>
        </td>
      </tr>
    </tbody>
  </table>
</div>

* 签名算法举例

假设网关三元组为:

```plain
	"product_key": "a1NCDGc4Lkw",
	"device_name": "chenlu_gateway",
	"device_secret": "6X8bMz4GeeqTrx4cX66DYts1udwkxWhU"

```

则待签名的串为:

```plain
clientIdalibaba_iotdeviceNamechenlu_gatewayproductKeya1NCDGc4Lkwtimestamp1541070000
```

签名结果为:

```plain
08f8dbe0b21a238a0c0bb1f2ff92a351ec07a72e635d966e5d5e82613076cd79
```

完整的消息头为:
```plain
{
	"uuid": "alibaba_iot",
	"product_key": "a1NCDGc4Lkw",
	"device_name": "chenlu_gateway",
	"version": "version_",
	"IP": "",
	"MAC": "MAC_",
	"token": "",
	"service_supported": "ssh",
	"signmethod": "hmacsha256",
	"sign": "08f8dbe0b21a238a0c0bb1f2ff92a351ec07a72e635d966e5d5e82613076cd79"
}
```

* 云端响应格式：

| 参数 | 类型 | 必选 | 描述 |
| --- | --- | --- | --- |
| code | int | √ | 0 表示成功，其他表示失败，具体错误码请参考错误码表 |
| data | ojbect |  | 为null |
| message | string |  | 如果失败，则返回失败的详细错误原因，成功则返回null |

### <a name="mq6dxc"></a>1.3.2  前端连接请求
由前端向云端发起连接，表明需要调试具体哪台设备。消息头里的msg\_type为 3.

* 参数列表

<div class="bi-table">
  <table>
    <colgroup>
      <col width="141px" />
      <col width="72px" />
      <col width="63px" />
      <col width="472px" />
    </colgroup>
    <tbody>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">参数</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">类型</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">必选</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">描述</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">uuid</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p"></div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">设备的唯一标示，厂商自定义提供，可以为设备别名等。</div>
        </td>
      </tr>
      <tr height="34px">
        <td rowspan="1" colSpan="1">
          <div data-type="p">service_port</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">int</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p"></div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">设备提供服务的端口号，比如SSH服务，端口则为22，非必填。</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">product_key</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">阿里云物联网平台上的网关ProductKey.</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">device_name</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">阿里云物联网平台上的网关DeviceName.</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">signmethod</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">列出支持的签名算法，目前仅支持hmacsha256。</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">sign</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">签名字符串格式为: clientId{uuid}deviceName{deviceName}productKey{productKey}timestamp{timestamp} 其中{}替换为真实的值，待签名的key为网关的DeivceSecret详见6.3.1的说明。
          </div>
        </td>
      </tr>
    </tbody>
  </table>
</div>

* 响应

| 参数 | 类型 | 必选 | 描述 |
| :--- | :--- | :--- | :--- |
| code | int | √ | 0 表示成功，其他表示失败，具体错误码请参考错误码表 |
| data | ojbect | ​ | 为null |
| message | string | ​ | 如果失败，则返回失败的详细错误原因，成功则返回null |


### <a name="x7k9xb"></a>1.3.3 边缘消息透传
远程终端服务向云端透传原始数据。消息头里的msg\_type为 4.
负载为原始的二进制，无固定格式。
消息头里必须填入云端发送的token，msg\_id亦需保持一致。

### <a name="0gt5il"></a>1.3.4 前端消息透传
前端向云端透传原始数据。消息头里的msg\_type为 5.
负载为原始的二进制，无固定格式。
消息头里必须填入云端发送的token，msg\_id亦需保持一致。

### <a name="9z15cb"></a>1.3.5 新建会话
新页面开启后，由云端向远程终端服务发起创建新Session。<strong><span data-type="color" style="color:#CF1322">注意，此时响应的消息头里会有token字段，以后 所有通讯均需要携带。</span></strong><strong><span data-type="color" style="color:#262626">此</span></strong>消息头里的msg\_type为 8.

* 请求列表：

| 参数 | 类型 | 必选 | 描述 |
| :--- | :--- | :--- | :--- |
| service\_port | int |  | 服务提供方的端口 |


* 响应：

<div class="bi-table">
  <table>
    <colgroup>
      <col width="111px" />
      <col width="105px" />
      <col width="127px" />
      <col width="405px" />
    </colgroup>
    <tbody>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">参数</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">类型</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">必选</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">描述</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">code</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">int</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">0 表示成功，其他表示失败，具体错误码请参考错误码表</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">data</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">ojbect</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">​</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">{</div>
          <div data-type="p">&quot;session_id&quot;: string</div>
          <div data-type="p">}</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">message</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">​</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">如果失败，则返回失败的详细错误原因，成功则返回null</div>
        </td>
      </tr>
    </tbody>
  </table>
</div>

### <a name="okllwv"></a>1.3.6 释放会话
页面关闭后，由云向远程终端服务发起释放<span data-type="color" style="color:#262626">Session。</span>消息头里的msg\_type为 9.
* 请求参数列表：无
* 响应： 

<div class="bi-table">
  <table>
    <colgroup>
      <col width="145px" />
      <col width="128px" />
      <col width="98px" />
      <col width="455px" />
    </colgroup>
    <tbody>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">参数</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">类型</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">必选</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">描述</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">code</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">int</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">0 表示成功，其他表示失败，具体错误码请参考错误码表</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">data</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">ojbect</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">​</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">{</div>
          <div data-type="p">&quot;session_id&quot;: string</div>
          <div data-type="p">}</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">message</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">​</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">如果失败，则返回失败的详细错误原因，成功则返回null</div>
        </td>
      </tr>
    </tbody>
  </table>
</div>


### <a name="rdttnx"></a>1.3.7 前端登录请求
前端在发送 6.3.2得到正确响应后，在消息头里携带token，发送此消息以完成登录到远程的设备上。消息头里的msg\_type为 20.
* 请求列表

| 参数 | 类型 | 必选 | 描述 |
| :--- | :--- | :--- | :--- |
| username | string | √​ | 调用设备服务所需的设备侧用户名 |
| password | string | √ | 调用设备服务所需的设备侧用户名对应的密码 |

* 响应

<div class="bi-table">
  <table>
    <colgroup>
      <col width="129px" />
      <col width="142px" />
      <col width="110px" />
      <col width="447px" />
    </colgroup>
    <tbody>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">参数</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">类型</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">必选</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">描述</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">code</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">int</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">√</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">0 表示成功，其他表示失败，具体错误码请参考错误码表</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">data</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">ojbect</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">​</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">{</div>
          <div data-type="p">&quot;session_id&quot;: string</div>
          <div data-type="p">}</div>
        </td>
      </tr>
      <tr>
        <td rowspan="1" colSpan="1">
          <div data-type="p">message</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">string</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">​</div>
        </td>
        <td rowspan="1" colSpan="1">
          <div data-type="p">如果失败，则返回失败的详细错误原因，成功则返回null</div>
        </td>
      </tr>
    </tbody>
  </table>
</div>


<span data-type="color" style="color:#CF1322"><strong>其他未指明Payload的返回格式均需要符合以下格式</strong></span>:
```plain
{
    "code": int,
    "data": {},
    "message": string
}
```
## <a name="c75ggr"></a>1.2 错误码
本模块可用错误码范围为: <span data-type="background" style="background-color:rgb(250, 250, 250)"><span data-type="color" style="color:#CF1322"><strong>101 600~101699</strong></span></span>

| 错误码 | 消息 | 描述 |
| --- | --- | --- |
| 0 | SUCCESS | 成功 |
| 101 600 | ERR\_SIGNATURE\_INVALID | 签名验证失败 |
| 101 601 | ERR\_PARAM\_INVALID | 入参不合法 |
| 101 602 | ERR\_SESSION\_LIMIT | Session已达最大值 |
| 101 603 | ERR\_SESSION\_NONEXISTENT | Session不存在 |
| 101 604 | ERR\_SERVICE\_UNAVALIBE | 服务不可达 |
| 101 605 | ERR\_SERVICE\_EXIT | 服务异常退出 |
| <span data-type="color" style="color:#F5222D">101650-101699</span> |  | <span data-type="color" style="color:#F5222D">该区间为服务端错误码，后续会补充</span> |




TODO
