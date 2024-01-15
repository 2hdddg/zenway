FROM ubuntu:22.04

# Baseline and software-properties-common to be able to add new repository
RUN apt-get update && \
    apt-get install -y software-properties-common build-essential pkg-config meson ninja-build && \
    apt-get clean
# Build dependencies
RUN add-apt-repository universe && apt-get update && \
    apt-get install -y libfmt-dev wayland-protocols libwayland-client0 libwayland-dev libcairo2-dev libpango1.0-dev libxkbcommon-dev liblua5.4-dev libpulse-dev && \
    apt-get clean

# Set manually when running locally or set by Github actions/checkout
ENV GITHUB_WORKSPACE=/code

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
CMD ["/entrypoint.sh"]
