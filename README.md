
# Projet CardJitsu - Structure et Organisation

> 📌 Ce projet contient une application serveur et des bibliothèques pour contrôler les périphériques du boîtier JoyPi (LCD, 7 segments, RFID...).  
> Le README couvre la structure du projet, la compilation croisée, les tests unitaires et les conventions.

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

Permet de compiler les bibliothèques dynamiques `.so` pour chaque périphérique (`libseven_seg.so`, `liblcd_custom.so`, etc.).  
Utilise `wiringPi` et `wiringPiDev` en lien dynamique.

Commande typique :
```sh
make lib7seg.so
make liblcd_custom.so
```

Inclut aussi une règle `upload` pour envoyer les `.so` dans `/home/pi/Desktop/lib`.

---

### 2. `src/test_unitaires/Makefile`

Compile les tests unitaires associés à chaque lib. Les binaires sont générés dans `bin/`.  
Inclut une commande `upload` pour transférer les binaires de test sur le Raspberry Pi.

Commande typique :
```sh
make segment
make lcd
make rfid
make upload
```

---

### 3. `Makefile` racine (à venir)

Ce fichier aura pour but de :
- Compiler toutes les bibliothèques dynamiques
- Compiler les binaires du projet final (serveur, application principale)
- Fournir des cibles `make all`, `make clean`, etc.

---

## ⚙ Compilation croisée

Le projet utilise `arm-linux-gnueabihf-gcc` pour compiler vers Raspberry Pi.  
Le chemin est défini dans chaque `Makefile` avec la variable `CC`.

---

## 📦 Tests des bibliothèques

### Test de `lib7seg.so` :
- Recompiler la lib :
  ```sh
  make lib7seg.so
  ```
- `make upload` (ou copier manuellement via `scp`)
- Copier la lib dans `/usr/lib` sur le RPi :
  ```sh
  sudo cp /home/pi/Desktop/lib/lib7seg.so /usr/lib/
  ```
- Compiler et uploader `test_lib_7seg`
- Lancer : `./test_lib_7seg`

---

### Test de `liblcd_custom.so` :
- Recompiler la lib :
  ```sh
  make liblcd_custom.so
  ```
- `make upload`
- Copier dans `/usr/lib` :
  ```sh
  sudo cp /home/pi/Desktop/lib/liblcd_custom.so /usr/lib/
  ```
- Compiler et uploader `test_lib_lcd`
- Lancer : `./test_lib_lcd`
- ✅ Le scroll des deux lignes se fait désormais **en parallèle**
- ⚠️ Les accents ne sont pas pris en charge (utiliser des caractères non accentués)

---

## ✅ Bonnes pratiques

- Headers perso : `include/`
- Headers externes : `data/gpio/include/`
- Bibliothèques dynamiques : `data/gpio/lib/`
- Binaires : `bin/`
- Sources de lib : `libs/`
- Tests : `src/test_unitaires/`

---

## 🚧 Partie Serveur (à compléter)

Le projet principal consiste en une application serveur :

- Écrit en C
- Permet l'identification de joueurs par badge RFID
- Gère des sessions via sockets TCP
- Protocole : à définir (message/ack, UID, inscription, etc.)

À compléter avec :
- Structure des fichiers du serveur
- Protocole réseau entre client (valise) et serveur
- Format des messages, gestion des cartes, session joueur

---

## 📄 Pense-bête

Merci de mettre à jour `version.md` régulièrement (au moins à chaque fin de session de travail).