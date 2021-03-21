FROM ubuntu:20.10
RUN apt-get update && apt-get install -y wget curl git file && rm -rf /var/lib/apt/lists/*

ADD . /app
RUN chmod +x /app/*.sh
WORKDIR /app
CMD [/app/filebeat_armhf.sh]

