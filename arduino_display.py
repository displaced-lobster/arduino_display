import datetime
import psutil
import pyowm
import serial
import time

from secrets import api_key  # Open Weather Map API key

ARDUINO = '/dev/ttyACM0'


class Package:
    def __init__(self, target, port=9600):
        self.target = target
        self.prev_wthr_time = None
        self.wthr_package = None
        self.package = ''
        self.time = datetime.datetime.now()
        self.owm = pyowm.OWM(api_key)
        self.ser = serial.Serial(target, port)

    def get_date_and_time_package(self):
        self.time = datetime.datetime.now()
        date_time = self.time.strftime('%H%M%w%m%d')
        if date_time[-2] == '0':
            date_time = date_time[:-2] + date_time[-1] + ' '
        return date_time

    def time_to_get_weather(self, interval=1800):
        if self.prev_wthr_time is None:
            return True
        return (self.time - self.prev_wthr_time).seconds >= interval

    def get_weather(self):
        status_map = {'clear sky': 'J',
                      'few clouds': 'F',
                      'scattered clouds': 'F',
                      'broken clouds': 'F',
                      'shower rain': 'B',
                      'rain': 'G',
                      'thunderstorm': 'I',
                      'snow': 'H',
                      'mist': 'C'}
        self.prev_wthr_time = self.time

        print('Making weather request...')

        obs = self.owm.weather_at_place('Edmonton, Canada')
        w = obs.get_weather()

        temp = w.get_temperature('celsius')
        temp = str(int(round(temp['temp'], 0)))
        if len(temp) == 1:
            temp = ' ' + temp

        weather_status = w.get_detailed_status()
        status_char = status_map[weather_status]
        if self.time.hour >= 22 or self.time.hour <= 6:
            if status_char == 'J':
                status_char = 'D'
            elif status_char == 'F':
                status_char = 'E'

        self.wthr_package = temp + status_char

    def get_cpu_and_ram_package(self):
        cpu = str(psutil.cpu_percent())
        if len(cpu) == 3:
            cpu = ' ' + cpu

        ram = str(psutil.virtual_memory().percent)
        if len(ram) == 3:
            ram = ' ' + ram

        return cpu + ram

    def create_and_send_package(self):
        package = self.get_date_and_time_package()
        if self.time_to_get_weather():
            self.get_weather()
        package += self.wthr_package
        package += self.get_cpu_and_ram_package()
        package += '!'
        print(package)
        self.ser.write(package.encode())

    def send_script_ended_package(self):
        self.ser.write('*'.encode())


def main():
    package = Package(ARDUINO)
    try:
        while True:
            package.create_and_send_package()
            time.sleep(10)
    except KeyboardInterrupt:
        package.send_script_ended_package()
        print('--Script Ended---')


if __name__ == '__main__':
    main()
