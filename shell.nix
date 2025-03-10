with import <nixpkgs> {};

pkgs.mkShell {
  nativeBuildInputs = [
    gcc
    ncurses
    doxygen
    gdb
  ];
 
  buildInputs = [stdenv ncurses];
}
