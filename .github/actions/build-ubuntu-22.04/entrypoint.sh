#!/bin/sh -l
set -e
cd $GITHUB_WORKSPACE
meson build-docker-ubuntu-22.04
ninja -C build-docker-ubuntu-22.04
