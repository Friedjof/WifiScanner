{ pkgs ? import <nixpkgs> {} }:
let
in
  pkgs.mkShell {
    buildInputs = with pkgs; [
      platformio
      avrdude
      platformio-core.udev
    ];
}