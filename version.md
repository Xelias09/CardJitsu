# Historique des versions

Pensez à mettre la date et l'heure de vos modifs pour évaluer le temps passé sur le projet !

---

## globale
### 2025-05-28 23h45
- création d'un makefile pour les librairies, il est normalement prêt pour chaque librairie à compléter au besoin
- Ajout de upload dans chaque makefile attention a l'adresse du rpi sur vos partages
- Vous pouvez tester la librairie 7 segments :
  - recompiler la lib 
  - upload sur le rpi
  - copier la lib de /home/pi/Desktop/lib vers /usr/lib
  - compiler test_lib_7seg
  - upload
  - lancer le programme

---

## Détail des modifs

## Librairie pour 7 segments 
### 2025-05-28 23h45
- Correction du mapping des digits (saut de l’adresse 0x04 pour éviter les deux-points)
- Ajout du tableau `digit_addresses[]`
- Nettoyage complet avec `clear_display()` sur 16 registres

### 2025-05-28 21h00
- Initialisation du HT16K33
- Affichage de chiffres simples (0–9)
- Fonctions `init_display`, `display_digit`, `clear_display`, `display_number`

---

## Librairie pour lcd
(à compléter)

---

## Librairie pour matrice LED
### 2025-05-29 23h07
- Création de la librairie de la matrice et son fichier d'entête
- Création du programme test_matrice
- modifications des makefiles (test et libs)
- recompiler la lib matrice en dynamique
- compiler le programme test_matrice
- upload la lib et le test
- test sur la malette

---

## Librairie pour lecteur RFID
(à compléter)