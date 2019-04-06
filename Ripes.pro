TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = src app test
test.depends = src
app.depends = src
