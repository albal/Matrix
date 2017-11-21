import serial
import time
import wmi
import math
import tkinter
import tkinter.ttk as ttk
import serial.tools.list_ports
from tkinter import *

global w
w = wmi.WMI(namespace="root\OpenHardwareMonitor")

global cpu
global gpu
global ram
global ramt
global bus
global load
global progdelay
progdelay = "2"

backlight = []
result = []

def serial_ports():    
    return serial.tools.list_ports.comports()

def spinCallBack():
    global progdelay
    progdelay = ww.get()

def save_setting():
    fo = open("LCDctrl.ini", "w")
    fo.write(result[0] + "\n")
    backlight.append(str(var.get()))
    fo.write(backlight[0] + "\n")
    fo.write(progdelay)
    fo.close()
    
def close_window():
    backlight.append(str(var.get()))
    root.destroy()

def on_select(event=None):
    result.append(cb.get()[0:4])

try:
    fo = open("LCDctrl.ini", "r+")
    str = fo.read(4);
    result.append(str)
    position = fo.seek(6, 0);
    backlight = fo.read(3);
    position = fo.seek(11, 0);
    progdelay = fo.read(2);
    fo.close()
except FileNotFoundError:
    root = tkinter.Tk()
    bottomframe = Frame(root)
    bottomframe.pack( side = BOTTOM )
    label = Label(root, text="Select COM Port")
    label2 = Label(root, text="Refresh Delay:")
    cb = ttk.Combobox(root, values=serial_ports())
    B = Button(bottomframe, text ="Close", command = close_window)
    S = Button(bottomframe, text ="Save", command = save_setting)
    var = DoubleVar()
    scale = Scale(root, variable = var, orient=HORIZONTAL, length=150, label="Backlight Intensity")
    scale.pack()
    ww = Spinbox(root, from_=2, to=10, command=spinCallBack)
    label2.pack()
    ww.pack()
    label.pack()
    cb.pack()
    S.pack(side=LEFT)
    B.pack(side=LEFT)
    cb.bind('<<ComboboxSelected>>', on_select)
    root.mainloop()
   

def getRamSize():
    used = 0
    free = 0
    hw_infos = w.Sensor()
    for sensor in hw_infos:
        if sensor.SensorType==u'Data':
            if sensor.Name == u'Used Memory':
                used = int(math.ceil(sensor.Value))
            elif sensor.Name == u'Available Memory':
                free = int(math.ceil(sensor.Value))
    return used + free - 1


def updateHWData():
    global w
    global cpu
    global gpu
    global ram
    global bus
    global load
    hw_infos = w.Sensor()
    cpu = [] 
    gpu = []
    bus = []
    load = []
    ram = []
    
    for sensor in hw_infos:
        #print (sensor.SensorType)
        #print(sensor.Name)
        #print(sensor.Value)
        
        if sensor.SensorType==u'Temperature':
            #if sensor.Name == u'CPU':
             #   cpu.insert(0,  "temp:%dC " % sensor.Value)
            if sensor.Name == u'GPU Core':
                gpu.insert(0,  "T:%d " % sensor.Value)
                
        if sensor.SensorType==u'Clock':
            #if sensor.Name == u'CPU Core #1':
             #   cpu.insert(1,  "CPU clk: %dMHz" % sensor.Value)
            if sensor.Name == u'GPU Core':
                gpu.insert(1,  "GPU/MEM %d " % sensor.Value)
            #if sensor.Name == u'Bus Speed':
             #   bus.insert(1, "Bus clk: %dMHz " % sensor.value)
            if sensor.Name == u'GPU Memory':
                bus.insert(0, "%d Mc" % sensor.value)
        
        if sensor.SensorType==u'Data':
            if sensor.Name == u'Used Memory':
                ram.insert(0,  "RAM: %.2fGB " % sensor.Value)
            if sensor.Name == u'Available Memory':
                ram.insert(1,  "Av:%.2fGB " % sensor.Value)
                
        if sensor.SensorType==u'Load':
            if sensor.Name == u'CPU Total':
                load.insert(0,  "CPU:%d%% " % sensor.Value)
            if sensor.Name == u'GPU Core':
                load.insert(1,  "GPU:%d%% " % sensor.Value)
            if sensor.Name == u'Memory':
                load.insert(2,  "%d%% " % sensor.Value)
            if sensor.Name == u'Used Space':
                load.insert(3,  "Used Space: %d " % sensor.Value)

#def serial_ports():
#    ports = ['COM%s' % (i + 1) for i in range(256)]
#    result = []
#    for port in ports:
#        try:
#            s = serial.Serial(port)
#            s.close()
#            result.append(port)
#        except (OSError, serial.SerialException):
#            pass
#    return result

arduino = serial.Serial(result[0], 19200, timeout=.7)
time.sleep(2)

arduino.write("Waiting for".encode())
arduino.write(bytearray([254, 71, 1, 2]))
arduino.write("Open HW Monitor".encode())

while len(w.Sensor()) < 1:
    time.sleep(1)
time.sleep(0.5)
cpu = [] 
gpu = []
ramt = ["Total:{:2d}GB ".format(getRamSize())]

while True:
    #startTime = int(round(time.time() * 1000))
    
    #for i in range(4):
    try:
        updateHWData()
        time.sleep(float(progdelay))
        arduino.write(bytearray([254, 88]))
        arduino.write(load[0].encode())
        arduino.write(load[1].encode())
        arduino.write(gpu[0].encode())
        arduino.write(bytearray([254, 71, 1, 2]))
        arduino.write(gpu[1].encode())
        arduino.write(bus[0].encode())
        updateHWData()
        time.sleep(float(progdelay))
        arduino.write(bytearray([254, 88]))
        arduino.write(ram[0].encode())
        arduino.write(load[2].encode())
        arduino.write(bytearray([254, 71, 1, 2]))
        arduino.write(ramt[0].encode())
        arduino.write(ram[1].encode())
    except IndexError:
        time.sleep(float(progdelay))

    #print ("estimated time: ", int(round(time.time() * 1000)) - startTime)

