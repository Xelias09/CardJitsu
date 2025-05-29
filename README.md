# Projet CardJitsu - Structure et Organisation

Ce projet repose sur une architecture modulaire pour gérer différents périphériques connectés à un Raspberry Pi via le boîtier JoyPi. Le code est écrit en C, avec une gestion propre des bibliothèques, des tests unitaires et de la compilation croisée pour architecture ARM.

---

## 🗂 Arborescence générale

```
CardJitsu/
├── bin/                  # Binaires générés (tests, programmes)
├── include/              # Headers des bibliothèques personnelles (7seg.h, data.h...)
├── libs/                 # Sources C des bibliothèques personnelles (7seg.c, rfid.c...)
├── data/
│   └── gpio/
│       ├── include/      # Headers externes (wiringPi, Devlib, etc.)
│       ├── lib/          # Bibliothèques compilées (.so)
│       └── bin/          # Binaire gpio
├── src/
│   └── test_unitaires/   # Tests unitaires des périphériques et bibliothèques (test_7segments.c...)
├── Makefile              # Makefile principal pour projet final
```

---

## 🛠 Les différents Makefiles

### 1. `libs/Makefile`

Ce fichier permet de compiler les bibliothèques dynamiques `.so` pour chaque périphérique (libseven\_seg.so, etc.). Il utilise `wiringPi` en lien dynamique.

Commande typique :

```sh
make lib7seg.so
```

### 2. `src/test_unitaires/Makefile`

Ce fichier compile les tests unitaires pour les bibliothèques et périphériques. Les exécutables sont placés dans `data/gpio/bin`.

Commande typique :

```sh
make -lib7seg
```

Inclut aussi une commande `upload` pour transférer les binaires sur la Raspberry Pi.

### 3. `Makefile` racine (à venir)

Ce fichier permettra de :

* Compiler les `.so`
* Compiler le projet final (serveur, application, etc.)

---

## ⚙ Compilation croisée

Le projet utilise `arm-linux-gnueabihf-gcc` pour compiler vers Raspberry Pi. Le chemin du compilateur est défini dans chaque `Makefile` via la variable `CC`.

---

## ✅ Bonnes pratiques

* Les headers perso vont dans `include/`
* Les headers externes vont dans `data/gpio/include/`
* Les `.so` sont dans `data/gpio/lib/`
* Les binaires sont dans `bin/`
* Les sources de lib sont dans `libs/`
* Les tests sont dans `src/test_unitaires/`

---

À compléter : description du projet final, fonctionnement du serveur, protocole de communication, etc.
## Merci de mettre à jour `version.md` régulièrement (au moins à chaque fin de session de travail).
