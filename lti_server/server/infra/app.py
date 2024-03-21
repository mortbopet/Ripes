from flask import Flask, render_template

app = Flask(__name__, template_folder="../templates")


@app.route("/", methods=["GET"])
def main_page():
    return render_template("main_page.html")


@app.route("/task/<task_id>", methods=["GET"])
def task(task_id):
    return render_template("task.html", task_id=task_id)


@app.route("/user/<user_id>", methods=["GET"])
def user_summary(user_id):
    return render_template("summary.html", user_id=user_id)
