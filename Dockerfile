FROM ubuntu:18.04
MAINTAINER Kotyarich
USER root

RUN apt update -y
RUN apt install -y wget cmake g++ make

ENV PROJECT_NAME kotya_server

WORKDIR /opt/$PROJECT_NAME/
ADD . /opt/$PROJECT_NAME/

RUN mkdir build &&\
    cd build &&\
    cmake -DCMAKE_BUILD_TYPE=Release .. &&\
    make

EXPOSE 80
CMD ["/opt/kotya_server/build/server", "-p", "80", "-c", "/etc/httpd.conf"]