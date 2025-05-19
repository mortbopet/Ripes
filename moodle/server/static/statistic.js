function initStatisticsTable(data) {
    $('#statsTable').DataTable({
        data: data,
        language: {
            url: 'https://cdn.datatables.net/plug-ins/2.3.1/i18n/ru.json'
        },
        columns: [
            { data: 'full_name' },
            { data: 'user_id' },
            { data: 'course_title' },
            { data: 'task_id' },
            { data: 'grade' },
            { data: 'send_timestamp' }
        ],
        pageLength: 25,
        responsive: true,
    });
}

$(document).ready(function() {
    initStatisticsTable($('#statsTable').data('table-data'));
});