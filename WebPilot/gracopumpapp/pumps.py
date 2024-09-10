'''
Handle pump communication
'''
from GracoPump import settings
import paho.mqtt.publish as pahoPublish


# Global constants
MQTT_BROKER = 'localhost'
MQTT_USER = 'gracoweb'
MQTT_PASS = 'g9UdjawKEvZ7yUAN'

PumpCommands = ['SetPumpStatus',
                'ResetTotalizer',
                'ClearAlarmStatus',
                'SetFlowRate',
                'SetOnTime',
                'SetOffTime',
                'SetOnCycles',
                'SetPumpOnTimeout',
                'SetHighPressureTrigger',
                'SetLowPressureTrigger',
                'SetLowBatteryTrigger',
                'SetBatteryWarningTrigger',
                'SetSystemPublicationPeriod',
                'ActivationKey',
                'SetPumpName',
                'SetTankLevelNotifyTrigger',
                'SetTankLevelShutoffTrigger',                
                'SetFlowVerifyPercentage',
                'SetTemperatureControl',
                'SetTemperatureSetpoint',
                'W1SetRate',
                'W2SetRate',
                'W3SetRate',
                'W4SetRate',
                'W5SetRate',
                'W6SetRate',
                'W7SetRate',
                'W8SetRate',
                'W1ResetTot',
                'W2ResetTot',
                'W3ResetTot',
                'W4ResetTot',
                'W5ResetTot',
                'W6ResetTot',
                'W7ResetTot',
                'W8ResetTot',
                'SetWellStatus',
                'SetAnalogInputMode', 
                'SetAinmALow',
                'SetAinmAHigh',
                'SetAinFlowRateLow',
                'SetAinFlowRateHigh',
                ]

PrettyPump = {'connection': ['Not connected', 'Connected'],
              }


AlarmList = ['Relay Ctrl Mode',  # bit 0
             'Low tank shutoff',
             'Low tank notify',
             'Analog In',
             'Modbus Comms',
             'Flow Accuracy',
             'Count not achieved',
             'Input 1',
             'Input 2',
             'Temperature',
             'Low battery',
             'Disabled by remote',
             'High pressure',
             'Low pressure',
             'Over current',
             'Solenoid 1',
             'Solenoid 2',
             'Solenoid 3',
             'Solenoid 4',
             'Solenoid 5',
             'Solenoid 6',
             'Solenoid 7',
             'Solenoid 8',
             ]

# The alarms that must be manually cleared
# Tuple uses indexes in the AlarmList list corresponding to bit number
# This is NOT the same as the alarm IDs used in the firmware!
AlarmsManuallyCleared = (1, 3, 5, 6, 7, 8, 12, 13)


# Input bounds (low, high)
Bounds = {'FlowRate': (0.01, 600.0),  # gallons/day
          'Time': (0, 100 * 60 * 60),  # seconds
          'Cycles': (0, 1000000),
          'Status': (0, 1),
          'Pressure': (0, 7500),  # PSI
          'Battery': (0, 30),  # volts
          'Percent': (0, 100), # %
          'TankVolume': (0, 9999), # gallons
          'Temperature': (-40, 118), # deg Fahrenheit
          'TempControl': (0, 3),
          'AinMode': (0, 1),
          'mA': (4, 20), # mA
          }


def alarm_bitfield_to_list(bitfield):
    '''Translate an alarm bitfield into a list of active alarms'''
    active_alarms = []
    bitfield_as_int = int(bitfield)

    for bit in range(0, len(AlarmList)):
        if (1 << bit) & bitfield_as_int:
            active_alarms.append(bit)

    return active_alarms


class PumpSet(object):
    '''Class for controlling the pumps'''

    @staticmethod
    def set_pump(pump_id, topic, value, retain=False):
        '''Set the given topic about the given pump to the given value.
        Assumes that value is of the proper type for the given aspect.'''

        if topic not in PumpCommands:
            raise ValueError('Invalid pump command')

        if len(pump_id) < 1:
            raise ValueError('Invalid pump ID')

        topic_full = '%s/%s' % (topic, pump_id)
        payload = str(value)
        auth = {'username': MQTT_USER, 'password': MQTT_PASS}
        if settings.DEBUG:
            print('FAKE MQTT FOR DEBUG ###\n\tTopic: %s\n\tPayload: %s' % (topic_full, payload))
        else:
            pahoPublish.single(topic_full, payload=payload, retain=retain, qos=2, hostname=MQTT_BROKER, auth=auth)

    @staticmethod
    def disconnect(pumps):
        '''
        Disconnect all pumps in the given list by connecting as each known pump's ID and writing "0" to the ID's ActiveClients entry.

        This can be useful after restarting mosquitto in case some pumps appear to be stuck or unresponsive.

        It works because mosquitto will disconnect older connections using the same ID as a new connection. The write
        to ActiveClients ensures that ActiveClients/+ stays in sync with reality.
        '''
        success = True

        try:
            auth = {'username': MQTT_USER, 'password': MQTT_PASS}
            payload = str(0)
            retain = True

            # For each pump, connect as that pump's ID and write '0' to ActiveClients/<pump_id>
            for pump in pumps:
                pump_id = pump.unique_id
                topic = 'ActiveClients/%s' % (pump_id,)
                pahoPublish.single(topic, payload=payload, client_id=pump_id, retain=retain, qos=2, hostname=MQTT_BROKER, auth=auth)

        except:
            success = False

        return success

