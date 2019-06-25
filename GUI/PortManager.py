import serial
import serial.tools.list_ports
import threading
import time

class booleanStatus(object):
    def __init__(self, val=False):
        self.__val__ = val
    
    def get(self):
        return self.__val__
    
    def set(self, val):
        self.__val__ = val

class PortManager(object):
    def __init__(self, status_intf=booleanStatus, warning_intf=None, data_handler=None, baud_rates=9600):
        # self.root = root
        self.port_list = []
        self.port_status_list = []
        self.__baud_rates = baud_rates
        self.__ser = None
        self.__ser_idx = -1
        self.__status_intf = status_intf
        self.__warning_intf = warning_intf if warning_intf is not None else self.__showWarning
        self.__data_handler = data_handler if data_handler is not None else self.__handle_data
        self.__read_thread = None
        self.refreshList()

    def __showWarning(self, title, message):
        print("Warning! Title: {}, Message: {}".format(title, message))

    def __handle_data(self, byte):
        print(hex(byte), end =" ")

    def __read_from_port(self):
        while True:
            try:
                data = self.__ser.read()
                for byte in data:
                    self.__data_handler(byte)
            except:
                break

    def disconnectAllPorts(self):
        if self.__ser is not None:
            try:
                self.__ser.close()
            except:
                self.__warning_intf(title='Warning', message='Close port failed')
            # post check
            if self.__ser.isOpen():
                self.__warning_intf(title='Warning', message='Port still open')
                return
            time.sleep(0.1)
            if self.__read_thread.isAlive():
                self.__warning_intf(title='Warning', message='Reading thread is still alive')
                return
            self.__read_thread = None
            self.__ser = None
            self.__ser_idx = -1

        for idx, port in enumerate(self.port_list):
            status = self.port_status_list[idx]
            if status.get():
                # disconnect this port
                status.set(False)

    def connectPort(self, idx):
        if self.__ser is not None:
            return
        port = self.port_list[idx].device
        try:
            print("BD rate: {}".format(self.__baud_rates))
            ser = serial.Serial(port,self.__baud_rates, timeout=0)
        except: # port occupied by other program
            self.__warning_intf(title='Warning', message='{} is occupied'.format(port))
            status = self.port_status_list[idx]
            status.set(False)
            return 0
        if ser.isOpen(): 
            self.__ser = ser
            self.__ser_idx = idx
            status = self.port_status_list[idx]
            status.set(True)
            self.__read_thread = threading.Thread(target=self.__read_from_port, args=())
            self.__read_thread.start()
            return 1
        else:
            status = self.port_status_list[idx]
            status.set(False)
            self.__warning_intf(title='Warning', message='Failed to open {}'.format(port))
            return 0

    def refreshList(self):
        port_list = list(serial.tools.list_ports.comports())
        self.port_list = [item for item in port_list if 'Serial' in item.description or "usbserial" in item.device]
        self.port_status_list = [self.__status_intf() for _ in self.port_list]

    def sendMessage(self, message):
        if self.__ser is not None and self.__ser.isOpen():
            try:
                self.__ser.write(message.encode())
            except:
                self.__warning_intf(title='Warning', message='Transmission failed')
        else:
            self.__warning_intf(title='Warning', message='Port is not open')

    def sendByteList(self, byte_list):
        if self.__ser is not None and self.__ser.isOpen():
            try:
                self.__ser.write(byte_list)
            except:
                self.__warning_intf(title='Warning', message='Transmission failed')
        else:
            self.__warning_intf(title='Warning', message='Port is not open')
