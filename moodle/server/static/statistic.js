function initStatisticsTable(data) {
    $('#statsTable').DataTable({
        data: data,
        language: {
            url: 'https://cdn.datatables.net/plug-ins/2.3.1/i18n/ru.json'
        },
        columns: [
            { data: 'event_timestamp' },
            { data: 'full_name' },
            { data: 'email' },
            { data: 'course_title' },
            { data: 'task_id' },
            { data: 'event_type' },
            { data: 'grade_value' },
            { data: 'code' }
        ],
        pageLength: 10,
        responsive: true,
        order: []
    });
}

$(document).ready(function() {
    initStatisticsTable($('#statsTable').data('table-data'));
});