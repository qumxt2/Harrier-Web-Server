from PySide.QtCore import *
from PySide.QtGui import *
from harrierUI import *
import sys
import minimalmodbus

# Constants
BAUD_RATE = 115200
MODBUS_TIMEOUT = 10.0

# Modbus Registers
REG_PRESSURE_PSI = 1000
REG_BATTERY_MILLIVOLTS = 1002
REG_PUMP_STATUS = 1004
REG_CYCLE_PROGRESS = 1006
REG_TOTALIZER = 1008
REG_GRAND_TOTALIZER = 1010
REG_ALARM_BITFIELD = 1012
REG_REMOTE_DISABLE_ACTIVE = 1014
REG_PUMP_STATUS_SET = 1016

REG_ALARM_ACTION = 1028
REG_METERING_MODE = 1030
REG_ON_TIME = 1032
REG_OFF_TIME = 1034
REG_ON_CYCLES = 1036
REG_ON_TIMEOUT = 1038
REG_POWER_SAVE_MODE = 1040
REG_UNITS = 1042
REG_PRESSURE_OFFSET = 1044
REG_PRESSURE_SLOPE = 1046
REG_KFACTOR = 1048
REG_DESIRED_FLOW_RATE = 1050
REG_SOFTWARE_VERSION = 1052
REG_HIGH_PRESSURE_TRIGGER = 1054
REG_LOW_PRESSURE_TRIGGER = 1056
REG_BATTERY_WARNING_TRIGGER = 1058
REG_BATTERY_SHUTOFF_TRIGGER = 1060
REG_ALARM1_TRIGGER = 1062
REG_ALARM2_TRIGGER = 1064
REG_REMOTE_OFF_TRIGGER = 1066
REG_VOLUME_MODE_INTERVAL = 1068
REG_METERING_MODE_SET = 1070
REG_ON_TIME_SET = 1072
REG_OFF_TIME_SET = 1074
REG_ON_CYCLES_SET = 1076
REG_ON_TIMEOUT_SET = 1078
REG_POWER_SAVE_MODE_SET = 1080
REG_UNITS_SET = 1082
REG_PRESSURE_OFFSET_SET = 1084
REG_PRESSURE_SLOPE_SET = 1086
REG_KFACTOR_SET = 1088
REG_DESIRED_FLOW_RATE_SET = 1090
REG_HIGH_PRESSURE_TRIGGER_SET = 1092
REG_LOW_PRESSURE_TRIGGER_SET = 1094
REG_BATTERY_WARNING_TRIGGER_SET = 1096
REG_BATTERY_SHUTOFF_TRIGGER_SET = 1098
REG_ALARM1_TRIGGER_SET = 1100
REG_ALARM2_TRIGGER_SET = 1102
REG_REMOTE_OFF_TRIGGER_SET = 1104
REG_VOLUME_MODE_INTERVAL_SET = 1106
REG_ALARM_ACTION_SET = 1108

REG_TOTALIZER_RESET = 1120
REG_CLEAR_ALARM_STATUS = 1122

class appWindow(QtGui.QMainWindow):

    # Member variables
    modbus = None
    pumpStatus = 0

#*******************************************************************************
    def __init__(self, *args):
        QtGui.QMainWindow.__init__(self, *args)
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.configureCallbacks()

