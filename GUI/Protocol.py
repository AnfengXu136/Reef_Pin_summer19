
class Protocol(object):
    """
    0x0D 0x0A length data_0 data_1 ... sum (sum starts from length)
    """
    def __init__(self, recv_package_handler=None):
        self.max_package_length = 20
        self.__max_frame_length = 45
        self.__max_buffer_length = self.__max_frame_length - 4 # exclude header, length and sum
        self.__first_byte = 0x0D
        self.__second_byte = 0x0A
        self.__pad_byte = 0x50
        # below used by recvByteHandler 
        self.__recv_package_handler = recv_package_handler if recv_package_handler is not None else self.__recvPackageHandler
        self.__recv_state = 0 # 0 is idle, 1 is waiting for length, 2 is normal receiving
        self.__recv_buffer = []
        self.__recv_last_byte = 0x00
        self.__recv_length = 0
        self.__recv_ignore_flag = 0
        self.__recv_sum = 0
        return

    # Receiving

    def __recvIdleStatus(self):
        self.__recv_state = 0 # 0 is idle, 1 is waiting for length, 2 is normal receiving
        self.__recv_buffer = []
        self.__recv_length = 0
        self.__recv_sum = 0

    def recvByteHandler(self, byte):
        if self.__recv_state == 0:
            if self.__recv_last_byte == self.__first_byte and byte == self.__second_byte:
                self.__recv_state = 1
                self.__recv_buffer.clear()
            else:
                self.__recvIdleStatus()
        elif self.__recv_state == 1:
            assert self.__recv_last_byte == self.__second_byte, "last byte not matching to {}".format(self.__second_byte)
            self.__recv_state = 2
            self.__recv_buffer.clear()
            self.__recv_length = byte
            self.__recv_sum = byte
            if byte == self.__first_byte:
                self.__recv_ignore_flag = 1
            else:
                self.__recv_ignore_flag = 0
        elif self.__recv_state == 2:
            if self.__recv_last_byte == self.__first_byte and byte == self.__second_byte:
                self.__recv_state = 1
                self.__recv_buffer.clear()
            else:
                if len(self.__recv_buffer) < self.__recv_length:
                    self.__recv_sum += byte
                    if len(self.__recv_buffer) < self.__max_buffer_length:
                        if self.__recv_ignore_flag:
                            self.__recv_ignore_flag = 0
                        else:
                            self.__recv_buffer.append(byte)
                    else:
                        self.__recvIdleStatus()
                elif len(self.__recv_buffer) == self.__recv_length:
                    if self.__recv_sum%256 == byte: # sum match
                        package = self.removePadding(self.__recv_buffer)
                        self.__recv_package_handler(package)
                        self.__recvIdleStatus()
                    else:
                        self.__recvIdleStatus()
                else:
                    self.__recvIdleStatus()
        else:
            self.__recvIdleStatus()
        # under all conditions
        self.__recv_last_byte = byte
        return

    def removePadding(self, package_padded):
        package = []
        idx = 0
        while idx < len(package_padded):
            byte = package_padded[idx]
            package.append(byte)
            if byte == self.__first_byte:
                idx += 2
            else:
                idx += 1
        return package

    def __recvPackageHandler(self, package):
        self.printAsHex(package)

    # Sending

    def packageToFrame(self, package):
        if len(package) > self.max_package_length:
            raise ValueError('package is oversized')
        # check range
        for byte in package:
            if byte < 0 or byte >= 256:
                raise ValueError('byte in package is out of range, byte = {}'.format(byte))
        # padding
        package_padded = []
        for byte in package:
            package_padded.append(byte)
            if byte == self.__first_byte:
                package_padded.append(self.__pad_byte)
        # padd for the length
        if len(package_padded) == self.__first_byte:
            frame = [len(package_padded), self.__pad_byte] + package_padded
        else:
            frame = [len(package_padded)] + package_padded
        # add sum
        frame = frame + [sum(frame)%256]
        # add header
        frame = [self.__first_byte, self.__second_byte] + frame
        # check frame length
        if len(frame) > self.__max_frame_length:
            raise ValueError('frame is oversized')
        return frame

    # Util

    def isFrameLegal(self, frame):
        if len(frame) < 4:
            return False
        if not frame[0] == self.__first_byte:
            return False
        if not frame[1] == self.__second_byte:
            return False
        if not frame[2] == len(frame) - 4:
            return False
        if not frame[-1] == sum(frame[2:-1])%256:
            return False
        return True

    def printAsHex(self, frame):
        print(" ".join(["{:02x}".format(item) for item in frame]))

