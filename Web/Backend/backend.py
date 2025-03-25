from flask import Flask, request
from flask_cors import CORS  # Import Flask-CORS
import os
from datetime import datetime
import numpy as np
import io

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

# Global variables
current_concentration = 0        # Latest metal concentration value
concentration_history = []       # History of concentration values for plotting vs time
time_history = []                # History of time values corresponding to the concentration
latest_voltage = []              # Voltage data from the latest CSV message
latest_current = []              # Current data from the latest CSV message

# Interpolation line parameters
line1_slope = 10.268
line1_intercept = 1.1028

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

@app.route('/upload', methods=['POST'])
def upload():
    global current_concentration, concentration_history, time_history, latest_voltage, latest_current

    csv_data = request.data.decode('utf-8')
    print("Received CSV data:")
    print(csv_data)

    # Parse CSV data (skip header row)
    data = np.genfromtxt(io.StringIO(csv_data), delimiter=',', skip_header=1)

    # Data columns: [Index, Current_Amps, Voltage_V, Time_ms]
    current_values = data[:, 1]
    voltage_values = data[:, 2]
    time_values = data[:, 3]

    # Update latest voltage and current arrays
    latest_current = current_values.tolist()
    latest_voltage = voltage_values.tolist()

    # Find the top current value and its index
    max_index = np.argmax(current_values)
    top_current = current_values[max_index]
    print(f"Top current value (in Amps): {top_current}")

    # Convert current from Amps to microamps (µA)
    top_current_microamps = top_current * 1e6
    print(f"Top current value (in µA): {top_current_microamps}")

    # Calculate the metal concentration using:
    # y (µA) = slope * concentration + intercept  =>  concentration = (y - intercept) / slope
    current_concentration = (top_current_microamps - line1_intercept) / line1_slope
    print(f"Calculated metal concentration: {current_concentration}")

    # Use the time value corresponding to the max current measurement.
    measurement_time = time_values[max_index]
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
    # Run the Flask webserver on all interfaces at port 80.
    app.run(host='0.0.0.0', port=80)
