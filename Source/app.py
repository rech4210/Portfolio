from flask import Flask, request

app = Flask(__name__)

@app.route("/api/skill-log", methods=["POST"])
def log_skill():
    data = request.get_json()
    print("Log:", data, flush=True)
    return "OK"
