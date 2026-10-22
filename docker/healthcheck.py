"""Verify that both the noVNC page and its VNC backend are ready."""

import socket
import urllib.request


with urllib.request.urlopen("http://127.0.0.1:6080/vnc.html", timeout=2) as response:
    if response.status != 200:
        raise SystemExit("noVNC is unavailable")

with socket.create_connection(("127.0.0.1", 5900), timeout=2) as connection:
    if connection.makefile("rb").read(12) != b"RFB 003.008\n":
        raise SystemExit("VNC is not ready")
