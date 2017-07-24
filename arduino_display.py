#!/usr/bin/env python3

import datetime
import psutil
import pyowm
import serial
import serial.tools.list_ports
import time

from secrets import api_key  # Open Weather Map API key

ARDUINO = 'Arduino Mega 2560'  # Arduino to be used


class Connection:
    """Creates a serial connection to the arduino and prepares a "package" to
    be delivered. Houses all functions used to create the package.

    Package layout:
    <time><date><temp><weather><cpu><ram>
    hhmmDDMMWCP.URA.M!
    h - hour
    m - minute
    D - day
    M - month
    CP.U - cpu as a decimal
    RA.M - ram as a decimal
    ! - denotes the end of the package

    * - as the first character is the script ending signal.
    """

    def __init__(self, target, port=9600):
        """target - arduino description, port - baudrate."""
        self.target = target
        self.prev_wthr_time = None
        self.wthr_package = '  _'
        self.package = ''
        self.time = datetime.datetime.now()
        self.start_time = self.time
        self.prev_report = self.time
        self.owm = pyowm.OWM(api_key)
        self.device = self.find_device()
        self.ser = serial.Serial(self.device, port)

    def get_date_and_time_package(self):
        """Update the date and time and convert to a string for the package."""
        self.time = datetime.datetime.now()
        date_time = self.time.strftime('%H%M%w%m%d')
        if date_time[-2] == '0':
            date_time = date_time[:-2] + date_time[-1] + ' '
        return date_time

    def time_to_get_weather(self, interval=1800):
        """Decide to get weather based upon a set interval (in seconds)."""
        if self.prev_wthr_time is None:
            return True
        return (self.time - self.prev_wthr_time).seconds >= interval

    def get_weather(self):
        """Use Open Weather Map to get location weather, extract temperature
        and weather description, use status_map to map weather description to
        the appropriate character to be used be the weather font.
        """

        # https://openweathermap.org/weather-conditions
        icon_map = {'01d': 'J', '01n': 'D',
                    '02d': 'F', '02n': 'E',
                    '03d': 'A', '03n': 'A',
                    '04d': 'A', '04n': 'A',
                    '09d': 'G', '09n': 'G',
                    '10d': 'B', '10n': 'B',
                    '11d': 'I', '11n': 'I',
                    '13d': 'H', '13n': 'H',
                    '50d': 'C', '50n': 'C'}

        self.prev_wthr_time = self.time

        print('Making weather request...')

        # Try and except to avoid exception on connection failure
        try:
            obs = self.owm.weather_at_place('Edmonton, Canada')
            w = obs.get_weather()

            temp = w.get_temperature('celsius')
            temp = str(int(round(temp['temp'], 0)))
            if len(temp) == 1:
                temp = ' ' + temp

            icon = w.get_weather_icon_name()
            if icon in icon_map:
                icon_char = icon_map[icon]
            else:
                print('WARNING:', icon, 'not in icon_map!')
                icon_char = '_'

            self.wthr_package = temp + icon_char

        # TODO: Modify for specific exception
        except:
            print('OWM connection error...')

    def get_cpu_and_ram_package(self):
        """Use psutil to retrieve cpu and ram usage as a percentage and return
        together as a concatenated string.
        """
        cpu = str(psutil.cpu_percent())
        if len(cpu) == 3:
            cpu = ' ' + cpu

        ram = str(psutil.virtual_memory().percent)
        if len(ram) == 3:
            ram = ' ' + ram

        return cpu + ram

    def create_and_send_package(self):
        """Create the package be calling the appropriate class functions,
        terminate ('!') and send via serial connection to arduino.
        """
        package = self.get_date_and_time_package()
        if self.time_to_get_weather():
            self.get_weather()
        package += self.wthr_package
        package += self.get_cpu_and_ram_package()
        package += '!'

        self.ser.write(package.encode())

    def send_script_end_signal(self):
        """Send single character ('*') to singal script end."""
        self.ser.write('*'.encode())

    def find_device(self):
        """Determine the location of the arduino based upon a description."""
        ports = list(serial.tools.list_ports.comports())
        for p in ports:
            if p.description == ARDUINO:
                return p.device

    def check_for_input(self):
        if self.ser.in_waiting:
            self.toggle_lights()

    def time_for_new_package(self, interval=10):
        return (datetime.datetime.now() - self.time).seconds >= interval

    def report_up_time(self, interval=7200):
        if (self.time - self.prev_report).seconds >= interval:
            self.prev_report = self.time
            report = (self.time - self.start_time).seconds // 3600
            print('Running for:', report, 'hours')

    def send_start_signal(self):
        self.ser.write('#'.encode())


def main():
    connection = Connection(ARDUINO)
    print('Starting communication...')
    connection.send_script_end_signal()

    try:
        while True:
            if connection.time_for_new_package():
                connection.create_and_send_package()
            connection.check_for_input()
            connection.report_up_time()
            time.sleep(1)
    except KeyboardInterrupt:
        connection.send_script_end_signal()
        print('\n--Script Ended---')
    except:
        connection.send_script_end_signal()
        print('FATAL ERROR!')
        raise


if __name__ == '__main__':
    main()
