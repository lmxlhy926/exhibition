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