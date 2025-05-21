function showModal(title, message, type = 'info') { 
    $('.ui.modal.app-notification-modal').remove();

    const modalId = 'app-notification-modal-' + Date.now(); 
    let iconHtml = '';
    let buttonClass = 'blue'; 

    switch (type) {
        case 'success':
            iconHtml = '<i class="check circle outline green icon"></i> ';
            buttonClass = 'green';
            break;
        case 'error':
            iconHtml = '<i class="times circle outline red icon"></i> ';
            buttonClass = 'red';
            break;
        case 'info':
            iconHtml = '<i class="info circle blue icon"></i> ';
            buttonClass = 'blue';
            break;
    }

    const headerContent = `${iconHtml}${title}`;
    const modalHtml = `
        <div class="ui mini modal app-notification-modal" id="${modalId}">
            <div class="header">${headerContent}</div>
            <div class="content">
                <p>${message}</p>
            </div>
            <div class="actions">
                <div class="ui ${buttonClass} ok button">OK</div>
            </div>
        </div>
    `;

    $(document.body).append(modalHtml);
    $('#' + modalId)
        .modal({
            closable: true, 
            onHidden: function() {
                
                $(this).remove();
            }
        })
        .modal('show');
}


function server_get_request() {
    alert('GET-запрос отправлен')
    fetch('/', {
        method: 'GET'
    }).then((res) => {
        if (res.ok) {
            showModal('Успех', 'GET-запрос успешно обработан', 'success');
        } else {
            showModal('Ошибка', 'При обработке GET-запроса произошла ошибка', 'error');
        }
    })
}

function server_post_request() {
    alert('POST-запрос отправлен')
    fetch('/', {
        method: 'POST'
    }).then((res) => {
        if (res.ok) {
            showModal('Успех', 'POST-запрос успешно обработан.', 'success');
        } else {
            showModal('Ошибка', 'При обработке POST-запроса произошла ошибка.', 'error');
        }
    })
}

function send_grade_to_moodle(session_id) {
    if (session_id === 'None') {
        showModal('Ошибка авторизации', 'Вы не авторизовались через Moodle. Пожалуйста, войдите через Moodle.', 'error');
        return
    }

    if (session_id.trim() === '') {
        showModal('Ошибка данных', 'Не указан ID сессии. Невозможно отправить оценку.', 'error');
        return
    }

    const grade = document.getElementById("slider_value").textContent

    fetch('/ripes/' + session_id + '/' + grade, {
        method: 'POST'
    }).then((res) => {
        if (res.ok) {
            showModal('Успех', 'Оценка успешно отправлена в Moodle.', 'success');
        } else {
            showModal('Ошибка отправки', 'Отправить оценку не удалось', 'error');
        }
    })
}

function delete_grade_from_moodle(session_id) {
    if (session_id === 'None') {
        showModal('Ошибка авторизации', 'Вы не авторизовались через Moodle. Пожалуйста, войдите через Moodle.', 'error');
        return
    }

    if (session_id.trim() === '') {
        showModal('Ошибка данных', 'Не указан ID сессии. Невозможно удалить оценку.', 'error');
        return
    }

    const grade = document.getElementById("slider_value").textContent

    fetch('/ripes/' + session_id + '/delete', {
        method: 'DELETE'
    }).then((res) => {
        if (res.ok) {
            showModal('Успех', 'Оценка успешно удалена из Moodle.', 'success');
        } else {
            showModal('Ошибка удаления', 'Удалить оценку из Moodle не удалось.', 'error');
        }
    })
}

function update_slider_value(value) {
    const float_value = value / 10
    document.getElementById("slider_value").textContent = float_value.toFixed(1);
}