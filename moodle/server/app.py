import os
import uuid
import time
from uuid import uuid4

import requests
import logging
from dotenv import load_dotenv
from flask import Flask, render_template, request, redirect, url_for, Response, jsonify
from oauthlib.oauth1 import Client
from requests.exceptions import MissingSchema, ConnectionError
from werkzeug.exceptions import HTTPException

load_dotenv()
app = Flask(
    __name__,
    template_folder="templates",
    static_folder="static"
)
app.logger.setLevel(logging.INFO)

# TODO (Kirill Karpunin): need to make a database for this or something else BUT NOT A DICT!!!!!!!!!!!!!
sessions_dict: dict = {}


@app.route('/lti', methods=['POST'])
def lti_request() -> str | Response:
    """
    A method that processes a request from Moodle.
    Retrieves user data from the request and prints it to the console.

    :return: A template rendering the page with Ripes.
    """

    lti_data = request.form

    user_id = lti_data.get('user_id', 'No user ID')
    full_name = lti_data.get('lis_person_name_full', 'Unknown user')
    email = lti_data.get('lis_person_contact_email_primary', 'No email')
    course_title = lti_data.get('context_title', 'No course title')
    roles = lti_data.get('roles', 'No user roles')

    # TODO (Kirill Karpunin): maybe change this later
    session_id = uuid4()

    app.logger.info(f"Session ID: {session_id}")
    app.logger.info(f"User ID: {user_id}")
    app.logger.info(f"Full Name: {full_name}")
    app.logger.info(f"Email: {email}")
    app.logger.info(f"Course Title: {course_title}")
    app.logger.info(f"Roles: {roles}")

    lis_outcome_service_url = lti_data.get('lis_outcome_service_url')
    lis_result_sourcedid = lti_data.get('lis_result_sourcedid')

    sessions_dict[session_id] = {
        'lis_outcome_service_url': lis_outcome_service_url,
        'lis_result_sourcedid': lis_result_sourcedid,
        'user_id': user_id,
        'full_name': full_name,
        'email': email,
        'course_title': course_title,
        'roles': roles,
        'ripes_data': None
    }

    return redirect(url_for("main_page", session_id=session_id))


@app.route('/ripes/<session_id_str>/<grade_str>', methods=["POST"])
def send_grade_to_moodle(session_id_str: str, grade_str: str) -> str | Response:
    """
    Method for sending a grade to Moodle.
    Uses session ID to get a specific connection ID to set a grade for a correct student.

    :param session_id_str: String containing session ID.
    :param grade_str: String containing grade.
    :return: An error message if something went wrong, else a template rendering the page with Ripes.
    """
    try:
        session_id = uuid.UUID(session_id_str)
    except ValueError:
        err_message = f"invalid uuid: {session_id_str}"
        app.logger.info(err_message)
        return render_error(err_message)

    try:
        float(grade_str)
    except ValueError:
        err_message = f"invalid grade: {grade_str}"
        app.logger.info(err_message)
        return render_error(err_message)

    if session_id not in sessions_dict:
        err_message = f"invalid session id: {session_id}"
        app.logger.info(err_message)
        return render_error(err_message)

    lis_outcome_service_url, lis_result_sourcedid = sessions_dict[session_id]

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
        return render_error(err_message)

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
        return render_error(err_message)
    except ConnectionError:
        err_message = f"unable to connect: {lis_outcome_service_url}"
        app.logger.info(err_message)
        return render_error(err_message)

    if response.status_code == 200:
        app.logger.info("grade successfully sent to Moodle!")
    else:
        app.logger.info(f"failed to send grade. Status code: {response.status_code}")

    return redirect(url_for("main_page"))


