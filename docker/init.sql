CREATE TABLE IF NOT EXISTS connection_meta (
    session_id UUID PRIMARY KEY,
    task_id VARCHAR(256) NOT NULL,
    course_title VARCHAR(512) NOT NULL,
    user_id BIGINT NOT NULL,
    full_name VARCHAR(1024) NOT NULL,
    email VARCHAR(256) NOT NULL,
    outcome_service_url VARCHAR(1024) NOT NULL,
    sourced_id VARCHAR(256) NOT NULL
);

CREATE TABLE IF NOT EXISTS statistics (
    statistics_id SERIAL PRIMARY KEY,
    session_id UUID REFERENCES connection_meta(session_id) ON DELETE CASCADE,
    grade NUMERIC(3,2) DEFAULT 0.00,
    send_timestamp TIMESTAMP
);