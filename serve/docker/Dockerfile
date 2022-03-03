


#from vnv_env:latest
from vnv_base:latest

RUN wget -qO - https://www.mongodb.org/static/pgp/server-5.0.asc | apt-key add -
RUN echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu bionic/mongodb-org/5.0 multiverse" | tee /etc/apt/sources.list.d/mongodb-org-5.0.list
RUN apt-get update && apt-get install -y mongodb-server
RUN mkdir /home/vnv && mv /root/software/vnv/examples /home/vnv/examples

COPY ./launch.sh ./launch.sh
CMD ["./launch.sh" , "password" ]
