#!/bin/sh -l
set -e
cd $GITHUB_WORKSPACE
meson build-docker-ubuntu
ninja -C build-docker-ubuntu
