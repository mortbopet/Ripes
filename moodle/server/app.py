from datetime import datetime
import logging
import os
import uuid
import json
from uuid import uuid4

import requests
from dotenv import load_dotenv
from flask import Flask, render_template, request, redirect, url_for, Response, jsonify
from oauthlib.oauth1 import Client
from peewee import *
from requests.exceptions import MissingSchema, ConnectionError
from werkzeug.exceptions import HTTPException

from tester.all_tasks import get_task_by_id

load_dotenv()
app = Flask(
    __name__,
    template_folder="templates",
    static_folder="static"
)
app.logger.setLevel(logging.INFO)

db = PostgresqlDatabase(
    database=os.getenv('POSTGRES_DB'),
    user=os.getenv('POSTGRES_USER'),
    password=os.getenv('POSTGRES_PASSWORD'),
    host=os.getenv('POSTGRES_HOST'),
    port=int(os.getenv('POSTGRES_PORT'))
)


class BaseModel(Model):
    class Meta:
        database = db


class ConnectionMeta(BaseModel):
    session_id = UUIDField(primary_key=True)
    task_id = CharField(max_length=256)
    course_title = CharField(max_length=512)
    user_id = BigIntegerField()
    full_name = CharField(max_length=1024)
    email = CharField(max_length=256)
    roles = CharField(max_length=512)
    outcome_service_url = CharField(max_length=1024)
    sourced_id = CharField(max_length=256)

    class DoesNotExist(DoesNotExist):
        pass

    class Meta:
        table_name = 'connection_meta'


class Events(BaseModel):
    class EventType:
        SESSION_OPENED = 'SESSION_OPENED'
        GRADE_SENT = 'GRADE_SENT'

    event_id = BigAutoField(primary_key=True)
    session_id = ForeignKeyField(ConnectionMeta, field='session_id', backref='events', on_delete='CASCADE')
    event_type = CharField(choices=[
        EventType.SESSION_OPENED,
        EventType.GRADE_SENT
    ])
    event_timestamp = DateTimeField()

    class Meta:
        table_name = 'events'


class Grades(BaseModel):
    grade_id = BigAutoField(primary_key=True)
    event_id = ForeignKeyField(Events, field='event_id', backref='grades', on_delete='CASCADE')
    grade_value = FloatField()
    code = CharField(4096)

    class Meta:
        table_name = 'grades'


@app.route('/lti', methods=['POST'])
def lti_request() -> str | Response:
    """
    A method that processes a request from Moodle.
    Retrieves user data from the request and prints it to the console.

    :return: A template rendering the page with Ripes.
    """
    if request.method != 'POST':
        return render_error("Bad Request")

    lti_data = request.form

    user_id = lti_data.get('user_id', 'No user ID')
    full_name = lti_data.get('lis_person_name_full', 'Unknown user')
    email = lti_data.get('lis_person_contact_email_primary', 'No email')
    course_title = lti_data.get('context_title', 'No course title')
    roles = lti_data.get('roles', 'No user roles')
    task_id = lti_data.get('custom_task_id', '0')

    session_id = uuid4()

    app.logger.info(f"Session ID: {session_id}")
    app.logger.info(f"Task ID: {task_id}")
    app.logger.info(f"User ID: {user_id}")
    app.logger.info(f"Full Name: {full_name}")
    app.logger.info(f"Email: {email}")
    app.logger.info(f"Course Title: {course_title}")
    app.logger.info(f"Roles: {roles}")

    lis_outcome_service_url = lti_data.get('lis_outcome_service_url')
    lis_result_sourcedid = lti_data.get('lis_result_sourcedid')

    with db.connection_context():
        ConnectionMeta.create(session_id=session_id, task_id=task_id, course_title=course_title, user_id=int(user_id),
                              full_name=full_name, email=email, roles=roles,
                              outcome_service_url=lis_outcome_service_url, sourced_id=lis_result_sourcedid)

        Events.create(session_id=session_id, event_type=Events.EventType.SESSION_OPENED, event_timestamp=datetime.now())

    return redirect(url_for("main_page", session_id=session_id))


