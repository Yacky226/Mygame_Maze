#!/usr/bin/env bash
set -Eeuo pipefail

pids=()
cleanup() {
    trap - EXIT TERM INT
    if ((${#pids[@]})); then
        kill "${pids[@]}" 2>/dev/null || true
        wait "${pids[@]}" 2>/dev/null || true
    fi
}
trap cleanup EXIT
trap 'exit 0' TERM INT

Xvfb "$DISPLAY" -screen 0 1300x800x24 -nolisten tcp -ac +extension GLX -noreset &
pids+=("$!")

# Wait for X11 before SFML creates its OpenGL context.
ready=false
for ((attempt = 0; attempt < 100; attempt++)); do
    if xdpyinfo -display "$DISPLAY" >/dev/null 2>&1; then
        ready=true
        break
    fi
    if ! kill -0 "${pids[0]}" 2>/dev/null; then
        break
    fi
    sleep 0.1
done
if [[ "$ready" != true ]]; then
    echo "Le serveur graphique Xvfb n'a pas pu demarrer." >&2
    exit 1
fi

# VNC stays inside the container. Only the browser endpoint is published.
x11vnc -display "$DISPLAY" -rfbport 5900 -localhost -nopw \
    -forever -shared -noxdamage -quiet &
pids+=("$!")
websockify --web=/usr/share/novnc/ 6080 127.0.0.1:5900 &
pids+=("$!")
./maze &
pids+=("$!")

echo "Maze : http://localhost:6080/vnc.html?autoconnect=true&resize=scale"

# Stop all services if the game closes or one of its dependencies fails.
set +e
wait -n "${pids[@]}"
status=$?
set -e
exit "$status"
