CREATE TABLE IF NOT EXISTS connection_meta (
    session_id UUID PRIMARY KEY,
    user_id BIGINT NOT NULL,
    full_name VARCHAR(1024) NOT NULL,
    email VARCHAR(256) NOT NULL,
    outcome_service_url VARCHAR(1024) NOT NULL,
    sourced_id VARCHAR(256) NOT NULL
);

CREATE TABLE IF NOT EXISTS statistics (
    statistics_id SERIAL PRIMARY KEY,
    session_id UUID NOT NULL,
    status VARCHAR(8) DEFAULT 'NOT SENT',
    grade NUMERIC(4,2) DEFAULT 0.00
);