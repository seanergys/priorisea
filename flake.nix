{
  description = "Shell for PrioriSea";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default";
    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
    };
  };
  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        version = "0.84.0";
        flux-core = pkgs.stdenv.mkDerivation {
          pname = "flux-core";
          inherit version;
          src = pkgs.fetchurl {
            url = "https://github.com/flux-framework/flux-core/releases/download/v${version}/flux-core-${version}.tar.gz";
            sha256 = "sha256-bPRdQY/wIes4MAyllFAYBOGrdPyKq8L9EDAj4ShLw7w=";
          };

          nativeBuildInputs = with pkgs; [
            autoconf
            automake
            libtool
            gnumake
            pkg-config
            bash
          ];

          buildInputs  = with pkgs; [
            zeromq
            libuuid
            jansson
            lz4
            libarchive
            hwloc
            sqlite
            lua51Packages.lua
            lua51Packages.luaposix
            (python312.withPackages (ps: with ps; [
              cffi
              ply
              pyyaml
              setuptools
              sphinx
            ]))
            aspell
            aspellDicts.en
            time
            valgrind
            mpich
            jq
            ncurses
          ];

          postPatch = ''
            patchShebangs .
          '';

          buildPhase = "make -j$NIX_BUILD_CORES";
          installPhase = "make install";
        };
      in {
        packages.default = flux-core;

        formatter = pkgs.alejandra;

        devShells.default = pkgs.mkShell {
          packages = [
            pkgs.clang-tools
            pkgs.jansson
            pkgs.fmt
            flux-core
          ];
          shellHook = ''
              export CPATH="$PWD/external/croncpp/include:$PWD/external/CLI11/include:$PWD/external/tinyexpr:$PWD/src:$CPATH"
          '';
        };
      }
    );
}
