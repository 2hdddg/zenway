#!/bin/sh -l
set -e

cd $GITHUB_WORKSPACE
meson build-docker-arch
ninja -C build-docker-arch

