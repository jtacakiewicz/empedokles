{ pkgs ? import <nixpkgs> { } }:

let
  inherit (pkgs) stdenv lib;

  linux-libs = with pkgs; lib.optionals stdenv.isLinux [
    vulkan-loader
    libx11
    libx11.dev
    libxext
    libxrandr
    libxi
    libxcursor
    libxinerama
    libxrender
    libxfixes
    mesa
    systemd
  ];

  darwin-libs = with pkgs; lib.optionals stdenv.isDarwin (with darwin.apple_sdk.frameworks; [
    Cocoa
    CoreVideo
    IOKit
    moltenvk
  ]);

  lib-path = lib.makeLibraryPath (with pkgs; [
    glfw
    libGL
    openal
  ] ++ linux-libs);

in
pkgs.mkShell {
  name = "vulkan-env";

  packages = with pkgs; [
    pkg-config
    glfw
    freetype
    shaderc
    glslang
    renderdoc
    tracy
    valgrind
    typos
    pre-commit
    kdePackages.kcachegrind
  ];

  buildInputs = with pkgs; [
    libvorbis.dev
    libogg.dev
    libGL
    libGLU
    openal
    glm
    glfw
    csfml
    freetype
    vulkan-headers
    vulkan-validation-layers
    vulkan-tools
    vulkan-tools-lunarg
    cmake
    gdb
  ] ++ linux-libs ++ darwin-libs;

  VULKAN_SDK = "${pkgs.vulkan-headers}";
  VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";

  shellHook = ''
    echo "--- Vulkan Development Environment (${stdenv.hostPlatform.system}) ---"

    # Platform-specific library path exports
    ${if stdenv.isLinux then ''
      export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${lib-path}"
    '' else if stdenv.isDarwin then ''
      export DYLD_LIBRARY_PATH="$DYLD_LIBRARY_PATH:${lib-path}"
    '' else ""}

    alias vkinfo='vulkaninfo'
  '';
}
