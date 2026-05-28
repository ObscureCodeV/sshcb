{
  description = "SSHCB develop";

  inputs = {
    nixpkgs.url = "github:NixOs/nixpkgs/nixos-25.11";
  };

  outputs = { self, nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import <nixpkgs> {};
      currentDir = builtins.toString ./.;
      testDir = "${currentDir}/test/test-res";
    in {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          cmake
          gnumake
          libssh.dev
          zig
          pkg-config
         ];
         SSHCB_PORT = "2456";
         SSHCB_SERVER_INTERFACE_IP = "127.0.0.1";
         SSHCB_KNOWNHOST = "${testDir}/known_hosts";
         SSHCB_AUTHORIZED_KEYS = "${testDir}/known_users";
         SSHCB_SERVER_PUBKEY_FILE = "${testDir}/server.pub";
         SSHCB_SERVER_PRIVKEY_FILE = "${testDir}/server";
         SSHCB_CLIENT_PUBKEY_FILE = "${testDir}/client.pub";
         SSHCB_CLIENT_PRIVKEY_FILE = "${testDir}/client";
      }
    };
  };
}

