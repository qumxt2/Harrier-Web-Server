'''
Background worker to watch for messages from the MQTT server and 
insert/update the message payloads in the Graco pump web app's
MySQL database.

The reason this was created was due to the latency involved in
connecting to the MQTT server on every web request and the
in-elegance of doing it in a Django-tied background worker
process.  

Created on Sep 23, 2014

@author: Jeff Keacher
'''
import paho.mqtt.client as pahoClient
import mysql.connector
import re
import os
from time import sleep
import datetime
from version import GetVersion
import logging

# Global constants
MQTT_BROKER = 'localhost'
MQTT_USER = 'gracoweb'
MQTT_PASS = 'g9UdjawKEvZ7yUAN'

LOG_FILE_NAME = 'mqtt_db_worker.log'

ORIGIN_PUMP = 1
ORIGIN_SERVER = 2
ORIGIN_MQTT = 4
TARGET_PUMP = 1
EVENT_DEBUG = 4
ACTION_CREATE = 1
ACTION_UPDATE = 3
ACTION_NA = 6
VERSION_2 = 2

IS_ACTIVE_COLUMN = 'is_active'

DATABASES = {
    'default': {
        'NAME': 'gracopump',
        'ENGINE': 'mysql.connector.django',
        'USER': 'gracoweb',
        'PASSWORD': 'lKJSDfkljhkwjQ$2',
        'OPTIONS': {
            'autocommit': True,
        },
    },
}

PumpTopics = {  # Topic name in MQTT : column name in database
    'PumpStatus': 'status',
    'FlowRate': 'flow_rate',
    'Totalizer': 'totalizer_resetable',
    'GrandTotalizer': 'totalizer_grand',
    'AlarmStatus': 'alarms_status',
    'PumpTopology': 'pump_topology',
    'ActiveClients': 'connection',
    'MeteringMode': 'metering_mode',
    'OnTime': 'metering_on_time',
    'OffTime': 'metering_off_time',
    'OnCycles': 'metering_on_cycles',
    'PumpOnTimeout': 'metering_on_timeout',
    'SoftwareVersion': 'firmware_version',
    'Location': 'location_reported',
    'PressureLevel': 'pressure_level',
    'BatteryVoltage': 'battery_voltage',
    'HighPressureTrigger': 'high_pressure_trigger',
    'LowPressureTrigger': 'low_pressure_trigger',
    'LowBatteryTrigger': 'low_battery_trigger',
    'BatteryWarningTrigger': 'battery_warning_trigger',
    'SignalStrength': 'signal_strength',
    'SystemPublicationPeriod': 'system_publication_period',
    'PowerSaveMode': 'power_save_mode',
    'TankLevelNotifyTrigger': 'tank_level_notify_trigger',
    'TankLevelShutoffTrigger': 'tank_level_shutoff_trigger',
    'TankLevel': 'tank_level',    
    'SensorType': 'sensor_type',
    'TankType': 'tank_type',
    'TankLevelVolumeMax': 'tank_level_volume_max',
    'FlowVerifyPercentage': 'flow_verify_percentage',    
    'FlowVerifyEnable': 'flow_verify_enable',   
    'AnalogInputMode': 'analog_input_mode',
    'RawAnalogIn': 'raw_analog_in',
    'Temperature': 'temperature',
    'TemperatureControl': 'temperature_control',
    'TemperatureSetpoint': 'temperature_setpoint',
    'MotorProtection': 'motor_protection_settings',
    'MotorCurrentMv': 'motor_current_mV',
    'MultiwellEnable': 'multiwell_enable',
    'W1Rate': 'well_flow_rate_1',
    'W2Rate': 'well_flow_rate_2',
    'W3Rate': 'well_flow_rate_3',
    'W4Rate': 'well_flow_rate_4',
    'W5Rate': 'well_flow_rate_5',
    'W6Rate': 'well_flow_rate_6',
    'W7Rate': 'well_flow_rate_7',
    'W8Rate': 'well_flow_rate_8',
    'W1Total': 'totalizer_well_1',
    'W2Total': 'totalizer_well_2',
    'W3Total': 'totalizer_well_3',
    'W4Total': 'totalizer_well_4',
    'W5Total': 'totalizer_well_5',
    'W6Total': 'totalizer_well_6',
    'W7Total': 'totalizer_well_7',
    'W8Total': 'totalizer_well_8',
    'W1GTotal': 'totalizer_grand_well_1',
    'W2GTotal': 'totalizer_grand_well_2',
    'W3GTotal': 'totalizer_grand_well_3',
    'W4GTotal': 'totalizer_grand_well_4',
    'W5GTotal': 'totalizer_grand_well_5',
    'W6GTotal': 'totalizer_grand_well_6',
    'W7GTotal': 'totalizer_grand_well_7',
    'W8GTotal': 'totalizer_grand_well_8',
    'WellStatus': 'well_status',
    'AinmALow': 'ain_mA_low',
    'AinmAHigh': 'ain_mA_high',
    'AinFlowRateLow': 'ain_flow_rate_low',
    'AinFlowRateHigh': 'ain_flow_rate_high',
}

