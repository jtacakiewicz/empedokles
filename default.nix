with import <nixpkgs> {};

let
    lib-path = with pkgs; lib.makeLibraryPath [
        vulkan-loader
        glfw
        xorg.libX11.dev
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
        xorg.libX11
        xorg.libX11.dev
        xorg.libXext
        xorg.libXext.dev
        xorg.libXrandr
        xorg.libXrandr.dev
        xorg.libXi
        xorg.libXi.dev
        xorg.libXcursor
        xorg.libXcursor.dev
        xorg.libXinerama
        xorg.libXinerama.dev
        xorg.libXrender
        xorg.libXrender.dev
        xorg.libXfixes
        xorg.libXfixes.dev
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
