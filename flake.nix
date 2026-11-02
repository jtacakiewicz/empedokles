{
  description = "Vulkan development environment (x86_64-linux + aarch64-darwin)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-darwin" ] (system:
      let
        pkgs = import nixpkgs { inherit system; };
        inherit (pkgs) stdenv lib;

        linux-libs = lib.optionals stdenv.hostPlatform.isLinux (with pkgs; [
          libx11
          libx11.dev
          libxext
          libxrandr
          libxi
          libxcursor
          libxinerama
          libxrender
          libxfixes
          systemd
          vulkan-tools-lunarg
          renderdoc
          valgrind
          kdePackages.kcachegrind
          libGL
          libGLU
          gdb
        ]);

        darwin-libs = lib.optionals stdenv.hostPlatform.isDarwin (with pkgs; [
          apple-sdk_26
          moltenvk
          libcxx
        ]);

        lib-path = lib.makeLibraryPath (with pkgs; [
          glfw
          openal
          vulkan-loader
        ] ++ lib.optionals stdenv.hostPlatform.isLinux [ libGL libGLU ]
          ++ lib.optionals stdenv.hostPlatform.isDarwin [ moltenvk ]
        );
      in
      {
        devShells.default = pkgs.mkShell {
          name = "vulkan-env";

          packages = with pkgs; [
            pkg-config
            glfw
            freetype
            shaderc
            glslang
            tracy
            typos
            pre-commit
          ];

          buildInputs = with pkgs; [
            vulkan-loader
            vulkan-tools
            vulkan-validation-layers
            libvorbis.dev
            libogg.dev
            openal
            glm
            glfw
            sfml
            freetype
            cmake
            ccls
            clang-tools
            clang
          ] ++ linux-libs ++ darwin-libs;

          VULKAN_SDK = "${pkgs.vulkan-headers}";
          VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
          OpenAL_PATH = "${pkgs.openal}";

          shellHook = ''
            echo "--- Vulkan Development Environment (${stdenv.hostPlatform.system}) ---"

            # Platform-specific library path exports
            ${lib.optionalString stdenv.hostPlatform.isLinux ''
              export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${lib-path}"
            ''}
            ${lib.optionalString stdenv.hostPlatform.isDarwin ''
              export DYLD_FALLBACK_LIBRARY_PATH="$DYLD_FALLBACK_LIBRARY_PATH:${lib-path}"
            ''}

            alias vkinfo='vulkaninfo'
          '';
        };
      });
}
