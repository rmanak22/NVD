from flask import Flask, jsonify, send_from_directory
import time

# Instruct Flask to serve from the current directory (.)
# at the root URL path (i.e., no /static prefix)
app = Flask(__name__,
            static_folder='.',       # Serve from current dir
            static_url_path='')      # at the root URL

@app.route('/')
def index():
    # Serve the index.html file from the current directory.
    return send_from_directory('.', 'index.html')

@app.route('/data')
def get_data():
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
    concentration = 2500
    # Use the current time in milliseconds for the time history
    current_time_ms = int(round(time.time() * 1000))

    return jsonify({
        "current_concentration": concentration,
        "concentration_history": [concentration],
        "time_history": [current_time_ms],
        "latest_voltage": voltage,
        "latest_current": current
    })

if __name__ == '__main__':
    app.run(debug=True)
