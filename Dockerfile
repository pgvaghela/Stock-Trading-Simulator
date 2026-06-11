# ---- Stage 1: Build the React frontend ----
FROM node:18-alpine AS frontend-builder
WORKDIR /app/frontend
COPY demo/src/main/resources/static/frontend/package*.json ./
RUN npm ci --silent
COPY demo/src/main/resources/static/frontend/ ./
RUN npm run build 2>/dev/null || npm run build

# ---- Stage 2: Build the AWS C++ SDK (cloudwatch only) ----
FROM ubuntu:22.04 AS aws-sdk-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        cmake git g++ make libssl-dev libcurl4-openssl-dev zlib1g-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 --branch 1.11.210 \
        https://github.com/aws/aws-sdk-cpp.git /tmp/aws-sdk && \
    cmake -S /tmp/aws-sdk -B /tmp/aws-sdk-build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_ONLY="cloudwatch" \
        -DENABLE_TESTING=OFF \
        -DAUTORUN_UNIT_TESTS=OFF && \
    cmake --build /tmp/aws-sdk-build -j"$(nproc)" && \
    cmake --install /tmp/aws-sdk-build --prefix /opt/aws-sdk && \
    rm -rf /tmp/aws-sdk /tmp/aws-sdk-build

# ---- Stage 3: Build the C++ backend ----
FROM ubuntu:22.04 AS backend-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        cmake git g++ make libsqlite3-dev libssl-dev libcurl4-openssl-dev \
        zlib1g-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=aws-sdk-builder /opt/aws-sdk /opt/aws-sdk

WORKDIR /build
COPY backend/ ./

RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH=/opt/aws-sdk && \
    cmake --build build -j"$(nproc)"

# ---- Stage 4: Minimal runtime image ----
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        libsqlite3-0 libssl3 libcurl4 ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=backend-builder /build/build/stock_simulator ./
COPY --from=backend-builder /opt/aws-sdk/lib/             ./aws-lib/
COPY --from=frontend-builder /app/frontend/build/         ./static/

ENV STATIC_DIR=/app/static
ENV PORT=8080
ENV DB_PATH=/app/data/stock_simulator.db
ENV LD_LIBRARY_PATH=/app/aws-lib

RUN mkdir -p /app/data

EXPOSE 8080
VOLUME ["/app/data"]

ENTRYPOINT ["./stock_simulator"]