#*******************************************************************************
    def configureCallbacks(self):
        self.ui.PumpStatusButton.clicked.connect(self.onStatusButtonClick)
        self.ui.ConnectButton.clicked.connect(self.onConnectButtonClick)
        self.ui.MeteringModeSelectBox.currentIndexChanged.connect(self.onMeteringModeChange)
        self.ui.FlowRateSpinBox.valueChanged.connect(self.onFlowRateChange)
        self.ui.ResetTotalizerButton.clicked.connect(self.onResetTotalizerClick)
        self.ui.UpdateButton.clicked.connect(self.onUpdateButtonClick)
        self.ui.ClearAlarmButton.clicked.connect(self.onClearAlarmButtonClick)
        self.ui.HighPressureTriggerSpinBox.valueChanged.connect(self.onHighPressureTriggerChange)
        self.ui.LowPressureTriggerSpinBox.valueChanged.connect(self.onLowPressureTriggerChange)
        self.ui.OnCyclesSpinBox.valueChanged.connect(self.onOnCyclesChange)
        self.ui.IntervalSelectBox.currentIndexChanged.connect(self.onIntervalChange)
        self.ui.KFactorSpinBox.valueChanged.connect(self.onKFactorChange)
        self.ui.OnTimeEditBox.timeChanged.connect(self.onOnTimeChange)
        self.ui.OffTimeEditBox.timeChanged.connect(self.onOffTimeChange)
        self.ui.OnTimeoutEditBox.timeChanged.connect(self.onOnTimeoutChange)
        self.ui.PowerSaveOnTriggerSpinBox.valueChanged.connect(self.onPowerSaveOnChange) 
        self.ui.LowBatteryTriggerSpinBox.valueChanged.connect(self.onLowBatteryTriggerChange)
        self.ui.Input1SelectBox.currentIndexChanged.connect(self.onInput1Change)
        self.ui.Input2SelectBox.currentIndexChanged.connect(self.onInput2Change)
        self.ui.RemoteOffSelectBox.currentIndexChanged.connect(self.onRemoteOffChange)
        self.ui.PowerSavingsSelectBox.currentIndexChanged.connect(self.onPowerSavingModeChange)
        self.ui.UnitsSelectBox.currentIndexChanged.connect(self.onUnitsChange)
        self.ui.PressureOffsetSpinBox.valueChanged.connect(self.onPressureOffsetChange)
        self.ui.PressureSensitivitySpinBox.valueChanged.connect(self.onPressureSensitivityChange)
        self.ui.AlarmActionSelectBox.currentIndexChanged.connect(self.onAlarmActionChange)

#*******************************************************************************
    def modbusWrite(self, register, value):
        self.modbus.write_long(register - 1 , value)

#*******************************************************************************
    def modbusRead(self, register):
        return self.modbus.read_long(register - 1)
 
#*******************************************************************************
    def readModbusRegisters(self):
        self.updatePumpStatus()
        self.updateMeteringMode()
        self.updateFlowRate()
        self.updateTotalizer()
        self.updateGrandTotalizer()
        self.updateBatteryVoltage()
        self.updatePressure()
        self.updateVersion()
        self.updateHighPressureTrigger()
        self.updateLowPressureTrigger()
        self.updatePowerSaveOnTrigger()
        self.updateLowBatteryTrigger()
        self.updateKFactor()
        self.updateOnCycles()
        self.updateInterval()
        self.updateOnTime()
        self.updateOffTime()
        self.updateOnTimeout()
        self.updateInput1()
        self.updateInput2()
        self.updateRemoteOff()
        self.updatePowerSavingMode()
        self.updateUnits()
        self.updatePressureOffset()
        self.updatePressureSensitivity()
        self.updateAlarmAction()
       
#*******************************************************************************
    def onConnectButtonClick(self,):
        minimalmodbus.CLOSE_PORT_AFTER_EACH_CALL = True
        minimalmodbus.BAUDRATE = BAUD_RATE
        minimalmodbus.TIMEOUT = MODBUS_TIMEOUT
        try:
            comPort = '\\\\.\\' + self.ui.ComPortField.text()
            slaveAddress = int(self.ui.SlaveIDField.text())
            self.modbus = minimalmodbus.Instrument(comPort, slaveAddress)
            self.readModbusRegisters()
            self.ui.ConnectButton.setEnabled(False)
        except:
            pass

#*******************************************************************************
    def onUpdateButtonClick(self):
        self.readModbusRegisters()

#*******************************************************************************
    def onClearAlarmButtonClick(self):
        self.modbusWrite(REG_CLEAR_ALARM_STATUS, 1)
        self.updatePumpStatus()
        
#*******************************************************************************        
    def updatePumpStatus(self):
        self.pumpStatus = self.modbusRead(REG_PUMP_STATUS)
        if (self.pumpStatus == 0):
            self.ui.PumpStatusButton.setText('Turn Pump On')
            self.ui.StatusField.setText('Standby')
        elif (self.pumpStatus == 1):
            self.ui.PumpStatusButton.setText('Turn Pump Off')
            self.ui.StatusField.setText('Run')
        elif (self.pumpStatus == 2):
            self.ui.PumpStatusButton.setText('Turn Pump On')
            self.ui.StatusField.setText('Disabled By Alarm')
            self.activeAlarm = True
        elif (self.pumpStatus == 3):
            self.ui.PumpStatusButton.setText('Turn Pump On')
            self.ui.StatusField.setText('Disabled By Remote')
            
        self.updateAlarmsActive()
        
