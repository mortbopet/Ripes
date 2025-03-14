## Build docker image

To build latest version of Ripes from `master` branch issue the command. Execute it from project root dir

```bash
docker build --rm --tag ripes -f ./docker/ripes.dockerfile .
```

Or to build from specific branch

```bash
docker build --rm --build-arg BRANCH=my_branch --tag ripes:latest -f ./docker/ripes.dockerfile .
```

## Run docker container

Enable external connection to your X server (run for each session)
```bash
xhost +
```
**Note:** you may add this line to `~/.xsessionrc` file to avoid having to run it for each session

After that, issue the command

```bash
docker run --rm -it --net=host -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix ripes:latest
```

## Run docker compose

In current service postgres db and pdAdmin are allowed:

To run docker compose type the command

```bash
docker compose up
```

You need to create .env file in docker directory to run compose. 

Example .env file:
```bash
PORT=5432
HOST=service_db
POSTGRES_DB=docker_service_db
POSTGRES_USER=docker_service
POSTGRES_PASSWORD=docker_service
PGADMIN_DEFAULT_EMAIL=pgadmin@mail.ru
PGADMIN_CONFIG_SERVER_MODE="False"
PGADMIN_DEFAULT_PASSWORD=password
PGADMIN_HOST_PORT=5051
```

After that pgAdmin will be available via 127.0.0.1:5051
