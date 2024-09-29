import serial
import re
import subprocess
import time
import requests
from dotenv import dotenv_values
from sentence_list import sentence_list

config = dotenv_values(".env")
url = f'https://api.telegram.org/bot{config["token"]}/sendMessage'

ser = serial.Serial("/dev/ttyUSB0", 9600)

while True:
    valid_data_re = re.compile(r"^\$([A-Z0-9\.\,\-\$\*])+")
    tags = []

    # lê a linha e remove CRLF
    gps_line = ser.readline()
    gps_line = gps_line[:-2]

    try:
        gps_line = gps_line.decode("utf-8")
        match = valid_data_re.match(gps_line)

        if match is not None:
            tag_regex = re.compile(r"^(\$[A-Z]{5})")
            match = tag_regex.match(gps_line)

            # TODO: fazer um préprocessamento nos dados para enviar
            # apenas quando houver latitude, longitude e altitude
            if match is not None and (
                match[1] == "$GNRMC" or match[1] == "$GPGGA"
            ):
                data = {"chat_id": config["chat_id"], "text": gps_line}
                req = requests.post(url, data=data)
                if req.status_code != requests.codes.ok:
                    print("erro!")
                
                time.sleep(60)
    except UnicodeDecodeError:
        print("deu ruim!")


def get_tag(gps_line):
    regex_tag = re.compile(r"")
