cd docker
docker build -f docker/Dockerfile.env -t vnv_env .

cd ..
docker build -f docker/Dockerfile -t vnv_base .


