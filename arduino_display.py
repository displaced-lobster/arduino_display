import datetime
import psutil
import pyowm
import serial
import time

from secrets import api_key

ARDUINO = '/dev/ttyACM0'


def main():
    status_map = {'clear sky': 'J',
                  'few clouds': 'F',
                  'scattered clouds': 'F',
                  'broken clouds': 'F',
                  'shower rain': 'B',
                  'rain': 'G',
                  'thunderstorm': 'I',
                  'snow': 'H',
                  'mist': 'C'}
    prev_wthr_time = None
    while True:
        ser = serial.Serial(ARDUINO, 9600)
        package = ''
        now = datetime.datetime.now()
        package += now.strftime('%H%M%w%m%d')
        if prev_wthr_time is None or (now - prev_wthr_time).seconds >= 1800:
            prev_wthr_time = now
            print('Making weather request.')
            owm = pyowm.OWM(api_key)
            obs = owm.weather_at_place('Edmonton, Canada')
            w = obs.get_weather()
            weather_status = w.get_detailed_status()
            temp = w.get_temperature('celsius')
            temp = str(int(round(temp['temp'], 0)))
            status_char = status_map[weather_status]
            if now.hour >= 22 or now.hour <= 6:
                if status_char == 'J':
                    status_char = 'D'
                elif status_char == 'F':
                    status_char = 'E'
            if len(temp) == 1:
                temp = '0' + temp
        package += temp + status_char
        cpu = str(psutil.cpu_percent())
        if len(cpu) == 3:
            cpu = '0' + cpu
        ram = str(psutil.virtual_memory().percent)
        if len(ram) == 3:
            ram = '0' + ram
        package += cpu + ram + '!'
        ser.write(package.encode())
        print(package)
        print(len(package))
        time.sleep(10)


if __name__ == '__main__':
    main()