@app.route('/ripes/<session_id_str>/<grade_str>/<code>', methods=["POST"])
def send_grade_to_moodle(session_id_str: str, grade_str: str, code: str) -> tuple[str, int] | Response:
    """
    Method for sending a grade to Moodle.
    Uses session ID to get a specific connection ID to set a grade for a correct student.

    :param session_id_str: String containing session ID.
    :param grade_str: String containing grade.
    :return: An error message if something went wrong, else a template rendering the page with Ripes.
    """
    if request.method != 'POST':
        return render_error("Bad Request"), 400

    try:
        session_id = uuid.UUID(session_id_str)
    except ValueError:
        err_message = f"invalid uuid: {session_id_str}"
        app.logger.info(err_message)
        return render_error(err_message), 400

    try:
        float(grade_str)
    except ValueError:
        err_message = f"invalid grade: {grade_str}"
        app.logger.info(err_message)
        return render_error(err_message), 400

    try:
        connection_meta: ConnectionMeta = ConnectionMeta.get(ConnectionMeta.session_id == session_id)
    except ConnectionMeta.DoesNotExist:
        err_message = f"invalid session id: {session_id}"
        app.logger.info(err_message)
        return render_error(err_message), 400

    lis_outcome_service_url: str = str(connection_meta.outcome_service_url)
    lis_result_sourcedid: str = str(connection_meta.sourced_id)

    xml = f"""<?xml version="1.0" encoding="UTF-8"?>
        <imsx_POXEnvelopeRequest xmlns="http://www.imsglobal.org/services/ltiv1p1/xsd/imsoms_v1p0">
          <imsx_POXHeader>
            <imsx_POXRequestHeaderInfo>
              <imsx_version>V1.0</imsx_version>
              <imsx_messageIdentifier>{uuid.uuid4()}</imsx_messageIdentifier>
            </imsx_POXRequestHeaderInfo>
          </imsx_POXHeader>
          <imsx_POXBody>
            <replaceResultRequest>
              <resultRecord>
                <sourcedGUID>
                  <sourcedId>{lis_result_sourcedid}</sourcedId>
                </sourcedGUID>
                <result>
                  <resultScore>
                    <language>en</language>
                    <textString>{grade_str}</textString>
                  </resultScore>
                </result>
              </resultRecord>
            </replaceResultRequest>
          </imsx_POXBody>
        </imsx_POXEnvelopeRequest>"""

    LTI_KEY = os.getenv("LTI_KEY")
    LTI_SECRET = os.getenv("LTI_SECRET")

    if LTI_KEY is None or LTI_SECRET is None:
        err_message = 'LTI consumer key or LTI shared secret are not set'
        app.logger.info(err_message)
        return render_error(err_message), 500

    client = Client(LTI_KEY, LTI_SECRET)
    uri, headers, body = client.sign(
        lis_outcome_service_url,
        http_method='POST',
        body=xml,
        headers={'Content-Type': 'application/xml'}
    )

    try:
        response = requests.post(lis_outcome_service_url, data=body, headers=headers)
    except MissingSchema:
        err_message = f"invalid URL: {lis_outcome_service_url}"
        app.logger.info(err_message)
        return render_error(err_message), 500
    except ConnectionError:
        err_message = f"unable to connect: {lis_outcome_service_url}"
        app.logger.info(err_message)
        return render_error(err_message), 500

    if response.status_code == 200:
        app.logger.info("grade successfully sent to Moodle!")

        with db.connection_context():
            evt = Events.create(session_id=session_id, event_type=Events.EventType.GRADE_SENT, event_timestamp=datetime.now())
            Grades.create(event_id=evt.event_id, grade_value=round(float(grade_str), 2), code=code)
    else:
        err_message = f"failed to send grade. Status code: {response.status_code}"
        app.logger.info(err_message)
        return render_error(err_message), response.status_code

    return redirect(url_for("main_page"))


