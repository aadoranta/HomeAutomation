import time
import requests
import threading
import urllib
import pytz

from flask import Flask, render_template, request
from multiprocessing import Array, Value
from datetime import datetime
from utils import *

# GLOBAL VARIABLES
TOGGLE_EVENT = threading.Event()
DELAY = 15
HTML_VARS = dict()
START_TIME = Array('c', b"19:00")
END_TIME = Array('c', b"09:00")
LENGTH = Value('i', 15)


app = Flask(__name__)


@app.route('/', methods=['GET'])
def index():

    global HTML_VARS
    HTML_VARS["light_toggle"] = "Turn Lamp Off"
    print(LENGTH.value)

    return render_template(
                            'index.html', 
                            light_toggle=HTML_VARS["light_toggle"],
                            start_time=START_TIME.value.decode(),
                            end_time=END_TIME.value.decode(),
                            interval_length=LENGTH.value
    )

"""
    Background functions (separate thread)
"""


def sensor_function(start_time_of_day, end_time_of_day, interval_length):

    # Communicates with the sensor and relay

    # Translate information received to information needing to be sent
    response_dictionary = {"1": "ON", "0": "OFF"}

    # Determine whether or not to actively sense motion based on various user input
    def toggle_condition(start_time, end_time):

        # Get current time
        current_timezone = pytz.timezone('America/New_York')
        current_time = datetime.now(current_timezone).time()

        # Get shared values of start and end time
        start_time = start_time.value.decode()
        end_time = end_time.value.decode()

        # Boolean to see if user has turned on light
        lamp_toggle_button = TOGGLE_EVENT.is_set()

        # Boolean to determine if we are in correct times of day to perform motion sensing
        within_timeframe = True if current_time >= datetime.strptime(start_time, "%H:%M").time() \
                                    and current_time < datetime.strptime(end_time, "%H:%M").time() \
                                else False

        return lamp_toggle_button and within_timeframe

    while True:

        if toggle_condition(start_time_of_day, end_time_of_day):
            # Make the GET request to the ESP8266 module
            sensor_response = get_request(url=f'http://192.168.86.39/', uri='get-motion')

            # Get the Sensor feedback
            if sensor_response.status_code == 200:
                motion_flag = response_dictionary[sensor_response.text]

            if motion_flag == "ON":
                
                # Set initial time at which motion was detected
                init_time = time.time()

                while time.time() < init_time + interval_length.value*60 + 1:

                    # Create the JSON payload
                    payload = {"message": motion_flag}

                    # Send the ON message to the relay
                    relay_response = post_request(url=f'http://192.168.86.32/', uri='motion', payload=payload)

                    # Check for motion again
                    sensor_response = get_request(url=f'http://192.168.86.39/', uri='get-motion')

                    if sensor_response.status_code == 200:
                        inner_motion_check = response_dictionary[sensor_response.text]
                    
                    # Reset the initial time if motion is sensed
                    if inner_motion_check == "ON":
                        init_time = time.time()

                    # If the user has turned off the lamp, exit the thread function
                    if not toggle_condition(start_time_of_day, end_time_of_day):
                        break
                    
                    time.sleep(0.5)
            
            else:

                # Turn off the lamp if no motion detected
                payload = {"message": "OFF"}
                relay_response = post_request(url=f'http://192.168.86.32/', uri='motion', payload=payload)

        else:
            
            # Turn off the lamp if user selects "Turn off Lamp"
            payload = {"message": "OFF"}
            relay_response = post_request(url=f'http://192.168.86.32/', uri='motion', payload=payload)
            
        time.sleep(0.5)


"""
    Functions for user interaction
"""

@app.route('/toggle_lamp', methods=['GET', 'POST'])
def toggle_lamp():

    # Turn off and on lamp depending on user input
    # Changes a global variable that is read by the motion-sensing thread

    global HTML_VARS

    if request.method == 'POST':
        current_state = HTML_VARS["light_toggle"]

        if current_state == "Turn Lamp Off":
            TOGGLE_EVENT.clear()    
            HTML_VARS["light_toggle"] = "Turn Lamp On"
        else:
            TOGGLE_EVENT.set()
            HTML_VARS["light_toggle"] = "Turn Lamp Off"

        return render_template(
                                'index.html', 
                                light_toggle=HTML_VARS["light_toggle"],
                                start_time=START_TIME.value.decode(), 
                                end_time=END_TIME.value.decode(),
                                interval_length=LENGTH.value
                                )

    return render_template('index.html',
                            start_time=START_TIME.value.decode(), 
                            end_time=END_TIME.value.decode(),
                            interval_length=LENGTH.value
                            )   

@app.route('/update_time', methods=['POST'])  
def update_time():

    # Reads a start and end time for the lamp to detect motion
    # Updates a global variable (TIMES OF DAY) that is visible to the motion-sensing thread

    global START_TIME
    global END_TIME

    # Get the times of day from user
    START_TIME.value = request.form.get('start-time').encode()
    END_TIME.value = request.form.get('end-time').encode()

    # Get interval length from user input
    try:
        LENGTH.value = int(request.form.get('interval-length'))
    except ValueError:
        LENGTH.value = 15

    # Might want to attach date to the times. Use a utils function

    return render_template(
                            'index.html', 
                            light_toggle=HTML_VARS["light_toggle"],
                            start_time=START_TIME.value.decode(), 
                            end_time=END_TIME.value.decode(),
                            interval_length=LENGTH.value
                            )

# Threading actions
TOGGLE_EVENT.set()
sensor_thread = threading.Thread(target=sensor_function, args=(START_TIME, END_TIME, LENGTH))
sensor_thread.daemon = True
sensor_thread.start()

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
