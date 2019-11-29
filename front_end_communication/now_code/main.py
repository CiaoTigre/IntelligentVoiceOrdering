__author__ = 'dss'

import os
from redismq import RSMQueue
import json
import time
from menu_api import menu

orderID = 1  # 订单号

if __name__ == '__main__':

    q1 = RSMQueue('track')
    menu1 = menu()
    while True:
        # 获得信息
        receive_info = menu1.get_order_from_kevin()
        # print(receive_info)

        # 发送信息给前端
        try:
            mid = q1.publish(receive_info)  # msg数据
            print('publish 1', receive_info)
            # 发送信息给ratio下单
            if receive_info['OrderInfo']['orderFinish'] == 'true':
                # 解析数据
                to_ratio_data = menu1.parse_data()
                # 发送数据
                if to_ratio_data != 0:  # 解析错误返回0
                    if menu1.send_order_to_ratio(to_ratio_data) == 0:  # 发送成功
                        orderID += 1
        except Exception as e:
            print(e)

        # 等待
        time.sleep(.1)
    os.close(f)
