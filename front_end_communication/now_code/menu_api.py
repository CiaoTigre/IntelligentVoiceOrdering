import requests  # 导入requests包
import json
import os
import time


class menu:
    today_menu_url = "http://106.15.206.196:8011/api/v1/menu/todayDrink"
    order_url = "http://106.15.206.196:8011/api/v1/menu/addDrinkOrder"
    get_order_info_path = "../../myfifo/ros_2_frontEnd"
    CONFIG = {'url': order_url, 'headers': {'Content-Type': 'application/json'}}
    today_menu_name_and_id = {}
    receive = []
    is_open = 0
    f = 0

    def __init__(self):
        self.get_today_menu()

    def get_today_menu(self):
        html_str = requests.get(self.today_menu_url)  # 发送Get请求
        today_menu = json.loads(html_str.text)
        print(today_menu["menu"])
        for i in today_menu["menu"]:
            self.today_menu_name_and_id[i['nameCn']] = [i['productId'], i['price']]
        print('打印今天饮料')
        for i in self.today_menu_name_and_id:
            print(i, self.today_menu_name_and_id[i])
        # print(self.today_menu_name_and_id)

    def get_order_from_kevin(self):
        if self.is_open == 0:
            self.f = os.open(self.get_order_info_path, os.O_RDONLY)
            print("Client open f", self.f)
            self.is_open = 1
        self.receive = os.read(self.f, 1024)
        self.receive = json.loads(str(self.receive, encoding='utf-8'))  # 解析json
        return self.receive

    def send_order_to_ratio(self, data):
        response = requests.post(url=self.order_url, data=json.dumps(data), headers=self.CONFIG['headers'])
        print("发送给ration后的回应", response.text)
        response = json.loads(response.text)
        if response['errno'] != 0:
            print("error, 下单失败")
            return 1
        return 0

    def parse_data(self):
        # TODO 获得orderID, customerID, createTime, drinkID, cupType, temp, totalPrice
        data = {}
        orderDetail = {}
        now_time = int(time.time() * 1000)
        data['orderID'] = str(now_time)
        data['customerID'] = 1  # TODO 暂时不知道从哪获取
        data['createTime'] = now_time
        data['orderDetail'] = [orderDetail]
        try:
            orderDetail['drinkID'] = self.today_menu_name_and_id[self.receive['OrderInfo']['DrinkName']][0]
            print(self.receive['OrderInfo']['CupType'])
            orderDetail['cupType'] = int(self.receive['OrderInfo']['CupType'])
            orderDetail['temp'] = int(self.receive['OrderInfo']['Temp'])
            orderDetail['totalPrice'] = self.today_menu_name_and_id[self.receive['OrderInfo']['DrinkName']][1]
            print(data)
            return data
        except Exception as e:
            print("当日饮料不包括:%s" % self.receive['OrderInfo']['DrinkName'])
            print(e)
            return 0



