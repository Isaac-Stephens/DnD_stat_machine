#!/bin/bash

# Define color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

# Update package list
echo "Updating package lists..."
sudo apt update

# Install necessary packages
echo "Installing required packages..."
sudo apt install -y \
    g++ \
    cmake \
    make \
    pkg-config \
    libglfw3-dev \
    libvulkan-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    xorg-dev \
    mesa-vulkan-drivers \
    vulkan-validationlayers \
    vulkan-tools

# Verify Vulkan installation
echo "Verifying Vulkan installation..."
if ! command -v vulkaninfo &> /dev/null; then
    echo -e "${RED}ERROR: Vulkan not found. Please check your installation.${NC}"
    exit 1
else
    echo "Vulkan is installed correctly!"
fi
echo -e "${GREEN}|==========================================|${NC}"
echo -e "${GREEN}| All dependencies installed successfully! |${NC}"
echo -e "${GREEN}|==========================================|${NC}"

