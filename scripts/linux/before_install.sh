export CC=gcc-6
export CXX=g++-6

git fetch --unshallow
git pull --tags
git describe

sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-6 90

source /opt/qt*/bin/qt*-env.sh
