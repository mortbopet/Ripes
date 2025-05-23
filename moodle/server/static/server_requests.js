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