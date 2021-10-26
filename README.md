# Among us decentralized

[![Travis Build Status](https://travis-ci.com/Benjamin-Loison/Among-us-decentralized.svg?branch=main)](https://app.travis-ci.com/Benjamin-Loison/Among-us-decentralized)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/Benjamin-Loison/Among-us-decentralized?branch=main&svg=true)](https://ci.appveyor.com/project/Benjamin-Loison/Among-us-decentralized)

Language: C++  
Graphics library: Qt

Crossplatform

## Objectives

- [X] map/players (assets from native game for instance) - https://github.com/Perfare/AssetStudio
- [X] Show the map
- [X] Handle player movement
- [ ] Maintain game state
- [ ] Display other players
- [ ] Display name tags
- [ ] Menu (main menu and "meetings")
- [ ] Network: peer-to-peer with secret (visibility) - using OpenSSL/Helib for cryptography
- [ ] Tasks:
    - [X] Fix Wiring (needs debugging)
    - [ ] Other tasks
    - [ ] Set places for tasks
- [ ] vocal chat during "meetings" (not priority)
- [ ] extra anti cheat (not priority)

Realtime game similar to (C++ Qt decentralized...) https://github.com/Benjamin-Loison/Travian-blockchained

## Building
Requires Qt 5.12.11 (or probably any other recent version of Qt 5). To build, you can either:
- open `AmongUsDecentralized.pro` in Qt Creator, then click "Build Project",
- or, from a build directory, run `qmake path/to/AmongUsDecentralized.pro` then `make`.

-----------

5.1 s pour faire upper engine vers cafetaria (arrêt table) en "frotant" le mur du bas 2 431 pixels d'après ma mesure relativement précise donc une vitesse de 477 pixels par seconde

make some tests (by a human for tasks for instance but precise a procedure)

faire une option (booléenne dans le code par exemple) pour ne pas utiliser tout ce qui touche à QMediaPlayer qui pose parfois problème chez Benjamin par exemple.

Pour utiliser le multijoueur il est supposé que lorsque vous vous connecté à un autre joueur son port soit ouvert.
<!-- Pour utiliser le multijoueur il est supposé que chaque joueur ait son port ouvert--><!-- ou qu'à l'aide de l'UPnp se soit possible de l'ouvrir. -->
<!-- Pour utiliser le multijoueur il est supposé qu'au moins un joueur ait son port ouvert ou qu'à l'aide de l'UPnP se soit possible de l'ouvrir. -->
<!-- avec l'IPv6 les ports sont ouverts par défaut ?! -->
