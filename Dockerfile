FROM alpine:latest AS builder

RUN apk add --no-cache g++ make wget

WORKDIR /app

RUN wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
RUN wget https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp

COPY main.cpp .
RUN g++ -O3 -std=c++11 -pthread -static main.cpp -o net_radar

FROM alpine:latest

WORKDIR /app

RUN apk add --no-cache arp-scan

COPY --from=builder /app/net_radar .
COPY index.html .

EXPOSE 8080

CMD ["./net_radar"]
