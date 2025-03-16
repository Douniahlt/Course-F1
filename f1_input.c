/*!\file f1_input.c
 * \brief Implémentation des fonctions de gestion des entrées pour le jeu F1
 */

 #include <GL4D/gl4duw_SDL2.h>
 #include <stdio.h>
 #include <stdlib.h>
 
 #include "f1_types.h"
 #include "f1_input.h"
 
 /* Gestion des touches enfoncées */
 void keydown(int keycode) {
     switch(keycode) {
         case SDLK_v:
             /* Changer de vue */
             _viewMode = (_viewMode + 1) % 4; /* Cycle entre les 4 vues */
             printf("Vue changée: %d\n", _viewMode); /* Pour déboguer */
             break;
 
         case SDLK_UP:
             /* Accélération */
             _playerCar.speed += _playerCar.acceleration;
             if(_playerCar.speed > _playerCar.maxSpeed)
                 _playerCar.speed = _playerCar.maxSpeed;
             break;
             
         case SDLK_DOWN:
             /* Freinage/Marche arrière */
             _playerCar.speed -= _playerCar.acceleration;
             if(_playerCar.speed < -_playerCar.maxSpeed/2)
                 _playerCar.speed = -_playerCar.maxSpeed/2;
             break;
             
         case SDLK_LEFT:
             /* Tourner à gauche */
             _playerCar.steering = 1.0f;
             break;
             
         case SDLK_RIGHT:
             /* Tourner à droite */
             _playerCar.steering = -1.0f;
             break;
             
         case SDLK_ESCAPE:
             /* Quitter */
             exit(0);
             break;
     }
 }
 
 /* Gestion des touches relâchées */
 void keyup(int keycode) {
     switch(keycode) {
         case SDLK_LEFT:
         case SDLK_RIGHT:
             /* Arrêter de tourner */
             _playerCar.steering = 0.0f;
             break;
     }
 }