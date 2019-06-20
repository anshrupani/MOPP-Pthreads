FROM tudinfse/cds_server

RUN apt-get update
RUN apt-get install -y build-essential


ADD ./mopp-2018-t0-harmonic-progression-sum /tmp/mopp-2018-t0-harmonic-progression-sum
ADD ./mopp-2018-t2-levenshtein-large /tmp/mopp-2018-t2-levenshtein-large
ADD ./mopp-2018-t2-levenshtein /tmp/mopp-2018-t2-levenshtein
ADD ./mopp-2016-t3-sudokount /tmp/mopp-2016-t3-sudokount

RUN cd /tmp/mopp-2018-t0-harmonic-progression-sum && make && cp /tmp/mopp-2018-t0-harmonic-progression-sum/harmonic-progression-sum /usr/bin/
RUN cd /tmp/mopp-2018-t2-levenshtein-large && make && cp /tmp/mopp-2018-t2-levenshtein-large/lev /usr/bin/
RUN cd /tmp/mopp-2018-t2-levenshtein && make && cp /tmp/mopp-2018-t2-levenshtein/levsmall /usr/bin/
RUN cd /tmp/mopp-2016-t3-sudokount && make && cp /tmp/mopp-2016-t3-sudokount/sudokount /usr/bin/
ADD ./cds_server.json /etc/
