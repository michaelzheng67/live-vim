FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    libboost-all-dev \
    libncurses-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make DOCKER=1

EXPOSE 8002

CMD ["./live-vim"]
