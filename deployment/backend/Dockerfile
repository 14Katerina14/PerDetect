# syntax=docker/dockerfile:1.7

FROM ubuntu:24.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
ARG VCPKG_COMMIT=d015e31e90838a4c9dfa3eed45979bc70d9357fc
ARG SECUREZONE_BUILD_ID=local

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
        autoconf \
        autoconf-archive \
        automake \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        git \
        libtool \
        ninja-build \
        pkg-config \
        python3 \
        tar \
        unzip \
        zip \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && git -C /opt/vcpkg checkout "${VCPKG_COMMIT}" \
    && /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics

WORKDIR /src

# Install manifest dependencies before copying the source so application-only
# changes can reuse the expensive vcpkg layer.
COPY vcpkg.json ./
RUN /opt/vcpkg/vcpkg install \
        --triplet x64-linux \
        --x-install-root=/opt/vcpkg-installed

COPY . .

RUN cmake -S . -B /src/build \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DVCPKG_INSTALLED_DIR=/opt/vcpkg-installed \
        -DVCPKG_MANIFEST_INSTALL=OFF \
        -DSECUREZONE_BUILD_MONGODB_INFRA=ON \
        -DSECUREZONE_BUILD_MONGODB_TESTS=OFF \
        -DSECUREZONE_BUILD_MONGODB_INTEGRATION_TESTS=OFF \
        -DSECUREZONE_BUILD_HTTP_RUNTIME=ON \
    && cmake --build /src/build --target SecureZone.Api SecureZone.PasswordHash SecureZone.ProvisionUser --parallel 2 \
    && mkdir -p /opt/securezone \
    && cp /src/build/SecureZone.Api/SecureZone.Api /opt/securezone/SecureZone.Api \
    && cp /src/build/SecureZone.Tools/SecureZone.PasswordHash /opt/securezone/SecureZone.PasswordHash \
    && cp /src/build/SecureZone.Tools/SecureZone.ProvisionUser /opt/securezone/SecureZone.ProvisionUser

FROM ubuntu:24.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive
ARG SECUREZONE_BUILD_ID=local

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
        ca-certificates \
        curl \
        libsasl2-2 \
        libssl3 \
        zlib1g \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd --system securezone \
    && useradd --system --gid securezone --home-dir /app --shell /usr/sbin/nologin securezone

WORKDIR /app
COPY --from=build --chown=securezone:securezone /opt/securezone/SecureZone.Api /app/SecureZone.Api
COPY --from=build --chown=securezone:securezone /opt/securezone/SecureZone.PasswordHash /app/SecureZone.PasswordHash
COPY --from=build --chown=securezone:securezone /opt/securezone/SecureZone.ProvisionUser /app/SecureZone.ProvisionUser

ENV SECUREZONE_API_HOST=0.0.0.0 \
    SECUREZONE_API_PORT=8080 \
    SECUREZONE_MONGO_DATABASE=securezone \
    SECUREZONE_QR_PRESENCE_MINUTES=2 \
    SECUREZONE_JWT_TTL_MINUTES=60 \
    SECUREZONE_UNIDENTIFIED_GRACE_SECONDS=120 \
    SECUREZONE_BUILD_ID=${SECUREZONE_BUILD_ID}

EXPOSE 8080

USER securezone

HEALTHCHECK --interval=10s --timeout=3s --start-period=10s --retries=5 \
    CMD curl --fail --silent --show-error http://127.0.0.1:8080/health || exit 1

ENTRYPOINT ["/app/SecureZone.Api"]
