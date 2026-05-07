{
  description = "Shell for PrioriSea";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default";
    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
    };
    flux-core = {
      url = "path:./nix";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.systems.follows = "systems";
      inputs.flake-utils.follows = "flake-utils";
    };
  };
  outputs = {self, nixpkgs, flake-utils, flux-core, ...}:
  flake-utils.lib.eachDefaultSystem(system:
    let
      pkgs = nixpkgs.legacyPackages.${system};
      core = flux-core.packages.${system}.default;
    in {
      formatter = pkgs.alejandra;
      devShells.default = pkgs.mkShell {
        packages = [
          pkgs.clang-tools
          pkgs.jansson
          pkgs.fmt
          core
          self.packages.${system}.default
        ];
        shellHook = ''
            export CPATH="$PWD/src/lib/tinyexpr:$PWD/src/common:$CPATH"
            load-sea-priority() {
                flux jobtap load ${self.packages.${system}.default}/lib/flux/job-manager/plugins/sea_priority.so
                flux jobtap load ${self.packages.${system}.default}/lib/flux/job-manager/plugins/sea_debug.so
            }
            remove-sea-priority() {
                flux jobtap remove sea_priority.so
                flux jobtap remove sea_debug.so
            }
            echo "Run 'load-sea-priority' to load the jobtap plugin."
            echo "Run 'remove-sea-priority' to remove the jobtap plugin."
          '';

      };
      packages.default = pkgs.stdenv.mkDerivation {
        pname = "priorisea";
        version = "0.1.0";
        src = ./.;

        nativeBuildInputs = with pkgs; [
          autoreconfHook
          pkg-config
        ];

        buildInputs  = with pkgs; [
          core
          jansson
          fmt
        ];

        buildPhase = "make -j$NIX_BUILD_CORES";
        installPhase = "make install";
      };
    });
}
