import os
import uuid
from uuid import uuid4

import requests
from dotenv import load_dotenv
from flask import Flask, render_template, request, redirect, url_for
from oauthlib.oauth1 import Client
from requests.exceptions import MissingSchema, ConnectionError

load_dotenv()
app = Flask(
    __name__,
    template_folder="templates",
    static_folder="static"
)

# TODO (Kirill Karpunin): need to make a database for this or something else BUT NOT A DICT!!!!!!!!!!!!!
sessions_dict: dict = {}


@app.route('/lti', methods=['POST'])
def lti_request() -> str:
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

    print(f"Session ID: {session_id}")
    print(f"User ID: {user_id}")
    print(f"Full Name: {full_name}")
    print(f"Email: {email}")
    print(f"Course Title: {course_title}")
    print(f"Roles: {roles}")

    lis_outcome_service_url = lti_data.get('lis_outcome_service_url')
    lis_result_sourcedid = lti_data.get('lis_result_sourcedid')

    sessions_dict[session_id] = (lis_outcome_service_url, lis_result_sourcedid)

    return redirect(url_for("main_page"))


# TODO (Kirill Karpunin): GET method here is only for testing. DON'T FORGET TO REMOVE IT!!!!!!!!!!!!!!!!!!!!
@app.route('/ripes/<session_id_str>/<grade_str>', methods=["GET", "POST"])
def send_grade_to_moodle(session_id_str: str, grade_str: str) -> str:
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
        print(f"invalid uuid: {session_id_str}")
        return "Something went wrong"  # TODO (Kirill Karpunin): need to make error page

    try:
        float(grade_str)
    except ValueError:
        print(f"invalid grade: {grade_str}")
        return "Something went wrong"  # TODO (Kirill Karpunin): need to make error page

    if session_id not in sessions_dict:
        print(f"invalid session id: {session_id}")
        return "Something went wrong"  # TODO (Kirill Karpunin): need to make error page

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
        print("LTI consumer key or LTI shared secret are not set")
        return "Something went wrong"  # TODO (Kirill Karpunin): need to make error page

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
        print(f"invalid URL: {lis_outcome_service_url}")
        return "Something went wrong"  # TODO (Kirill Karpunin): need to make error page
    except ConnectionError:
        print(f"unable to connect: {lis_outcome_service_url}")
        return "Something went wrong"  # TODO (Kirill Karpunin): need to make error page

    if response.status_code == 200:
        print("grade successfully sent to Moodle!")
    else:
        print(f"failed to send grade. Status code: {response.status_code}")

    return redirect(url_for("main_page"))


@app.route('/', methods=['GET', 'POST'])
def main_page() -> str:
    if request.method == 'POST':
        print('пришел POST')
        return render_template("index.html")
    else:
        print('пришел GET')
        return render_template("index.html")


if __name__ == '__main__':
    app.run(debug=False)
