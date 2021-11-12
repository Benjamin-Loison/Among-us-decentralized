# Among us decentralized

[![Travis Build Status](https://travis-ci.com/Benjamin-Loison/Among-us-decentralized.svg?branch=main)](https://app.travis-ci.com/Benjamin-Loison/Among-us-decentralized)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/Benjamin-Loison/Among-us-decentralized?branch=main&svg=true)](https://ci.appveyor.com/project/Benjamin-Loison/Among-us-decentralized)
[![Mac](https://github.com/Benjamin-Loison/Among-us-decentralized/actions/workflows/cpp.yml/badge.svg)](https://github.com/Benjamin-Loison/Among-us-decentralized/actions/workflows/cpp.yml)

Language: C++  
Graphics library: Qt

Crossplatform

## Objectives

Course schedule: http://www.lsv.fr/~chatain/enseignement/GL/

- [X] map/players (assets from native game for instance) - https://github.com/Perfare/AssetStudio
- [X] Show the map
- [X] Handle player movement
- [X] Maintain game state
- [X] Display other players
- [X] Display name tags
- [ ] Menu (main menu and "meetings")
- [ ] Network: peer-to-peer with secret (visibility) - using OpenSSL/Helib for cryptography
- [ ] Tasks:
    - [X] Fix Wiring
    - [X] Asteroids
    - [ ] Swipe Card
    - [ ] Other tasks?
    - [X] Set places for tasks (currently not very user-friendly, as players need to know where the tasks are)
- [ ] Vocal chat during "meetings" (not prioritary)
- [ ] Extra anti cheat (not prioritary)
- [ ] Packaging
- Tests :
    - Continuous integration:
        - [X] Windows (Helib incompatible?)
        - [X] Linux
        - [X] MacOS
    - [ ] Other tests

Realtime game similar to (C++ Qt decentralized...) https://github.com/Benjamin-Loison/Travian-blockchained

## Building
Requires Qt 5.12.11 (or probably any other recent version). To build on Ubuntu, you can either:
- open `AmongUsDecentralized.pro` in Qt Creator, then click "Build Project",
- or, from a build directory, run `qmake path/to/AmongUsDecentralized.pro` then `make`.

-----------

For network maybe just working with Discord Rich Presence with a high level of abstraction would be nice

5.1 s pour faire upper engine vers cafetaria (arrêt table) en "frotant" le mur du bas 2 431 pixels d'après ma mesure relativement précise donc une vitesse de 477 pixels par seconde

make some tests (by a human for tasks for instance but precise a procedure)

faire une option (booléenne dans le code par exemple) pour ne pas utiliser tout ce qui touche à QMediaPlayer qui pose parfois problème chez Benjamin par exemple.

Pour utiliser le multijoueur il est supposé que lorsque vous vous connectez à un autre joueur son port est ouvert.
<!-- Pour utiliser le multijoueur il est supposé que chaque joueur ait son port ouvert--><!-- ou qu'à l'aide de l'UPnp se soit possible de l'ouvrir. -->
<!-- Pour utiliser le multijoueur il est supposé qu'au moins un joueur ait son port ouvert ou qu'à l'aide de l'UPnP se soit possible de l'ouvrir. -->
<!-- avec l'IPv6 les ports sont ouverts par défaut ?! -->


----------

Ideas for protocols and methods 
killed people wait at their bodies the round's end
dead people are declared at the end of each round, then becomes phantoms: they only send their info to other phantom    s,but they can still make tasks

chacun choisit un antécédent de fonction de hachage aléatoirement
chacun envoie aux autres son haché
Le random commun est donné par les hachés
Les antécédents définissent l'ordre des taches de chacun
En fin de partie, on peut vérifier les antécédents de tout de monde

Due to helib limitations, it is not possible to know whether 2 players can see each other, just the distance between     both
Thus, when they reach a certain distance, they agree on a a protocol to see each other in clear


dealing with doors is optionnal


the main objective is to make sure that no one cheated at the end of the round/game                                      
