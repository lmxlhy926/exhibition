整体配置站点
    配置文件：
        baseInfo.json:      存储账户、电视参数等基本信息
        httpConfig.json:    存储云端http服务器的Ip和端口号
        mqttConfig.json:    存储云端mqtt服务器的Ip和端口号
        record.json:        记录加入大白名单操作和电视注册操作

    config.cpp运行逻辑：
        1. 设置配置文件加载路径
        2. 初始化cloudUtil: 加载http服务器地址，开启线程执行电视加入大白名单操作
        3. 加载mqtt服务器地址，设置回调函数和预处理函数，订阅相关主题，连接服务器。
        4. 设置服务回调函数

/data/changhong/edge_midware/config /data/changhong/edge_midware/paramData &


配置站点文件同步逻辑
    发布消息
    {
        "file" : "rule.json",
        "timeStamp": 17723424
    }

    1. 订阅其它配置站点的文件改变消息，有多少个配置站点则订阅多少个
    2. 收到消息后，和本地对应文件进行比较，
                1. 如果其更新，则更新之
                2. 如果一样，则不作任何处理
                3. 如果自己更新，则发布更新消息。

    订阅时机
        站点上线时，订阅局域网内发现的所有的配置站点


    文件更新消息触发机制：
        1. 站点上线
        2. 文件被改变
        3. 实时更新