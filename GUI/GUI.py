from functools import partial
import tkinter as tk
from tkinter import messagebox

from PortManager import PortManager
from Protocol import Protocol


# list comports

cnt = 1


class MainWindow(object):
    def __init__(self):
        """
        self.root
        self.frame_connect
        self.frame_transfer

        """
        self.root =tk.Tk() 
        self.port_manager = PortManager(
                status_intf = tk.BooleanVar, 
                warning_intf = tk.messagebox.showwarning,
                data_handler = self.handle_data)
        self.opt = self.initLayout(self.root)
        self.menubar = self.initMenubar(self.root)

    def initLayout(self, root):
        root.geometry('{}x{}'.format(600, 300))
        root.title("Smart Integrated Tile GUI")
        # create all of the main containers
        frame_connect = tk.Frame(root, bg='brown', width=200, height=300)
        frame_transfer = tk.Frame(root, bg='gray', width=400, height=300)
        # layout of containers
        frame_connect.grid(row=0, column=0)
        frame_transfer.grid(row=0, column=1)
        # output label
        opt = tk.Label(frame_connect, text='gggggggggggggggggggg', bg='green')
        opt.place(x=0, y=150, anchor='w')

        return opt

    def initMenubar(self, root):
        menubar = tk.Menu(root)
        # FIle menu
        filemenu = FileMenu(self, menubar)
        # Port menu
        portmenu = PortMenu(self, menubar)
        # Command menu
        commandmenu = CommandMenu(self, menubar)
        # add to root
        root.config(menu=menubar)

        return menubar

    def updateOpt(self, text):
        self.opt.config(text=text)

    def doJobCallBack(self):
        global cnt
        self.updateOpt('do job {}'.format(cnt))
        cnt += 1

    def handle_data(self, data):
        if len(data):
            self.updateOpt(text='Recv: {}'.format(data.decode()))

    def onExit(self):
        self.port_manager.disconnectAllPorts()
        self.root.quit()


class FileMenu(object):
    def __init__(self, main_window, menubar):
        filemenu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label='File', menu=filemenu)
        filemenu.add_command(label='New', command=main_window.doJobCallBack)
        filemenu.add_command(label='Open', command=main_window.doJobCallBack)
        filemenu.add_command(label='Save', command=main_window.doJobCallBack)
        filemenu.add_separator()
        filemenu.add_command(label='Exit', command=main_window.onExit) 



class PortMenu(object):
    def __init__(self, main_window, menubar):
        self.main_window = main_window
        self.port_manager = main_window.port_manager
        self.portmenu = tk.Menu(menubar, tearoff=0)

        menubar.add_cascade(label='Port', menu=self.portmenu)
        self.portmenu.add_command(label='Refresh', command=self.refreshPortMenu)
        self.portmenu.add_separator()
        self.refreshPortMenu()

    def refreshPortMenu(self):
        # disconnect all ports
        self.port_manager.disconnectAllPorts()
        # refresh port list
        self.port_manager.refreshList()

        self.portmenu.delete(2, 'end')
        for idx, port in enumerate(self.port_manager.port_list):
            self.portmenu.add_checkbutton(label="{}".format(port.device), onvalue=1, offvalue=0, 
                    variable=self.port_manager.port_status_list[idx], command=partial(self.connectCallBack, idx))
        self.main_window.updateOpt('port refreshed')

    # Callbacks
    def connectCallBack(self, idx):
        self.port_manager.disconnectAllPorts()
        if self.port_manager.connectPort(idx):
            self.main_window.updateOpt('comport {} connected'.format(idx))


class CommandMenu(object):
    def __init__(self, main_window, menubar):
        commandmenu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label='Command', menu=commandmenu)
        commandmenu.add_command(label='a', command=partial(main_window.port_manager.sendMessage, 'a'))
        commandmenu.add_command(label='b', command=partial(main_window.port_manager.sendMessage, 'b'))


main_window = MainWindow() 
main_window.root.mainloop()     # enter message loop