#*******************************************************************************
    def onStatusButtonClick(self):
        self.updatePumpStatus()
        if (self.pumpStatus == 0):
            self.modbusWrite(REG_PUMP_STATUS_SET, 1) # Run
        elif (self.pumpStatus == 1):
            self.modbusWrite(REG_PUMP_STATUS_SET, 0) # Standby
        self.updatePumpStatus()            

#*******************************************************************************
    def updateMeteringMode(self):
        meteringMode = int(self.modbusRead(REG_METERING_MODE))
        self.ui.MeteringModeSelectBox.setCurrentIndex(meteringMode)
        if (meteringMode == 0):
            self.ui.MeteringModeField.setText('Flow')
        elif (meteringMode == 1):
            self.ui.MeteringModeField.setText('Time')
        elif (meteringMode == 2):
            self.ui.MeteringModeField.setText('Cycle')
        
#*******************************************************************************
    def onMeteringModeChange(self):
        meteringMode = self.ui.MeteringModeSelectBox.currentIndex()
        self.modbusWrite(REG_METERING_MODE_SET, meteringMode)
        self.updateMeteringMode()

#*******************************************************************************
    def updateInput1(self):
        triggerInput1 = int(self.modbusRead(REG_ALARM1_TRIGGER))
        self.ui.Input1SelectBox.setCurrentIndex(triggerInput1)
        if (triggerInput1 == 0):
            self.ui.Input1Field.setText("Disabled")
        elif (triggerInput1 == 1):
            self.ui.Input1Field.setText("Nor Closed")
        elif (triggerInput1 == 2):
            self.ui.Input1Field.setText("Nor Open")
#*******************************************************************************
    def onInput1Change(self):
        triggerInput1 = self.ui.Input1SelectBox.currentIndex()
        self.modbusWrite(REG_ALARM1_TRIGGER_SET, triggerInput1)
        self.updateInput1()

#*******************************************************************************    
    def updateInput2(self):
        triggerInput2 = int(self.modbusRead(REG_ALARM2_TRIGGER))
        self.ui.Input2SelectBox.setCurrentIndex(triggerInput2)
        if (triggerInput2 == 0):
            self.ui.Input2Field.setText("Disabled")
        elif (triggerInput2 == 1):
            self.ui.Input2Field.setText("Nor Closed")
        elif (triggerInput2 == 2):
            self.ui.Input2Field.setText("Nor Open")
#*******************************************************************************
    def onInput2Change(self):
        triggerInput2 = self.ui.Input2SelectBox.currentIndex()
        self.modbusWrite(REG_ALARM2_TRIGGER_SET, triggerInput2)
        self.updateInput2()

#*******************************************************************************
    def updateRemoteOff(self):
        triggerRemoteOff = int(self.modbusRead(REG_REMOTE_OFF_TRIGGER))
        self.ui.RemoteOffSelectBox.setCurrentIndex(triggerRemoteOff)
        if (triggerRemoteOff == 0):
            self.ui.RemoteOffField.setText("Disabled")
        elif (triggerRemoteOff == 1):
            self.ui.RemoteOffField.setText("Nor Closed")
        elif (triggerRemoteOff == 2):
            self.ui.RemoteOffField.setText("Nor Open")
#*******************************************************************************
    def onRemoteOffChange(self):
        triggerRemoteOff = self.ui.RemoteOffSelectBox.currentIndex()
        self.modbusWrite(REG_REMOTE_OFF_TRIGGER_SET, triggerRemoteOff)
        self.updateRemoteOff()

#*******************************************************************************
    def updateFlowRate(self):
        flowRate = self.modbusRead(REG_DESIRED_FLOW_RATE)
        self.ui.FlowRateField.setText(str(flowRate / 100))
        self.ui.FlowRateSpinBox.setValue(flowRate)
        
#*******************************************************************************
    def onFlowRateChange(self):
        flowRate = self.ui.FlowRateSpinBox.value()
        self.modbusWrite(REG_DESIRED_FLOW_RATE_SET, flowRate)
        self.updateFlowRate()         

#*******************************************************************************
    def onHighPressureTriggerChange(self):
        highPressureTrigger = self.ui.HighPressureTriggerSpinBox.value()
        self.modbusWrite(REG_HIGH_PRESSURE_TRIGGER_SET, highPressureTrigger)
        self.updateHighPressureTrigger()

