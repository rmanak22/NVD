from flask import Flask, request
import os
from datetime import datetime

app = Flask(__name__)

@app.route('/upload', methods=['POST'])
def upload():
    csv_data = request.data.decode('utf-8')
    print("Received CSV data:")
    print(csv_data)

    # Ensure the output folder exists
    output_dir = "out"
    os.makedirs(output_dir, exist_ok=True)

    # Generate a filename based on the current timestamp.
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = os.path.join(output_dir, f"{timestamp}.csv")

    # Write the CSV data to the file.
    with open(filename, "w") as f:
        f.write(csv_data)

    print(f"Data saved to {filename}")
    return "Data received", 200

if __name__ == '__main__':
    # Run the Flask webserver on all interfaces at port 80.
    app.run(host='0.0.0.0', port=80)
