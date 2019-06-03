FROM tudinfse/cds_server

ADD ./mopp-2018-t0-harmonic-progression-sum /tmp/mopp-2018-t0-harmonic-progression-sum
ADD ./mopp-2018-t2-levenshtein /tmp/mopp-2018-t2-levenshtein

RUN apt-get update
RUN apt-get install -y build-essential

RUN cd /tmp/mopp-2018-t0-harmonic-progression-sum && make && cp /tmp/mopp-2018-t0-harmonic-progression-sum/harmonic-progression-sum /usr/bin/
RUN cd /tmp/mopp-2018-t2-levenshtein && make && cp /tmp/mopp-2018-t2-levenshtein/lev /usr/bin/
ADD ./cds_server.json /etc/