DebugTopics = ['DebugEvent',
               ]


REGEX_FOR_CLEAN = r'[^-a-zA-Z_0-9)(:\., #/]'


def main():

    log_level = logging.DEBUG
    home_path = os.path.expanduser('~')
    log_file_path = os.path.join(home_path, 'logs', LOG_FILE_NAME)
    log_format = '%(asctime)s:%(levelname)s:\t===%(message)s'
    logging.basicConfig(filename=log_file_path, level=log_level, format=log_format)
    logging.info('Graco MQTT/DB bridge worker version %s starting' % GetVersion())

    # Connect to the database
    db = Database()
    db.connect()

    # Connect to the MQTT server
    mqtt = PumpWatch(db)

    # Subscribe to ActiveClients to keep
    mqtt.subscribe_and_watch(PumpTopics.keys())
    mqtt.subscribe_and_watch(DebugTopics)

    # Pump MQTT loop
    while True:
        mqtt.loop_forever()


class Database(object):

    def __init__(self):
        self.connection = None
        self.allow_log_writes = False

    def connect(self):
        self.connection = mysql.connector.connect(user=DATABASES['default']['USER'],
                                                  password=DATABASES['default']['PASSWORD'],
                                                  host='127.0.0.1',
                                                  database=DATABASES['default']['NAME'],
                                                  )

        self.write_event_log('', EVENT_DEBUG, 'MQTT/db worker v%s started' % GetVersion(), ORIGIN_MQTT, action=ACTION_NA)

    def write(self, pump_id, topic, value):
        '''Update the database entry for the given pump with the given value for the given topic (topic form)'''

        # Sanitize inputs
        pump_id = re.sub(REGEX_FOR_CLEAN, '', pump_id)
        value = re.sub(REGEX_FOR_CLEAN, '', str(value))

        # If pump isn't in the database, create it
        if not self.pump_exists(pump_id):
            logging.warn('Message for unrecognized pump received: %s' % (str(pump_id),))
            self.db.write_event_log('', EVENT_DEBUG, 'Unrecognized pump: %s' % (str(pump_id),), ORIGIN_MQTT, action=ACTION_NA, success=False)
            return False

        column_name = PumpTopics[topic]

        old_value = self.get_current_value(pump_id, column_name)

        # Update the given active topic for the pump and its history
        self.update_attribute(pump_id, column_name, value)
        self.write_history_log(pump_id, column_name, value)

        self.generate_alarm_alert(pump_id, column_name, value, old_value)
        self.log_connection(pump_id, column_name, value, old_value)
        
        if column_name == 'firmware_version':
                self.write_event_log(pump_id, EVENT_DEBUG, 'Power Cycle', origin_type=ORIGIN_MQTT)

        return True

    def check_for_active(self, pump_id):
        '''Make sure that a connected pump is marked as active. This will "reanimate" a pump that has been deleted.'''
        is_active = self.get_current_value(pump_id, IS_ACTIVE_COLUMN)

        if is_active != 1 and is_active != '1':
            self.update_attribute(pump_id, IS_ACTIVE_COLUMN, '1')

    def get_pump_real_id(self, unique_id):
        '''Get a pump's database ID given its visible unique ID'''
        pump_real_id = None
        cursor = self.connection.cursor()

        # Get the actual pump ID
        query = ("SELECT id FROM gracopumpapp_pump WHERE unique_id = %s")
        cursor.execute(query, (unique_id,))
        result_row = cursor.fetchone()
        if result_row:
            pump_real_id = result_row[0]

        return pump_real_id

    def create_alarm_work_items(self, unique_id, pump_real_id, new_alarms):
        '''Create alarm work items if needed'''
        max_alarm_number = 32
        cursor = self.connection.cursor()

        # Create work items for each alarm bit set
        for alarm_id in range(0, max_alarm_number):
            if new_alarms & (1 << alarm_id) != 0:
                # check for existing identical uncompleted work item
                existing_work = False
                query = ("SELECT count(id) as existing_work FROM gracopumpapp_alarmwork WHERE pump_id = %s and alarm_id = %s and done = %s")
                cursor.execute(query, (pump_real_id, alarm_id, 0))
                result_row = cursor.fetchone()
                if result_row and result_row[0] > 0:
                    logging.debug("Found existing alarm work for alarm %u and pump %s; skipping." % (alarm_id, unique_id))
                    existing_work = True

                # Create one if needed
                if not existing_work:
                    logging.debug("Creating alarm work for alarm %u and pump %s" % (alarm_id, unique_id))
                    query = ("INSERT INTO gracopumpapp_alarmwork (pump_id, alarm_id, done, created_at) "
                             "VALUES (%s, %s, %s, %s)")
                    now = datetime.datetime.utcnow()
                    cursor.execute(query, (pump_real_id, alarm_id, 0, now))
                    self.connection.commit()

    def generate_alarm_alert(self, pump_id, column_name, value, old_value):
        '''Generate alart alert work items if needed'''
        pump_real_id = None

        if column_name == 'alarms_status':
            value_int = int(value)
            old_value_int = int(old_value)

            # See if any new alarm bits have been set
            new_alarms = (value_int ^ old_value_int) & value_int
            if new_alarms != 0:
                pump_real_id = self.get_pump_real_id(pump_id)

                if pump_real_id:
                    self.create_alarm_work_items(pump_id, pump_real_id, new_alarms)

    def log_connection(self, pump_id, column_name, value, old_value):
        '''Connection event logging'''
        if column_name == 'connection':
            if int(value) == 0:
                self.write_event_log(pump_id, EVENT_DEBUG, 'Pump is disconnected', origin_type=ORIGIN_MQTT)
            elif int(value) == 1:
                self.write_event_log(pump_id, EVENT_DEBUG, 'Pump is connected', origin_type=ORIGIN_MQTT)
                self.check_for_active(pump_id)

            # Ensure that we write the last_seen value the first time there is a connection
            if old_value is None or int(old_value) != int(value):
                self.update_attribute(pump_id, 'last_seen', datetime.datetime.now())

            # For disconnection notifications
            if old_value is not None and int(old_value) == 1 and int(value) == 0:
                self.update_attribute(pump_id, 'disconnection_noticed', '0')

    def write_event_log(self, pump_id, event_type, payload, origin_type=ORIGIN_MQTT, action=ACTION_UPDATE, success=True):
        '''Write an entry to the event log'''

        # Sanitize inputs
        regex_for_identifier = r'[^a-zA-Z_0-9\.]'
        regex_for_payload = r'[^-a-zA-Z_0-9\.#,? :\s=]'
        pump_id = re.sub(regex_for_identifier, '', pump_id)
        payload = re.sub(regex_for_payload, '', str(payload))
        event_type = int(event_type)

        if len(pump_id) > 0:
            target_type = TARGET_PUMP
        else:
            target_type = ''

        status = 1 if success else 0

        timestamp = datetime.datetime.now()

        cursor = self.connection.cursor()
        query = ("INSERT INTO gracopumpapp_log (target_type, target_id, origin_type, event_type, timestamp, message, entry_format, action, success) "
                 "VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)")

        logging.debug('Added event to event log')
        cursor.execute(query, (target_type, pump_id, origin_type, event_type, timestamp, payload, VERSION_2, action, status))
        self.connection.commit()

    def write_history_log(self, pump_id, column_name, value):
        '''Add to the immutable history log'''
        cursor = self.connection.cursor()
        timestamp = datetime.datetime.now()
        pump_id = re.sub(REGEX_FOR_CLEAN, '', pump_id)
        value = re.sub(REGEX_FOR_CLEAN, '', value)
        column_name = re.sub(REGEX_FOR_CLEAN, '', column_name)
        query = ("INSERT INTO gracopumpapp_history (pump_id, timestamp, attribute, value) "
                 "VALUES (%s, %s, %s, %s)")

        logging.debug("Added to history log")
        cursor.execute(query, (pump_id, timestamp, column_name, value))
        self.connection.commit()

    def update_attribute(self, pump_id, column_name, value):
        '''Assumes inputs have been sanitized and the given pump row exists'''
        cursor = self.connection.cursor()

        column_name = re.sub(REGEX_FOR_CLEAN, '', column_name)
        if not isinstance(value, datetime.datetime):
            value = re.sub(REGEX_FOR_CLEAN, '', value)

        query = "UPDATE gracopumpapp_pump SET {0}=%s, timestamp=%s WHERE unique_id=%s LIMIT 1".format(column_name)
        timestamp = datetime.datetime.now()

        logging.debug("pump_id = %s, column = %s, value = %s" % (pump_id, column_name, value))
        cursor.execute(query, (value, timestamp, pump_id))
        self.connection.commit()

    def pump_exists(self, pump_id):
        '''
        Check if the pump exists. If not, raise an error. All pumps must be created during their
        initial connection to the web site, which is when their credentials are generated and their
        ACL rights are configured.
        '''
        found_pump = False

        pump_id = re.sub(REGEX_FOR_CLEAN, '', pump_id)

        cursor = self.connection.cursor()
        query = "SELECT COUNT(unique_id) AS num_pumps FROM gracopumpapp_pump WHERE unique_id = %s"

        cursor.execute(query, (pump_id,))
        count_row = cursor.fetchone()

        if count_row:
            if count_row[0] == 1:
                found_pump = True

        return found_pump

    def get_default_customer_id(self):
        '''Get the default customer number'''
        cursor = self.connection.cursor()
        query = "SELECT id from gracopumpapp_customer WHERE organization_name = 'None'"
        cursor.execute(query)
        org_number = cursor.fetchone()

        return org_number[0]

    def get_current_value(self, pump_id, column_name):
        '''Get the current value for the given field for the given pump'''

        column_name = re.sub(REGEX_FOR_CLEAN, '', column_name)
        pump_id = re.sub(REGEX_FOR_CLEAN, '', pump_id)

        cursor = self.connection.cursor()
        query = "SELECT {0} from gracopumpapp_pump WHERE unique_id = %s LIMIT 1".format(column_name)
        cursor.execute(query, (pump_id,))
        old_value = cursor.fetchone()

        if old_value and len(old_value) > 0:
            retVal = old_value[0]
        else:
            retVal = None

        return retVal


