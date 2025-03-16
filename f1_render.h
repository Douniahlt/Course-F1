/*!\file f1_render.h
 * \brief Fonctions de rendu pour le jeu F1
 */

 #ifndef F1_RENDER_H
 #define F1_RENDER_H
 
 #include "f1_ai.h"  /* Ajout de l'inclusion pour les définitions des IA */
 
 /* Initialisation du rendu */
 void initRendering(void);
 
 /* Fonction de dessin */
 void draw(void);
 
 /* Fonctions de dessin spécifiques */
 void drawCar(Car *car);
 void drawTrack(void);
 void setCamera(void);
 void drawWaypoints(void);  /* Ajout pour voir les points de passage des IA */
 
 #endif /* F1_RENDER_H */