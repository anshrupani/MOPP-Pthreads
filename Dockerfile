FROM tudinfse/cds_server

ADD ./mopp-2018-t0-harmonic-progression-sum /tmp/mopp-2018-t0-harmonic-progression-sum

RUN apt-get update
RUN apt-get install -y build-essential

RUN cd /tmp/mopp-2018-t0-harmonic-progression-sum && make && cp /tmp/mopp-2018-t0-harmonic-progression-sum/harmonic-progression-sum /usr/bin/
ADD ./cds_server.json /etc/
