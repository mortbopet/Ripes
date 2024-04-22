from flask import Flask, render_template
from flask_cors import CORS, cross_origin

app = Flask(
    __name__,
    template_folder="../templates",
    static_url_path='',
    static_folder='../static',
    )
CORS(app)

@app.route("/", methods=["GET", "POST"])
@cross_origin()
def main_page():
    return render_template("ripes.html")

@app.route('/<task_id>/send/solution/', methods=['GET'])
def send_solution(task_id):
    user = check_auth()
    answer = int(request.args.get('answer', 0))
    solution_id = str(uuid4())
    add_solution(solution_id=solution_id, username=session['session_id'], task_id=task_id, score=answer/10,
        passback_params=user['tasks'].get(task_id)['passback_params'])

    return redirect(url_for('get_user_solution', solution_id=solution_id))

@app.route('/lti', methods=['POST'])
def lti_route():
    params = request.form
    consumer_secret = get_secret(params.get('oauth_consumer_key', ''))
    request_info = dict(
        headers=dict(request.headers),
        data=params,
        url=request.url,
        secret=consumer_secret
    )

    if check_request(request_info):
        # request is ok, let's start working!
        username = utils.get_username(params)
        custom_params = utils.get_custom_params(params)
        task_id = custom_params.get('task_id', 'default_task_id')
        role = utils.get_role(params)
        params_for_passback = utils.extract_passback_params(params)

        add_session(username, task_id, params_for_passback, role)
        session['session_id'] = username

        return redirect(url_for('index', task_id=task_id))
    else:
        abort(403)


app.run(host='0.0.0.0')
