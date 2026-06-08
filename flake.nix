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
        ];
        shellHook = ''
            export CPATH="$PWD/external/croncpp/include:$PWD/external/CLI11/include:$PWD/external/tinyexpr:$PWD/src:$CPATH"
        '';
      };
    });
}
