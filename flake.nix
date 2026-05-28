{
  description = "SSHCB develop";

  inputs = {
    nixpkgs.url = "github:NixOs/nixpkgs/nixos-25.11";
  };

  outputs = { self, nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          cmake
          gnumake
          gcc
          gdb
          libssh.dev
          zig
          pkg-config
        ];
        shellHook = ''
          export testRes="$PWD/test/test-res"
          export SSHCB_PORT="2456"
          export SSHCB_KNOWNHOST="$testRes/known_hosts"
          export SSHCB_AUTHORIZED_KEYS="$testRes/known_users"
          export SSHCB_SERVER_PUBKEY_FILE="$testRes/server.pub"
          export SSHCB_SERVER_PRIVKEY_FILE="$testRes/server"
          export SSHCB_CLIENT_PUBKEY_FILE="$testRes/client.pub"
          export SSHCB_CLIENT_PRIVKEY_FILE="$testRes/client"
        '';
      };
  };
}

