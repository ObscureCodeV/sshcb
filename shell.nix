let
  pkgs = import <nixpkgs> {};
in
pkgs.mkShell {
  buildInputs = with pkgs; [
    cmake
    gnumake
    gcc
    gdb
    pkg-config  # Обязателен для поиска libssh2
    libssh2     # Сама библиотека
    zlib        # libssh2 часто зависит от zlib, лучше добавить явно
  ];
}
