控制灯：
    1. 接收交互app下发的伪指令，提取伪指令信息，构造DownCommandData。
    2. 获取智慧家Adapter站点，zigbee站点设备列表信息;（只要能获取到一个站点的设备信息就算成功）
    3. 根据匹配规则，找到设备列表里匹配的设备信息项
    4. 依据DownCommandData和设备信息项构造控制指令
    5. 发送控制指令到相应的站点，对设备进行控制

电视执行命令
/data/changhong/edge_midware/synergy /data/changhong/chiotedge/paramData &