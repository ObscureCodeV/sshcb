let
  pkgs = import <nixpkgs> {};
in
pkgs.mkShell {
  buildInputs = with pkgs; [
    cmake
    gnumake
    libssh.dev
    zig
    pkg-config
  ];
  SSHCB_PORT = "2456";
  SSHCB_SERVER_INTERFACE_IP = "127.0.0.1";
  SSHCB_KNOWNHOST = "test-res/known_hosts";
  SSHCB_AUTHORIZED_KEYS = "test-res/known_users";
  SSHCB_SERVER_PUBKEY_FILE = "test-res/server.pub";
  SSHCB_SERVER_PRIVKEY_FILE = "test-res/server";
  SSHCB_CLIENT_PUBKEY_FILE = "test-res/client.pub";
  SSHCB_CLIENT_PRIVKEY_FILE = "test-res/client";
}
