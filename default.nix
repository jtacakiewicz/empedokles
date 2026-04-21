with import <nixpkgs> {};

let
    lib-path = with pkgs; lib.makeLibraryPath [
        vulkan-loader
        glfw
        libx11.dev
        libGL
        openal
    ];
in
mkShell {
    name = "vulkan";
    packages = [
        pkg-config
        glfw
        freetype
        shaderc             # GLSL to SPIRV compiler - glslc
        glslang
        renderdoc           # Graphics debugger
        tracy               # Graphics profiler
        valgrind
        typos
        pre-commit
        kdePackages.kcachegrind
    ];

    buildInputs = with pkgs; [
        systemd
        libvorbis.dev
        libogg.dev
        libx11
        libx11.dev
        libxext
        libxext.dev
        libxrandr
        libxrandr.dev
        libxi
        libxi.dev
        libxcursor
        libxcursor.dev
        libxinerama
        libxinerama.dev
        libxrender
        libxrender.dev
        libxfixes
        libxfixes.dev
        mesa
        libGL
        libGLU
        openal
        glm
        glfw
        csfml
        freetype
        vulkan-loader
        vulkan-headers
        vulkan-validation-layers
        vulkan-tools        # vulkaninfo
        vulkan-tools-lunarg # vkconfig
        cmake
    ];

    VULKAN_SDK = "${vulkan-headers}";
    VK_LAYER_PATH = "${vulkan-validation-layers}/share/vulkan/explicit_layer.d";
    shellHook = ''
        # Prepend or append directories to the PATH
        export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${lib-path}"
    '';
}
