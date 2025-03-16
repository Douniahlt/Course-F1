# F1 Racing Game

Un jeu de course de Formule 1 en 3D développé avec la bibliothèque GL4D.

## Description

F1 Racing Game est un simulateur de course où le joueur peut piloter une voiture de Formule 1 sur différents circuits contre des adversaires contrôlés par l'IA. Le jeu propose un rendu 3D réaliste des voitures et des circuits, ainsi qu'un système de physique permettant des collisions et des dégâts réalistes.

## Fonctionnalités

- Pilotage d'une voiture de Formule 1
- Système de collision et de dégâts affectant les performances des voitures
- Intelligence artificielle pour les adversaires
- Plusieurs vues de caméra

## Contrôles

- **Flèche Haut** : Accélérer
- **Flèche Bas** : Freiner/Reculer
- **Flèche Gauche** : Tourner à gauche
- **Flèche Droite** : Tourner à droite
- **V** : Changer de vue (course, face, arrière, dessus)
- **Échap** : Quitter le jeu

## Structure du Projet

Le projet est organisé en plusieurs modules :

- **f1_main.c** : Point d'entrée principal du jeu
- **f1_render.c** : Gestion du rendu graphique des voitures et du circuit
- **f1_physics.c** : Système de physique et détection de collision
- **f1_input.c** : Gestion des entrées utilisateur
- **f1_ai.c** : Intelligence artificielle des concurrents
- **f1_track.c** : Système de gestion des circuits
- **f1_types.h** : Définition des structures de données utilisées

## Prérequis

- Un système d'exploitation Linux, macOS ou Windows
- La bibliothèque GL4D (GL4Dummies)
- SDL2
- Un compilateur C compatible (GCC ou Clang)

## Compilation

Le projet utilise un Makefile pour la compilation. Pour compiler le jeu :

```bash
make
```

Pour nettoyer les fichiers de compilation :

```bash
make clean
```

Pour compiler et exécuter le jeu :

```bash
make run
```

## Perspectives d'Évolution

- Amélioration des circuits avec textures et environnements détaillés
- Mode multijoueur en écran partagé ou en réseau
- Système de chronométrage et classement des tours
- Interface utilisateur améliorée avec HUD pendant la course
- Mode championnat avec plusieurs courses


## Crédits

Développé avec la bibliothèque GL4D (GL4Dummies) créée par Farès Belhadj.
