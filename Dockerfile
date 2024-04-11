FROM gcc:9.5.0-bullseye

RUN apt update && apt upgrade -y
#RUN apt install make -y
#RUN apt install iputils-ping -y

COPY ./ ./

RUN make

CMD ["./ft_ping", "localhost"]
