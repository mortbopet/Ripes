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

    const grade = document.getElementById("slider_value").textContent

    fetch('/ripes/' + session_id + '/' + grade, {
        method: 'POST'
    }).then((res) => {
        if (res.ok) {
            alert('Оценка успешно отправлена')
        } else {
            alert('Отправить оценку не удалось')
        }
    })
}

function update_slider_value(value) {
    const float_value = value / 10
    document.getElementById("slider_value").textContent = float_value.toFixed(1);
}