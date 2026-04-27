# Dockerfile para entorno de desarrollo y compilación de FY-Engine
FROM ubuntu:24.04

# Evitar prompts interactivos durante la instalación
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependencias del sistema, compiladores y herramientas
# Incluye dependencias para GLFW (X11/Wayland) y Vulkan
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    curl \
    python3 \
    pkg-config \
    libvulkan-dev \
    vulkan-tools \
    vulkan-validationlayers \
    glslc \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxext-dev \
    libwayland-dev \
    libxkbcommon-dev \
    && rm -rf /var/lib/apt/lists/*

# Configurar el directorio de trabajo
WORKDIR /app

# Comando por defecto al levantar el contenedor (compila la versión debug)
CMD ["sh", "-c", "cmake --preset linux-ninja-debug && cmake --build --preset linux-ninja-debug"]
