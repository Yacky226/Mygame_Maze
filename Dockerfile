# syntax=docker/dockerfile:1
FROM ubuntu:24.04 AS builder

RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake libsfml-dev \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY CMakeLists.txt ./
COPY cmake/ ./cmake/
COPY core/ ./core/
COPY apps/cli/ ./apps/cli/
COPY scenarios/ ./scenarios/
COPY Mygame/ ./Mygame/
RUN cmake -S . -B /build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /build --parallel 2 \
    && ctest --test-dir /build --output-on-failure \
    && cmake --install /build --prefix /opt/maze

FROM ubuntu:24.04 AS runtime

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libsfml-graphics2.6 libsfml-window2.6 libsfml-system2.6 \
        libgl1-mesa-dri xvfb x11-utils x11vnc novnc websockify python3 tini \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --create-home --uid 10001 maze

COPY --from=builder /opt/maze/ /opt/maze/
COPY scenarios/ /opt/maze/scenarios/
COPY docker/start.sh docker/healthcheck.py /opt/maze/
# bash is invoked explicitly, so Windows executable permission bits are harmless.
RUN sed -i 's/\r$//' /opt/maze/start.sh \
    && install -d -m 1777 /tmp/.X11-unix

ENV DISPLAY=:99 \
    LIBGL_ALWAYS_SOFTWARE=1
USER maze
WORKDIR /opt/maze
EXPOSE 6080
HEALTHCHECK --interval=10s --timeout=5s --start-period=20s --retries=3 \
    CMD ["python3", "/opt/maze/healthcheck.py"]
ENTRYPOINT ["/usr/bin/tini", "--", "/bin/bash", "/opt/maze/start.sh"]
