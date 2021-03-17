

# FROM arm32v7-ub20 as ffmpeg
# WORKDIR /ffmpeg_inst
# RUN ldd `which ffmpeg` \
# | grep "=> /" \
# | awk '{print $3}' \
# | sort \
# | uniq \
# | xargs -I '{}' sh -c 'cp --parents `readlink -f {}` . ; cp --parents -P {} .' \
# && cp `which ffmpeg` --parents . 


# FROM debian:buster-slim
FROM pibase
LABEL maintainer="novice <novice79@126.com>"
ARG BUILD_DATE
# Labels.
LABEL org.label-schema.schema-version="1.0"
LABEL org.label-schema.build-date=$BUILD_DATE
LABEL org.label-schema.name="novice/py-portal"
LABEL org.label-schema.description="local [http/file/bt/dlna/socks5/nodejs/media] server"
LABEL org.label-schema.url="http://novice79.github.io/"
LABEL org.label-schema.docker.cmd="docker run -d --name pp --network=host -v \$PWD:/app/mystore novice/py-portal"
# docker image inspect --format='' novice/py-portal | grep -A 6 maintainer
# COPY sources.list /etc/apt/sources.list

# install docker on pi
# curl -fsSL https://get.docker.com | sudo bash -s
WORKDIR /app
# COPY pre_inst/lib/arm-linux-gnueabihf/ /usr/lib/arm-linux-gnueabihf/
# COPY pre_inst/usr/lib/arm-linux-gnueabihf/ /usr/lib/arm-linux-gnueabihf/
# COPY pre_inst/usr/local/bin/ /usr/local/bin/

# RUN apt-get update \
# 	&& apt-get install -y tzdata \
# 	unrar p7zip-full \
# 	transmission-daemon minidlna ffmpeg 
# # set timezone
# ENV TZ=Asia/Chongqing
# RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
# ---for ps & netstat
# RUN apt-get install -y procps net-tools 

COPY conf/settings.json /etc/transmission-daemon/settings.json
COPY conf/minidlna.conf /etc/minidlna.conf
# not need to add entire node things
# ADD node14.tar.gz /usr/
# just add one node executable
ADD bin/node /usr/bin/
COPY prefab ./prefab
COPY py ./py
COPY www ./www
COPY home ./home

# minidlna:1900/udp 8200/tcp; 
# py app: 57000/tcp 57001/tcp; 
# transmission: 51413 9091/tcp 42243/udp 53069/udp 45563/udp
EXPOSE 57000 57002 51413 9091/tcp 42243/udp 53069/udp 45563/udp 1900/udp 8200/tcp
# docker run with: -p 1900:1900/udp -p 8200:8200/tcp
# uploaded files in mystore, uploaded home site in home
# so docker run with: -v xxx:/app/mystore -v xxx:/app/mystore
VOLUME ["/app/mystore", "/app/home"]
ENTRYPOINT ["/app/py"]

# docker run -d --name pp -p 57000:57000 -p 57001:57001 \
# -p 1900:1900/udp -p 8200:8200/tcp \
# -p 51413:51413 -p 9091:9091/tcp -p 42243:42243/udp -p 53069:53069/udp -p 45563:45563/udp \
# -v $PWD/aaa:/app/mystore novice/py-portal

# docker run -d --name pp --network="host" -v $PWD/aaa:/app/mystore novice/py-portal