## Build docker image

To build latest version of Ripes from `master` branch issue the command

```bash
# Ripes from mortbopet repository
docker build --rm --tag ripes -f ripes.dockerfile .

# Ripes from current directory (start from root directory)
docker build --rm --tag ripes -f ./docker/ripes_new.dockerfile .
```

Or to build from specific branch

```bash
docker build --rm --build-arg BRANCH=my_branch --tag ripes:latest -f ripes.dockerfile .
```

## Run docker container

Enable external connection to your X server (run for each session)
```bash
xhost +
```
**Note:** you may add this line to `~/.xsessionrc` file to avoid having to run it for each session

After that, issue the command

```bash
# change `ripes:latest` if it hasn't specific name
docker run --rm -it --net=host -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix ripes:latest
```
