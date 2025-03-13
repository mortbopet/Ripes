from flask import Flask, render_template, request

app = Flask(
    __name__,
    template_folder="templates",
    static_folder="static"
)


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

    print(f"User ID: {user_id}")
    print(f"Full Name: {full_name}")
    print(f"Email: {email}")
    print(f"Course Title: {course_title}")
    print(f"Roles: {roles}")

    return render_template("index.html")


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
