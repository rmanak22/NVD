from flask import Flask, jsonify, send_from_directory
import time

app = Flask(__name__,
            static_folder='.',       # Serve files from current dir
            static_url_path='')      # at the root URL

@app.route('/')
def index():
    # Serve the index.html file from the current directory.
    return send_from_directory('.', 'index.html')

@app.route('/data')
def get_data():
    """
    First dataset (on first button press):
      - Reads data.txt
      - Hard-coded concentration = 2500
      - Single time stamp
      - Returns current/voltage arrays from data.txt
    """
    voltage = []
    current = []
    try:
        with open('data.txt', 'r') as f:
            lines = f.readlines()
        # Skip the first two lines (title and header)
        for line in lines[2:]:
            parts = line.strip().split()
            if len(parts) < 4:
                continue
            try:
                curr = float(parts[1])
                volt = float(parts[2])
                current.append(curr)
                voltage.append(volt)
            except ValueError:
                continue
    except Exception as e:
        print("Error reading data.txt:", e)

    # Hard-coded concentration value (ppb)
    concentration = 2532.46
    # Use the current time in milliseconds for the time history
    current_time_ms = int(round(time.time() * 1000))

    return jsonify({
        "current_concentration": concentration,
        "concentration_history": [concentration],  # array with single data point
        "time_history": [current_time_ms],         # array with single timestamp
        "latest_voltage": voltage,                 # entire data.txt
        "latest_current": current
    })

@app.route('/data2')
def get_data2():
    """
    Second dataset (on second button press):
      - Reads data2.txt
      - Hard-coded concentration = 500
      - Single time stamp
      - Returns current/voltage arrays from data2.txt
    """
    voltage = []
    current = []
    try:
        with open('data2.txt', 'r') as f:
            lines = f.readlines()
        # Skip the first two lines (title and header)
        for line in lines[2:]:
            parts = line.strip().split()
            if len(parts) < 4:
                continue
            try:
                curr = float(parts[1])
                volt = float(parts[2])
                current.append(curr)
                voltage.append(volt)
            except ValueError:
                continue
    except Exception as e:
        print("Error reading data2.txt:", e)

    # Hard-coded concentration value (ppb)
    concentration = 507.37
    # A new timestamp
    current_time_ms = int(round(time.time() * 1000))

    return jsonify({
        "current_concentration": concentration,
        "concentration_history": [concentration],  # single data point
        "time_history": [current_time_ms],         # single new timestamp
        "latest_voltage": voltage,                 # entire data2.txt
        "latest_current": current
    })

if __name__ == '__main__':
    app.run(debug=True)
