function server_get_request() {
    alert('GET-запрос отправлен')
    fetch('/', {
        method: 'GET'
    }).then((res) => {
        if (res.ok) {
            alert('GET-запрос успешно обработан')
        } else {
            alert('При обработке GET-запроса произошла ошибка')
        }
    })
}

function server_post_request() {
    alert('POST-запрос отправлен')
    fetch('/', {
        method: 'POST'
    }).then((res) => {
        if (res.ok) {
            alert('POST-запрос успешно обработан')
        } else {
            alert('При обработке POST-запроса произошла ошибка')
        }
    })
}

function send_grade_to_moodle(session_id) {
    if (session_id === 'None') {
        alert('Вы не авторизовались через Moodle')
        return
    }

    if (session_id.trim() === '') {
        alert('Не указан ID сессии')
        return
    }

    const grade = document.getElementById("slider_value").textContent

    fetch('/ripes/' + session_id + '/' + grade, {
        method: 'POST'
    }).then((res) => {
        if (res.ok) {
            alert('Оценка отправлена')
        } else {
            alert('Отправить оценку не удалось')
        }
    })
}

function delete_grade_from_moodle(session_id) {
    if (session_id === 'None') {
        alert('Вы не авторизовались через Moodle')
        return
    }

    if (session_id.trim() === '') {
        alert('Не указан ID сессии')
        return
    }

    const grade = document.getElementById("slider_value").textContent

    fetch('/ripes/' + session_id + '/delete', {
        method: 'DELETE'
    }).then((res) => {
        if (res.ok) {
            alert('Оценка удалена')
        } else {
            alert('Удалить оценку не удалось')
        }
    })
}

function update_slider_value(value) {
    const float_value = value / 10
    document.getElementById("slider_value").textContent = float_value.toFixed(1);
}