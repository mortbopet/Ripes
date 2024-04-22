from flask import abort, session
from db import get_session, add_session


def check_auth():
    session_id = session.get('session_id', None)
    user_session = get_session(session_id)
    if user_session:
        return user_session
    else:
        abort(401)


def check_admin():
    return get_session(session.get('session_id', None)).get('admin', False)


def check_task_access(task_id):
    user_session = get_session(session.get('session_id', None))
    if check_admin():
        return True
    else:
        return task_id in user_session['tasks']