#*******************************************************************************
    def onLowPressureTriggerChange(self):
        lowPressureTrigger = self.ui.LowPressureTriggerSpinBox.value()
        self.modbusWrite(REG_LOW_PRESSURE_TRIGGER_SET, lowPressureTrigger)
        self.updateLowPressureTrigger()
                
#*******************************************************************************        
    def updateTotalizer(self):
        total = self.modbusRead(REG_TOTALIZER)
        self.ui.TotalizerField.setText(str(total / 10))

#*******************************************************************************
    def onResetTotalizerClick(self):
        self.modbusWrite(REG_TOTALIZER_RESET, 1)
        self.updateTotalizer()

#*******************************************************************************
    def updateGrandTotalizer(self):
        total = self.modbusRead(REG_GRAND_TOTALIZER)
        self.ui.GrandTotalizerField.setText(str(total / 10))

#*******************************************************************************
    def updateBatteryVoltage(self):
        millivolts = self.modbusRead(REG_BATTERY_MILLIVOLTS)
        self.ui.BatteryVoltageField.setText(str(millivolts / 1000))

#*******************************************************************************
    def updatePressure(self):
        pressure = self.modbusRead(REG_PRESSURE_PSI)
        self.ui.PressureField.setText(str(pressure))

#*******************************************************************************
    def updateVersion(self):
        version = self.modbusRead(REG_SOFTWARE_VERSION)
        major = (version & (255<<16)) >> 16
        minor = (version & (255<<8)) >> 8
        build = (version & 255)
        self.ui.FirmwareVersionField.setText(str(major) + "." + str(minor) + "." + str(build))
#
#*******************************************************************************
    def updateHighPressureTrigger(self):
        highTrigger = self.modbusRead(REG_HIGH_PRESSURE_TRIGGER)
        self.ui.HighPressureTriggerField.setText(str(highTrigger))
        self.ui.HighPressureTriggerSpinBox.setValue(highTrigger)
#
#*******************************************************************************
    def updateLowPressureTrigger(self):
        lowTrigger = self.modbusRead(REG_LOW_PRESSURE_TRIGGER)
        self.ui.LowPressureTriggerField.setText(str(lowTrigger))
        self.ui.LowPressureTriggerSpinBox.setValue(lowTrigger)
#
#*******************************************************************************
    def updatePowerSaveOnTrigger(self):
        powerSaveTrigger = self.modbusRead(REG_BATTERY_WARNING_TRIGGER)
        self.ui.PowerSaveOnTriggerField.setText(str(powerSaveTrigger / 1000))
        self.ui.PowerSaveOnTriggerSpinBox.setValue(powerSaveTrigger)
#
#*******************************************************************************
    def onPowerSaveOnChange(self):
        powerSaveOnTrigger = self.ui.PowerSaveOnTriggerSpinBox.value()
        self.modbusWrite(REG_BATTERY_WARNING_TRIGGER_SET, powerSaveOnTrigger)
        self.updatePowerSaveOnTrigger()
        
#*******************************************************************************
    def updateLowBatteryTrigger(self):
        batteryTrigger = self.modbusRead(REG_BATTERY_SHUTOFF_TRIGGER)
        self.ui.LowBatteryTriggerField.setText(str(batteryTrigger / 1000))
        self.ui.LowBatteryTriggerSpinBox.setValue(batteryTrigger)
#
#*******************************************************************************
    def onLowBatteryTriggerChange(self):
        lowBatteryTrigger = self.ui.LowBatteryTriggerSpinBox.value()
        self.modbusWrite(REG_BATTERY_SHUTOFF_TRIGGER_SET, lowBatteryTrigger)
        self.updateLowBatteryTrigger()
        
#*******************************************************************************
    def updateAlarmsActive(self):
        alarmsActiveList = []
        alarmNamesList = ["Unknown", \
                          "Software Fault", \
                          "Low level", \
                          "Already pressurized", \
                          "Pressure not achieved", \
                          "Over cycle", \
                          "Cycle not detected", \
                          "Count not achieved", \
                          "Input 1", \
                          "Input 2", \
                          "Temperature", \
                          "Low battery", \
                          "Remote off", \
                          "High pressure", \
                          "Low pressure" ]
        alarmBitfield = self.modbusRead(REG_ALARM_BITFIELD)
        if alarmBitfield == 0:
            self.ui.AlarmActiveField.setText("None")
        else:
            for index in range(len(alarmNamesList)):
                mask = (1 << index)
                if alarmBitfield & mask:
                    alarmsActiveList.append(alarmNamesList[index+1])
                    self.ui.AlarmActiveField.setText( ', ' .join(alarmsActiveList))
