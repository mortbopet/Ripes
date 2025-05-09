#!/bin/sh
set -e

export DISPLAY=:99

if ! xdpyinfo -display "$DISPLAY" >/dev/null 2>&1; then
    Xvfb "$DISPLAY" -screen 0 1024x768x24 -ac +extension GLX +render -noreset &
fi

attempts=0
max_attempts=100
while [ $attempts -lt $max_attempts ]; do
    if xdpyinfo -display "$DISPLAY" >/dev/null 2>&1; then
        break
    fi
    attempts=$((attempts+1))
    sleep 0.1
done

if [ $attempts -eq $max_attempts ]; then
    echo "Xvfb failed to start"
    exit 1
fi

exec "$@"