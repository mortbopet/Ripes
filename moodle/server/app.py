import uuid
from uuid import uuid4
from xml.etree import ElementTree as ET

import requests
from flask import Flask, render_template, request, redirect, url_for
from requests.exceptions import MissingSchema, ConnectionError

app = Flask(
    __name__,
    template_folder="templates",
    static_folder="static"
)

# need to make a database for this or something else BUT NOT A DICT!!!!!!!!!!!!!
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

    # maybe change this later
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


# GET method here is only for testing. DON'T FORGET TO REMOVE IT!!!!!!!!!!!!!!!!!!!!
@app.route('/ripes/<session_id_str>/<grade_str>', methods=["GET", "POST"])
def send_grade_to_moodle(session_id_str: str, grade_str: str) -> str:
    try:
        session_id = uuid.UUID(session_id_str)
    except ValueError:
        return f"invalid uuid: {session_id_str}"  # need to make error page

    if session_id not in sessions_dict:
        return f"invalid session id: {session_id}"  # need to make error page

    lis_outcome_service_url, lis_result_sourcedid = sessions_dict[session_id]

    root = ET.Element('imsx_POXEnvelopeRequest', xmlns='http://www.imsglobal.org/services/ltiv1p1/xsd/imsoms_v1p0')

    header = ET.SubElement(root, 'imsx_POXHeader')
    header_info = ET.SubElement(header, 'imsx_POXRequestHeaderInfo')
    ET.SubElement(header_info, 'imsx_version').text = 'V1.2'
    ET.SubElement(header_info, 'imsx_messageIdentifier').text = str(uuid4())

    body = ET.SubElement(root, 'imsx_POXBody')
    replace_result = ET.SubElement(body, 'replaceResultRequest')
    result_record = ET.SubElement(replace_result, 'resultRecord')
    ET.SubElement(result_record, 'sourcedGUID').text = lis_result_sourcedid
    result = ET.SubElement(result_record, 'result')
    ET.SubElement(result, 'resultScore').text = grade_str
    ET.SubElement(result, 'language').text = 'en'

    xml_payload = ET.tostring(root, encoding='utf-8', method='xml')

    headers = {'Content-Type': 'application/xml'}
    try:
        response = requests.post(lis_outcome_service_url, data=xml_payload, headers=headers)
    except MissingSchema:
        return f"invalid URL: {lis_outcome_service_url}"  # need to make error page
    except ConnectionError:
        return f"unable to connect: {lis_outcome_service_url}"  # need to make error page

    if response.status_code == 200:
        print("Grade successfully sent to Moodle!")
    else:
        print(f"Failed to send grade. Status code: {response.status_code}")

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
