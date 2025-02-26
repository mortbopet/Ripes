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