from flask import Flask, render_template, request
from flask_cors import CORS  # if cross-origin requests are needed
import os
from datetime import datetime
import numpy as np
import io

app = Flask(__name__)
CORS(app)  # Enable CORS if needed

# Backend Global Variables
current_concentration = 0        # Latest metal concentration value
concentration_history = []       # History of concentration values for plotting vs time
time_history = []                # History of time values corresponding to the concentration
latest_voltage = []              # Voltage data from the latest CSV message
latest_current = []              # Current data (in µA) from the latest CSV message

# Interpolation line parameters
line1_slope = 0.03
line1_intercept = 1.0

# Serve the website (index.html must be in the "templates" folder)
@app.route('/')
def index():
    return render_template('index.html')

# Endpoint to return current sensor data as JSON
@app.route('/data', methods=['GET'])
def data():
    global current_concentration, concentration_history, time_history, latest_voltage, latest_current
    return {
        "current_concentration": current_concentration,
        "concentration_history": concentration_history,
        "time_history": time_history,
        "latest_voltage": latest_voltage,
        "latest_current": latest_current
    }

# Endpoint to receive CSV data from the ESP
@app.route('/upload', methods=['POST'])
def upload():
    global current_concentration, concentration_history, time_history, latest_voltage, latest_current

    csv_data = request.data.decode('utf-8')
    print("Received CSV data:")
    print(csv_data)

    # Parse CSV data (skip header row)
    data_arr = np.genfromtxt(io.StringIO(csv_data), delimiter=',', skip_header=1)

    # Data columns: [Index, Current_uA, Voltage_V, Time_ms]
    current_values = data_arr[:, 1]
    voltage_values = data_arr[:, 2]
    time_values = data_arr[:, 3]

    # Update latest voltage and current arrays
    latest_current = current_values.tolist()
    latest_voltage = voltage_values.tolist()

    # Find the top current value and its index (current is now in µA)
    max_index = np.argmax(current_values)
    top_current = current_values[max_index]
    print(f"Top current value (in µA): {top_current}")

    # Calculate the metal concentration using:
    # y (µA) = slope * concentration + intercept  =>  concentration = (y - intercept) / slope
    current_concentration = (top_current - line1_intercept) / line1_slope
    # Apply bounds: below 300 becomes 0; above 10,000 is capped at 10,000
    if current_concentration < 10:
        current_concentration = 0
    elif current_concentration > 10000:
        current_concentration = 10000
    print(f"Calculated metal concentration: {current_concentration}")

    # Use the current system time in HH:MM:SS format for logging
    measurement_time = datetime.now().strftime("%H:%M:%S")
    # Append new data to the history arrays
    concentration_history.append(current_concentration)
    time_history.append(measurement_time)

    # Ensure the output folder exists and write CSV data to a file.
    output_dir = "out"
    os.makedirs(output_dir, exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = os.path.join(output_dir, f"{timestamp}.csv")
    with open(filename, "w") as f:
        f.write(csv_data)
    print(f"Data saved to {filename}")

    return "Data received", 200

if __name__ == '__main__':
    # Run the Flask app on all interfaces at port 80.
    app.run(host='0.0.0.0', port=80)
