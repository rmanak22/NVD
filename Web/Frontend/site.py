from flask import Flask, render_template
import webbrowser
import threading
import time

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

def open_browser():
    # Wait a moment for the server to start
    time.sleep(1)
    webbrowser.open("http://127.0.0.1:8000/")

if __name__ == '__main__':
    # Start a thread that opens the browser
    threading.Thread(target=open_browser).start()
    # Run the frontend Flask app on port 8000
    app.run(host='0.0.0.0', port=8000)