#
#*******************************************************************************        
    def updateKFactor(self):
        KFactor = self.modbusRead(REG_KFACTOR)
        self.ui.KFactorField.setText(str(KFactor))
        self.ui.KFactorSpinBox.setValue(KFactor)

#*******************************************************************************
    def onKFactorChange(self):
        newKFactor = self.ui.KFactorSpinBox.value()
        self.modbusWrite(REG_KFACTOR_SET, newKFactor)
        self.updateKFactor()         

#*******************************************************************************
    def updateOnCycles(self):
        onCycles = self.modbusRead(REG_ON_CYCLES)
        self.ui.OnCyclesField.setText(str(onCycles))
        self.ui.OnCyclesSpinBox.setValue(onCycles)        

#*******************************************************************************
    def onOnCyclesChange(self):
        newOnCycles = self.ui.OnCyclesSpinBox.value()
        self.modbusWrite(REG_ON_CYCLES_SET, newOnCycles)
        self.updateOnCycles()         

#*******************************************************************************
    def updateOnTime(self):
        onTime = self.modbusRead(REG_ON_TIME)
        remainingSeconds = onTime
        
        hours = int(remainingSeconds / 3600)
        remainingSeconds -= (hours * 3600)
        minutes = int(remainingSeconds / 60)
        seconds = remainingSeconds - (minutes * 60)  
        
        self.ui.OnTimeField.setText(str(hours) + ":" + str(minutes) + ":" + str(seconds))

#*******************************************************************************
    def onOnTimeChange(self):
        newOnTime= self.ui.OnTimeEditBox.time().toString()
        hour = self.ui.OnTimeEditBox.time().hour()
        minute = self.ui.OnTimeEditBox.time().minute()
        second = self.ui.OnTimeEditBox.time().second()

        timeInSeconds = (hour * 3600) + (minute * 60) + second

        #self.ui.OnTimeField.setText(newOnTime)
        self.modbusWrite(REG_ON_TIME_SET, timeInSeconds)
        self.updateOnTime()  

#*******************************************************************************
    def updateOffTime(self):
        offTime = self.modbusRead(REG_OFF_TIME)
        remainingSeconds = offTime
        
        hours = int(remainingSeconds / 3600)
        remainingSeconds -= (hours * 3600)
        minutes = int(remainingSeconds / 60)
        seconds = remainingSeconds - (minutes * 60)  
        
        self.ui.OffTimeField.setText(str(hours) + ":" + str(minutes) + ":" + str(seconds))

#*******************************************************************************
    def onOffTimeChange(self):
        newOffTime= self.ui.OffTimeEditBox.time().toString()
        hour = self.ui.OffTimeEditBox.time().hour()
        minute = self.ui.OffTimeEditBox.time().minute()
        second = self.ui.OffTimeEditBox.time().second()

        timeInSeconds = (hour * 3600) + (minute * 60) + second

        #self.ui.OnTimeField.setText(newOnTime)
        self.modbusWrite(REG_OFF_TIME_SET, timeInSeconds)
        self.updateOffTime()  

#*******************************************************************************
    def updateOnTimeout(self):
        onTimeout = self.modbusRead(REG_ON_TIMEOUT)
        remainingSeconds = onTimeout
        
        hours = int(remainingSeconds / 3600)
        remainingSeconds -= (hours * 3600)
        minutes = int(remainingSeconds / 60)
        seconds = remainingSeconds - (minutes * 60)  
        
        self.ui.OnTimeoutField.setText(str(hours) + ":" + str(minutes) + ":" + str(seconds))

#*******************************************************************************
    def onOnTimeoutChange(self):
        newOnTimeout= self.ui.OnTimeoutEditBox.time().toString()
        hour = self.ui.OnTimeoutEditBox.time().hour()
        minute = self.ui.OnTimeoutEditBox.time().minute()
        second = self.ui.OnTimeoutEditBox.time().second()

        timeInSeconds = (hour * 3600) + (minute * 60) + second

        #self.ui.OnTimeField.setText(newOnTime)
        self.modbusWrite(REG_ON_TIMEOUT_SET, timeInSeconds)
        self.updateOnTimeout()

