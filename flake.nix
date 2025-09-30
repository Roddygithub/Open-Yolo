{
  description = "Open-Yolo - Gestionnaire de curseurs personnalisés pour Linux";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        
        open-yolo = pkgs.stdenv.mkDerivation {
          pname = "open-yolo";
          version = "1.0.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            cmake
            pkg-config
            wrapGAppsHook
          ];

          buildInputs = with pkgs; [
            gtkmm3
            SDL2
            SDL2_image
            libGL
            glew
            xorg.libX11
            xorg.libXext
            xorg.libXfixes
            xorg.libXi
            xorg.libXrandr
            xorg.libXcursor
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
            "-DBUILD_TESTS=OFF"
          ];

          meta = with pkgs.lib; {
            description = "Gestionnaire de curseurs personnalisés natif pour Linux";
            longDescription = ''
              Open-Yolo est une alternative native Linux à YoloMouse.
              Il permet de personnaliser les curseurs avec support des animations,
              multi-écrans, HiDPI et rendu GPU optimisé.
            '';
            homepage = "https://github.com/Roddygithub/Open-Yolo";
            license = licenses.mit;
            platforms = platforms.linux;
            maintainers = [ ];
          };
        };

      in
      {
        packages = {
          default = open-yolo;
          open-yolo = open-yolo;
        };

        apps = {
          default = {
            type = "app";
            program = "${open-yolo}/bin/OpenYolo";
          };
          open-yolo = {
            type = "app";
            program = "${open-yolo}/bin/OpenYolo";
          };
        };

        devShells.default = pkgs.mkShell {
          buildInputs = open-yolo.buildInputs ++ open-yolo.nativeBuildInputs ++ [
            pkgs.gdb
            pkgs.valgrind
            pkgs.clang-tools
          ];

          shellHook = ''
            echo "Open-Yolo Development Environment"
            echo "=================================="
            echo ""
            echo "Available commands:"
            echo "  cmake -B build -DCMAKE_BUILD_TYPE=Debug"
            echo "  cmake --build build"
            echo "  ./build/src/OpenYolo"
            echo ""
          '';
        };
      }
    );
}
