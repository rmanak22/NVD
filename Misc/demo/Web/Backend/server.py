from flask import Flask, request, jsonify
from flask_cors import CORS
import time

app = Flask(__name__)

# Configure CORS to allow your frontend's origin
cors = CORS(app, resources={r"/api/*": {"origins": "http://localhost:5174"}})

# Store water level data
data = []

@app.route('/api/water-level', methods=['POST'])
def update_water_level():
    req_data = request.json
    if "level" not in req_data:
        return jsonify({"error": "Missing 'level' field in request"}), 400

    current_time = time.strftime("%I:%M %p")
    new_entry = {"timestamp": current_time, "waterLevel": req_data["level"]}
    data.append(new_entry)

    return jsonify({"message": "Water level updated successfully", "entry": new_entry}), 200

@app.route("/api/water-level", methods=["GET"])
def get_water_level():
    return jsonify(data)

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=3000)
