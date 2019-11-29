# named pipe Client
#encoding: utf-8
 
import os
import json

get_info_path = "/home/kevin/myfifo/ros_2_frontEnd"

counter = 1

f = os.open(get_info_path, os.O_RDONLY)
print("Client open f", f)
 
#rf = None
 
while True:
    # Client发送请求
  
    # len_send = os.read(f,1024)
    len_send = json.loads(f)
    print("request", counter, len_send)
 
    counter += 1
 
os.close(f)

