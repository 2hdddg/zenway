FROM archlinux:latest

RUN pacman-key --init && pacman -Syu --noconfirm
# Baseline
# Use clang on arch
RUN pacman -S --noconfirm meson clang pkgconf
# Build depdendencies
RUN pacman -S --noconfirm wayland wayland-protocols fmt cairo pango libxkbcommon lua libpulse

# Set manually when running locally or set by Github actions/checkout
ENV GITHUB_WORKSPACE=/code

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
CMD ["/entrypoint.sh"]
