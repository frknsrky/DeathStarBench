FROM golang:1.21 as builder

RUN apt-get update && apt-get install -y gcc

WORKDIR /workspace

COPY go.sum go.sum
COPY go.mod go.mod
COPY vendor/ vendor/

COPY cmd/ cmd/
COPY dialer/ dialer/
COPY registry/ registry/
COPY services/ services/
COPY tls/ tls/
COPY tracing/ tracing/
COPY tune/ tune/

COPY config.json config.json

WORKDIR /workspace/services/perf
RUN gcc -std=c11 -c perf_api.c -o perf_api.o && ar rcs libperf_api.a perf_api.o

WORKDIR /workspace

ENV CGO_ENABLED=1 \
    CGO_LDFLAGS="-L/workspace/services/perf -lperf_api" \
    CGO_CFLAGS="-I/workspace/services/perf"

RUN GOOS=linux GO111MODULE=on go install -mod=vendor ./cmd/...


