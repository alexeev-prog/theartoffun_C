{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
	name = "c-dev-env";

	nativeBuildInputs = with pkgs; [
		gcc
		gnumake
		binutils
		pkg-config
		valgrind
		gdb
		doxygen
		python3
	];

	buildInputs = with pkgs; [
		glibc
		zlib
	];

	shellHook = ''
		export CC=gcc
	'';
}
