FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Actualizar e instalar dependencias basicas y de compilacion
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gdb \
    git \
    wget \
    curl \
    unzip \
    tar \
    pkg-config \
    # Dependencias para GLFW y GUI en Linux (X11 y Wayland)
    libx11-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxi-dev \
    libxinerama-dev \
    libwayland-dev \
    libxkbcommon-dev \
    # Librerias extra
    mesa-utils \
    vulkan-tools \
    libvulkan-dev \
    && rm -rf /var/lib/apt/lists/*

# Instalar Vulkan SDK (LunarG)
RUN wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | tee /etc/apt/trusted.gpg.d/lunarg.asc \
    && wget -qO /etc/apt/sources.list.d/lunarg-vulkan-noble.list http://packages.lunarg.com/vulkan/lunarg-vulkan-noble.list \
    && apt-get update \
    && apt-get install -y vulkan-sdk \
    && rm -rf /var/lib/apt/lists/*

# Crear un usuario no-root para desarrollo
ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME
WORKDIR /workspace

CMD ["/bin/bash"]
