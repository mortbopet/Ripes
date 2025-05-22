CREATE TABLE IF NOT EXISTS connection_meta (
    session_id UUID PRIMARY KEY,
    task_id VARCHAR(256) NOT NULL,
    course_title VARCHAR(512) NOT NULL,
    user_id BIGINT NOT NULL,
    full_name VARCHAR(1024) NOT NULL,
    email VARCHAR(256) NOT NULL,
    roles VARCHAR(512) NOT NULL,
    outcome_service_url VARCHAR(1024) NOT NULL,
    sourced_id VARCHAR(256) NOT NULL
);

CREATE TYPE EVENT_TYPE AS ENUM ('SESSION_OPENED', 'GRADE_SENT');
CREATE TABLE IF NOT EXISTS events (
    event_id BIGSERIAL PRIMARY KEY,
    session_id UUID REFERENCES connection_meta(session_id) ON DELETE CASCADE,
    event_type EVENT_TYPE NOT NULL,
    event_timestamp TIMESTAMP
);

CREATE TABLE IF NOT EXISTS grades (
    grade_id BIGSERIAL PRIMARY KEY,
    event_id BIGINT NOT NULL REFERENCES events(event_id) ON DELETE CASCADE,
    grade_value NUMERIC(3,2) NOT NULL,
    code VARCHAR(4096) NOT NULL
);