@app.route('/ripes/<session_id_str>/delete', methods=['DELETE'])
def erase_grade_from_moodle(session_id_str: str) -> str | Response:
    """
    Method for erasing a grade from Moodle.
    Uses session ID to get a specific connection ID to erase a grade for a correct student.

    :param session_id_str: String containing session ID.
    :return: An error message if something went wrong, else a template rendering the page with Ripes.
    """
    try:
        session_id = uuid.UUID(session_id_str)
    except ValueError:
        err_message = f"invalid uuid: {session_id_str}"
        app.logger.info(err_message)
        return render_error(err_message)

    if session_id not in sessions_dict:
        err_message = f"invalid session id: {session_id}"
        app.logger.info(err_message)
        return render_error(err_message)

    lis_outcome_service_url, lis_result_sourcedid = sessions_dict[session_id]

    xml = f"""<?xml version="1.0" encoding="UTF-8"?>
        <imsx_POXEnvelopeRequest xmlns="http://www.imsglobal.org/services/ltiv1p1/xsd/imsoms_v1p0">
            <imsx_POXHeader>
                <imsx_POXRequestHeaderInfo>
                    <imsx_version>V1.0</imsx_version>
                    <imsx_messageIdentifier>{uuid.uuid4()}</imsx_messageIdentifier>
                </imsx_POXRequestHeaderInfo>
            </imsx_POXHeader>
            <imsx_POXBody>
                <deleteResultRequest>
                    <resultRecord>
                        <sourcedGUID>
                            <sourcedId>{lis_result_sourcedid}</sourcedId>
                        </sourcedGUID>
                    </resultRecord>
                </deleteResultRequest>
            </imsx_POXBody>
        </imsx_POXEnvelopeRequest>"""

    LTI_KEY = os.getenv("LTI_KEY")
    LTI_SECRET = os.getenv("LTI_SECRET")

    if LTI_KEY is None or LTI_SECRET is None:
        err_message = 'LTI consumer key or LTI shared secret are not set'
        app.logger.info(err_message)
        return render_error(err_message)

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
        return render_error(err_message)
    except ConnectionError:
        err_message = f"unable to connect: {lis_outcome_service_url}"
        app.logger.info(err_message)
        return render_error(err_message)

    if response.status_code == 200:
        app.logger.info("grade successfully deleted from Moodle!")
    else:
        app.logger.info(f"failed to delete grade. Status code: {response.status_code}")

    return redirect(url_for("main_page"))

@app.route('/api/capture_ripes_data/<session_id_str>', methods=['POST'])
def capture_ripes_data(session_id_str: str):
    try:
        app.logger.info(f"Received data capture request for session: {session_id_str}")
        try:
            session_id = uuid.UUID(session_id_str)
        except ValueError:
            app.logger.error(f"Invalid session ID format received: {session_id_str}")
            return jsonify({"status": "error", "message": "Invalid session ID format"}), 400

        if session_id not in sessions_dict:
            app.logger.error(f"Session ID not found in active sessions: {session_id_str}")
            return jsonify({"status": "error", "message": "Session ID not found"}), 404

        if not request.is_json:
            app.logger.error("Request from Ripes client is not JSON")
            return jsonify({"status": "error", "message": "Request must be JSON"}), 415

        data = request.get_json()
        if data is None:
             app.logger.error("Failed to parse JSON data")
             return jsonify({"status": "error", "message": "Could not parse JSON data"}), 400

        code = data.get('code')
        output = data.get('output')

        if code is None or output is None:
            app.logger.error("Missing 'code' or 'output' key in JSON data from Ripes")
            return jsonify({"status": "error", "message": "Missing 'code' or 'output' key"}), 400

        # --- Log the data ---
        app.logger.info(f"--- Captured Ripes Data (Session: {session_id_str}) ---")
        app.logger.info("Source Code:")
        app.logger.info(f"\n{code}\n")
        app.logger.info("Console Output:")
        app.logger.info(f"\n{output}\n")
        app.logger.info("--- End Captured Data ---")

        # --- Store data (This should now work) ---
        sessions_dict[session_id]['ripes_data'] = {
            'code': code,
            'output': output,
            'timestamp': time.time()
        }
        app.logger.info(f"Stored captured data for session {session_id_str}")

        return jsonify({"status": "success", "message": "Data received and logged"}), 200

    except Exception as e:
        app.logger.exception(f"Unexpected error in capture_ripes_data for session {session_id_str}: {e}")
        # This should now work as long as jsonify is imported
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