@app.route('/api/capture_ripes_data/<session_id_str>', methods=['POST'])
def capture_ripes_data(session_id_str: str):
    """ Receives code, output, and registers data from Ripes client. """
    if request.method != 'POST':
        return render_error("Bad Request")

    try:
        app.logger.info(f"Received data capture request for session: {session_id_str}")
        try:
            session_id = uuid.UUID(session_id_str)
        except ValueError:
            app.logger.error(f"Invalid session ID format: {session_id_str}")
            return jsonify({"status": "error", "message": "Invalid session ID format"}), 400

        try:
            connection_meta: ConnectionMeta = ConnectionMeta.get(ConnectionMeta.session_id == session_id)
        except ConnectionMeta.DoesNotExist:
            app.logger.error(f"Session ID not found: {session_id_str}")
            return jsonify({"status": "error", "message": "Session ID not found"}), 404

        if not request.is_json:
            app.logger.error("Request is not JSON")
            return jsonify({"status": "error", "message": "Request must be JSON"}), 415

        data = request.get_json()
        if data is None:
            app.logger.error("Failed to parse JSON data")
            return jsonify({"status": "error", "message": "Could not parse JSON data"}), 400

        code = data.get('code')
        output = data.get('output')
        registers = data.get('registers')  # <-- Get the registers dictionary

        # --- Validate all required fields ---
        missing_keys = [k for k, v in {'code': code, 'output': output, 'registers': registers}.items() if v is None]
        if missing_keys:
            error_msg = f"Missing key(s) in JSON data: {', '.join(missing_keys)}"
            app.logger.error(error_msg)
            return jsonify({"status": "error", "message": error_msg}), 400

        # --- Optional: Validate registers format ---
        if not isinstance(registers, dict):
            error_msg = f"Expected 'registers' to be a dictionary, got {type(registers)}"
            app.logger.error(error_msg)
            return jsonify({"status": "error", "message": error_msg}), 400

        # --- Log the data ---
        app.logger.info(f"--- Captured Ripes Data (Session: {session_id_str}) ---")
        app.logger.info("Source Code:")
        app.logger.info(f"\n{code}\n")
        app.logger.info("Console Output:")
        app.logger.info(f"\n{output}\n")
        app.logger.info("Register States:")
        for reg, value in registers.items():  # Log registers nicely
            app.logger.info(f"  {reg}: {value}")
        app.logger.info("--- End Captured Data ---")

        # --- Run task checks ---
        with open(f"/tmp/{session_id_str}.s", mode="w") as f:
            f.write(code)

        task_id: str = str(connection_meta.task_id)
        task = get_task_by_id(task_id)(code_file=f"/tmp/{session_id_str}.s")
        message = "Success run"
        try:
            app.logger.info(f"start check")
            grade = task.run()
            app.logger.info(f"Success check run for {session_id_str} with grade {grade}")
            message = f"{message} grade: {grade}"
        except RuntimeError as e:
            app.logger.exception(f"Error during check run for session {session_id_str}: {e}")
            grade = 0.0
            message = f"Error during run: {e}"
            return jsonify({
                "status": "error",
                "message": message,
                "send_grade_address": url_for('send_grade_to_moodle', session_id_str=session_id_str, grade_str=str(grade), code=code)
            }), 500

        return jsonify({
            "status": "success",
            "message": message,
            "send_grade_address": url_for('send_grade_to_moodle', session_id_str=session_id_str, grade_str=str(grade), code=code)
        }), 200

    except Exception as e:
        app.logger.exception(f"Unexpected error in capture_ripes_data for session {session_id_str}: {e}")
        return jsonify({"status": "error", "message": f"Internal server error: {str(e)}"}), 500


@app.route('/', methods=['GET', 'POST'])
def main_page() -> str:
    session_id = request.args.get("session_id")
    if request.method == 'POST':
        app.logger.info('пришел POST')
        return render_template("index.html", session_id=session_id)
    elif request.method == 'GET':
        app.logger.info('пришел GET')
        return render_template("index.html", session_id=session_id)
    else:
        app.logger.info('пришел не GET и не POST')
        return render_error('Invalid request')

@app.route('/statistic/<session_id_str>', methods=['GET'])
def statistic_page(session_id_str: str) -> str:
    if request.method != 'GET':
        return render_error("Bad Request")

    try:
        session_id = uuid.UUID(session_id_str)
    except ValueError:
        err_message = f"invalid uuid: {session_id_str}"
        app.logger.info(err_message)
        return render_error(err_message)

    conn = ConnectionMeta.select().where(ConnectionMeta.session_id == session_id)[0]
    user_id = conn.user_id
    task_id = conn.task_id

    query = Events.select(
        Events.event_timestamp,
        ConnectionMeta.full_name,
        ConnectionMeta.email,
        ConnectionMeta.course_title,
        ConnectionMeta.task_id,
        Events.event_type,
        Grades.grade_value,
        Grades.code,
        ).join(ConnectionMeta, on=(Events.session_id == ConnectionMeta.session_id)
        ).join(Grades, JOIN.LEFT_OUTER, on=(Events.event_id == Grades.event_id)
        ).order_by(Events.event_timestamp.desc())

    query = query.where(ConnectionMeta.task_id == task_id)

    if not 'administrator' in conn.roles.lower():
        query = query.where(ConnectionMeta.user_id == user_id)

    query = query.dicts()

    return render_template('statistics.html', data=json.dumps(list(query), default=str, ensure_ascii=False))

@app.errorhandler(HTTPException)
def handle_exception(e):
    """
    Return JSON instead of HTML for HTTP errors
    """
    response = e.get_response()
    return render_template("error.html")


@app.errorhandler(404)
def page_not_found(e):
    return render_error('404 Not found')


def render_error(error_message) -> str:
    """
    Show error page with error_message
    """
    return render_template('error.html', error_message=error_message)


@app.after_request
def after_request(response):
    # Required headers for SharedArrayBuffer
    response.headers['Cross-Origin-Opener-Policy'] = 'same-origin'
    response.headers['Cross-Origin-Embedder-Policy'] = 'require-corp'
    return response


if __name__ == '__main__':
    app.run(debug=True)
