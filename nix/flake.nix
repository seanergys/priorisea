{
  description = "flux-core";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/a4bf06618f0b5ee50f14ed8f0da77d34ecc19160";
    systems.url = "github:nix-systems/default";
    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
    };
  };
  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }:
    let
     version = "0.84.0";
    in flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        packages.default = pkgs.stdenv.mkDerivation {
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
      }
    );
}
