import serial
import re
import subprocess
import time
import requests
from dotenv import dotenv_values
from sentence_list import sentence_list

config = dotenv_values(".env")
url = f'https://api.telegram.org/bot{config["token"]}/sendMessage'

while True:
    tags = []

    with serial.Serial('/dev/ttyUSB0', 9600) as ser:
        line = ''
        line = ser.read(256)
        try:
            line = line.decode("utf-8")
        except:
            line = ''

        print(f"[{line}]")

        for tag in sentence_list(line):
            tags.append(tag)

            if tag != 'GPGSV':
                count = tags.count(tag)
                if count >= 2:
                    data = {
                        'chat_id': config["chat_id"],
                        'text': line
                    }

                    req = requests.post(url, data=data)
                    if req.status_code != requests.status.ok:
                        print("erro!")
                        
                    time.sleep(60)
