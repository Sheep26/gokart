docker build -t gokart ./
if [[ "$*" == *-d* ]]; then
    docker run -p 1935:1935 -p 8001:8001 -p 8002:8002 -d gokart
else
    docker run -p 1935:1935 -p 8001:8001 -p 8002:8002 gokart
fi