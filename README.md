# Quoridor

## Usage

`./install/server [-m SIZE] [-t SHAPE] <PLAYER_1_PATH> <PLAYER_2_PATH>`

## Description

This program compute a Quoridor game between two artificial intelligences, and display the board all along the game.

Some players are available in the `./src/ia` directory.

## Options

* -m : define the board size (default: 15)

* -t : define the board shape between SQUARE, TORIC (not available), H (not available), and SNAKE (not available) (default: SQUARE)

## Compilation

* `make` : compilation of source files

* `make install` : installation of the executables in the `install` directory

* `make game` : launch a game between two players with default parameters

* `make test` : compilation and execution of the tests

* `make doc` : generate the Doxygen documentation in the `doc` directory

* `make clean` : clean the output directories

## Links

- [Project's subject](https://www.labri.fr/perso/renault/working/teaching/projets/2020-21-S6-C-Quoridor.php)