#*******************************************************************************
    def updatePowerSavingMode(self):
        powerSavingMode = self.modbusRead(REG_POWER_SAVE_MODE)
        self.ui.PowerSavingsSelectBox.setCurrentIndex(powerSavingMode)        
        if (powerSavingMode == 0):
            self.ui.PowerSavingsField.setText(str("Off"))
        elif (powerSavingMode == 1):
            self.ui.PowerSavingsField.setText(str("Notify"))
        elif (powerSavingMode == 2):
            self.ui.PowerSavingsField.setText(str("Min"))
        elif (powerSavingMode == 3):
            self.ui.PowerSavingsField.setText(str("Normal"))
        elif (powerSavingMode == 4):
            self.ui.PowerSavingsField.setText(str("Max"))

#*******************************************************************************
    def onPowerSavingModeChange(self):
        powerSavingMode = self.ui.PowerSavingsSelectBox.currentIndex()
        self.modbusWrite(REG_POWER_SAVE_MODE_SET, powerSavingMode)
        self.updatePowerSavingMode()

#*******************************************************************************
    def updateUnits(self):
        units = self.modbusRead(REG_UNITS)
        self.ui.UnitsSelectBox.setCurrentIndex(units)        
        if (units == 0):
            self.ui.UnitsField.setText(str("US"))
        elif (units == 1):
            self.ui.UnitsField.setText(str("Metric"))

#*******************************************************************************
    def onUnitsChange(self):
        units = self.ui.UnitsSelectBox.currentIndex()
        self.modbusWrite(REG_UNITS_SET, units)
        self.updateUnits()
        
#*******************************************************************************
    def updatePressureOffset(self):
        pressureOffset = self.modbusRead(REG_PRESSURE_OFFSET)
        self.ui.PressureOffsetField.setText(str(pressureOffset / 100))
        self.ui.PressureOffsetSpinBox.setValue(pressureOffset)

#*******************************************************************************
    def onPressureOffsetChange(self):
        pressureOffset = self.ui.PressureOffsetSpinBox.value()
        self.modbusWrite(REG_PRESSURE_OFFSET_SET, pressureOffset)
        self.updatePressureOffset()

#*******************************************************************************
    def updatePressureSensitivity(self):
        pressureSensitivity = self.modbusRead(REG_PRESSURE_SLOPE)
        self.ui.PressureSensitivityField.setText(str(pressureSensitivity / 100))
        self.ui.PressureSensitivitySpinBox.setValue(pressureSensitivity)

#*******************************************************************************
    def onPressureSensitivityChange(self):
        pressureSensitivity = self.ui.PressureSensitivitySpinBox.value()
        self.modbusWrite(REG_PRESSURE_SLOPE_SET, pressureSensitivity)
        self.updatePressureSensitivity()

#*******************************************************************************
    def updateInterval(self):
        interval = self.modbusRead(REG_VOLUME_MODE_INTERVAL)
        self.ui.IntervalSelectBox.setCurrentIndex(interval)
        if (interval == 0):
            self.ui.IntervalField.setText('Short')
        elif (interval == 1):
            self.ui.IntervalField.setText('Medium')
        elif (interval == 2):
            self.ui.IntervalField.setText('Long')  
                  
#*******************************************************************************
    def onIntervalChange(self):
        newInterval = self.ui.IntervalSelectBox.currentIndex()
        self.modbusWrite(REG_VOLUME_MODE_INTERVAL_SET, newInterval)
        self.updateInterval()  
               
#*******************************************************************************
    def updateAlarmAction(self):
        alarmAction = self.modbusRead(REG_ALARM_ACTION)
        self.ui.AlarmActionSelectBox.setCurrentIndex(alarmAction)
        if (alarmAction == 0):
            self.ui.AlarmActionField.setText("Stop")
        elif (alarmAction == 1):
            self.ui.AlarmActionField.setText("Notify")

#*******************************************************************************
    def onAlarmActionChange(self):
        newAlarmAction = self.ui.AlarmActionSelectBox.currentIndex()
        self.modbusWrite(REG_ALARM_ACTION_SET, newAlarmAction)
        self.updateAlarmAction()
#*******************************************************************************                            
app = QtGui.QApplication(sys.argv)
showWindow = appWindow()
showWindow.show()
app.exec_()