class PumpWatch(pahoClient.Client):
    '''Class for reading from the pump broker and sending updates to the database'''
    EVENT_DEBUG = 4

    def __init__(self, db):
        super(PumpWatch, self).__init__()

        self.username_pw_set(MQTT_USER, MQTT_PASS)
        self.connect(MQTT_BROKER)

        self.db = db

    def subscribe_and_watch(self, topic_list):
        from time import sleep
        for topic in topic_list:
            sub = "%s/+" % (topic,)
            self.message_callback_add(sub, callback=self.on_any_message)
            self.subscribe(sub)
            logging.info('Subscribed to %s' % re.sub(REGEX_FOR_CLEAN, '', topic))
            sleep(0.1)
            for _ in range(0,10):
                self.loop(0.01)

    def on_any_message(self, client, userdata, message):
        '''Parse the message and send it to the database'''
        try:
            self.parse_and_send(message)
        except Exception as e:
            logging.warn('Exception in on_message callback: %s' % (repr(e),))
            self.db.write_event_log('', EVENT_DEBUG, 'Exception in on_any_message(): %s' % repr(e), ORIGIN_MQTT, action=ACTION_NA, success=False)

    def parse_and_send(self, message):
        '''Figure out which topic, pump, and value we need to update, then send it to the db'''
        logging.debug("*Got message with topic %s and payload %s" % (message.topic, repr(message.payload)))
        parts = message.topic.partition('/')
        topic = re.sub('[^a-zA-Z0-9_]', '', parts[0])
        pump_id = re.sub('[^0-9]', '', parts[2])
        if type(message.payload) == bytes:
            probable_str = message.payload.partition(b'\0')[0]

            # Strip out non-printable garbage
            probable_stripped_str = ''.join([chr(i) if i >= 32 and i < 128 else '' for i in probable_str])
            value = probable_stripped_str

#            value = probable_stripped_str.decode('utf-8')
        else:
            value = message.payload

        if pump_id and len(pump_id) >= 8 and len(pump_id) <= 24:
            if str(topic) in PumpTopics:
                self.db.write(pump_id, topic, value)
            elif str(topic) in DebugTopics:
                self.db.write_event_log(pump_id, self.EVENT_DEBUG, value)
            else:
                logging.warn('Message for unrecognized topic received')
                self.db.write_event_log('', EVENT_DEBUG, 'Unrecognized message: %s %s %s' % (str(topic), str(pump_id), value), ORIGIN_MQTT, action=ACTION_NA, success=False)
        else:
            try:
                self.db.write_event_log('', EVENT_DEBUG, 'Invalid message: %s %s %s' % (str(topic), str(pump_id), repr(value)), ORIGIN_MQTT, action=ACTION_NA, success=False)
            except Exception as e:
                logging.warn('***Error while logging: %s' % repr(e))

if __name__ == '__main__':
    main()
