

FROM novice/build as my-build
WORKDIR /workplace
COPY dist ./dist
COPY 3p ./3p
COPY cpp ./cpp
COPY spa ./spa
COPY CMakeLists.txt .
COPY build.sh .
RUN mkdir -p lib \
    && cp -P /lib/x86_64-linux-gnu/libmagic.so* lib/ \
    && cp -P /lib/x86_64-linux-gnu/libsqlite3.so* lib/
# build spa ui
RUN cd spa && npm i && npm run build
# build c++ backend
RUN ./build.sh

FROM ubuntu:20.04
LABEL maintainer="novice <novice79@126.com>"
ARG BUILD_DATE
# Labels.
LABEL org.label-schema.schema-version="1.0"
LABEL org.label-schema.build-date=$BUILD_DATE
LABEL org.label-schema.name="novice/py-portal"
LABEL org.label-schema.description="local [http/file/bt/dlna/socks5/nodejs/media] server"
LABEL org.label-schema.url="http://novice79.github.io/"
LABEL org.label-schema.docker.cmd="docker run -d --name pp --network=host -v \$PWD:/app/mystore novice/py-portal"

WORKDIR /app
RUN apt-get update \
	&& apt-get install -y tzdata \
	unrar p7zip-full \
	transmission-daemon minidlna ffmpeg
COPY --from=my-build /workplace/dist /app
COPY --from=my-build /workplace/lib /lib/x86_64-linux-gnu/
COPY pi-dist/conf/settings.json /etc/transmission-daemon/settings.json
COPY pi-dist/conf/minidlna.conf /etc/minidlna.conf

# minidlna:1900/udp 8200/tcp; 
# py app: 57000/tcp 57001/tcp; 
# transmission: 51413 9091/tcp 42243/udp 53069/udp 45563/udp
EXPOSE 57000 57002 51413 9091/tcp 42243/udp 53069/udp 45563/udp 1900/udp 8200/tcp
# docker run with: -p 1900:1900/udp -p 8200:8200/tcp
# uploaded files in mystore, uploaded home site in home
# so docker run with: -v xxx:/app/mystore -v xxx:/app/mystore
VOLUME ["/app/mystore", "/app/home"]
ENTRYPOINT ["/app/py"]
