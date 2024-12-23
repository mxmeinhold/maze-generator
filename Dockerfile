FROM alpine:latest AS build
LABEL maintainer="Max Meinhold <mxmeinhold@gmail.com>"

RUN apk add --no-cache build-base bash

ENV LIBPNG_VER=1.6.44-r0
ENV ZLIB_VER=1.3.1-r2

RUN apk add --no-cache libpng-dev=${LIBPNG_VER} libpng-static=${LIBPNG_VER} \
    zlib-dev=${ZLIB_VER} zlib-static=${ZLIB_VER}

ENV IMG_VER=v0.1.1

WORKDIR /usr/src/img
ADD https://github.com/mxmeinhold/img/releases/download/${IMG_VER}/img-${IMG_VER}.tar.gz ./
RUN tar -xzf img-${IMG_VER}.tar.gz
RUN make install


WORKDIR /usr/src/maze
COPY ./*.c ./*.h Makefile ./
RUN make STATIC=1

FROM python:3.11-alpine
LABEL maintainer="Max Meinhold <mxmeinhold@gmail.com>"

RUN apk add git && \
    apk add tzdata && \
    cp /usr/share/zoneinfo/America/New_York /etc/localtime && \
    echo 'America/New_York' > /etc/timezone && \
    apk del tzdata

WORKDIR /opt/maze-web

COPY wsgi.py config.env.py ./
COPY requirements.txt .

RUN pip install -r requirements.txt

COPY maze_web ./maze_web
COPY --from=build /usr/src/maze/target/maze ./
COPY .git/ ./.git/

RUN touch maze.out && chmod a+rw maze.out

CMD ["gunicorn", "maze_web:APP", "--bind=0.0.0.0:5000", "--access-logfile=-"]
