/*!\file f1_render.c
 * \brief Implémentation des fonctions de rendu pour le jeu F1
 */

 #include <GL4D/gl4duw_SDL2.h>
 #include <GL4D/gl4dg.h>
 #include <GL4D/gl4du.h>
 #include <math.h>
 
 #include "f1_types.h"
 #include "f1_render.h"
 #include "f1_ai.h"  /* Pour accéder aux voitures IA */
 
 /* Prototype de fonction externe de f1_ai.h */
 extern Car* getAICar(int index);
 
 /* Variables pour les ressources OpenGL locales */
 static GLuint _track = 0;    /* Circuit */
 static GLuint _carBody = 0;  /* Corps de la voiture */
 static GLuint _wheel = 0;    /* Roue de voiture */
 static GLuint _cylinder = 0; /* Pour les suspensions */
 static GLuint _sphere = 0;   /* Pour le casque du pilote */
 static GLuint _disk = 0;     /* Pour les détails circulaires */
 /* _pId est déjà déclaré en externe dans f1_types.h */
 
 /* Initialisation du rendu */
 void initRendering(void) {
     /* Activer la synchronisation verticale */
     SDL_GL_SetSwapInterval(1);
     
     /* Générer les géométries */
     _carBody = gl4dgGenCubef();                /* Corps de la voiture */
     _wheel = gl4dgGenCylinderf(20, 2);         /* Roues comme des cylindres courts */
     _cylinder = gl4dgGenCylinderf(12, 12);     /* Pour les suspensions */
     _sphere = gl4dgGenSpheref(16, 16);         /* Pour le casque du pilote */
     _disk = gl4dgGenDiskf(20);                 /* Pour les détails circulaires */
     
     /* Création d'un circuit simple (un plan pour commencer) */
     _track = gl4dgGenQuadf();
     
     /* Création du programme GLSL */
     _pId = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
     
     /* Configuration OpenGL */
     glEnable(GL_DEPTH_TEST);
     glCullFace(GL_BACK);
     glEnable(GL_CULL_FACE);
     glEnable(GL_LIGHTING);
     glEnable(GL_LIGHT0);
     glEnable(GL_COLOR_MATERIAL);
     glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
     
     /* Configuration de la lumière */
     float lightPos[4] = {-50.0f, 100.0f, 50.0f, 1.0f};  /* Meilleure position pour l'éclairage */
     float lightAmbient[4] = {0.3f, 0.3f, 0.3f, 1.0f};
     float lightDiffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
     float lightSpecular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
     
     glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
     glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
     glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
     glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
     
     glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  /* Couleur bleu ciel */
     glViewport(0, 0, _ww, _wh);
     
     /* Initialisation des matrices GL4D */
     gl4duGenMatrix(GL_FLOAT, "model");
     gl4duGenMatrix(GL_FLOAT, "view");
     gl4duGenMatrix(GL_FLOAT, "proj");
     
     /* Configuration de la matrice de projection */
     gl4duBindMatrix("proj");
     gl4duLoadIdentityf();
     gl4duFrustumf(-0.5f, 0.5f, -0.5f * _wh / _ww, 0.5f * _wh / _ww, 1.0f, 1000.0f);
     
     /* Initialisation de la voiture du joueur */
     _playerCar.x = 0.0f;
     _playerCar.y = 0.06f;  /* Hauteur ajustée */
     _playerCar.z = 0.0f;
     _playerCar.rx = 0.0f;
     _playerCar.ry = 0.0f;
     _playerCar.rz = 0.0f;
     _playerCar.speed = 0.0f;
     _playerCar.acceleration = 0.1f;
     _playerCar.maxSpeed = 2.0f;
     _playerCar.steering = 0.0f;
     _playerCar.r = 0.9f;  /* Rouge vif */
     _playerCar.g = 0.1f;
     _playerCar.b = 0.1f;
     _playerCar.wheelRotation = 0.0f;
 
     printf("F1 Racing Game - Commandes:\n");
     printf("Flèches directionnelles pour conduire\n");
     printf("V pour changer de vue\n");
     printf("Échap pour quitter\n");
 }
 
 /* Déclaration en avant de la fonction qui sera implémentée plus tard */
 void drawWaypoints(void);
 
 /* Fonction de dessin principale */
 void draw(void) {
     /* Effacer les buffers */
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     
     /* Utiliser le programme GLSL */
     glUseProgram(_pId);
     
     /* Configurer la caméra */
     setCamera();
     
     /* Dessiner le circuit */
     drawTrack();
     
     /* Les waypoints sont désactivés pour l'instant car l'implémentation n'est pas encore prête */
     /* drawWaypoints(); */
     
     /* Dessiner la voiture du joueur */
     drawCar(&_playerCar);
     
     /* Dessiner les voitures IA */
     for(int i = 0; i < 3; i++) {  /* Nombre fixe de voitures IA */
         Car* aiCar = getAICar(i);
         if(aiCar) {
             drawCar(aiCar);
         }
     }
 }
 
 /* Configuration de la caméra selon le mode de vue */
 void setCamera(void) {
     gl4duBindMatrix("view");
     gl4duLoadIdentityf();
     
     switch(_viewMode) {
         case 0: /* Vue course (angle) */
             {
                 float camDistance = 3.0f;
                 float camHeight = 0.5f;
                 float camSide = 2.2f;
                 float angleRad = _playerCar.ry * M_PI / 180.0f;
                 float camX = _playerCar.x - camDistance * sinf(angleRad) + camSide * cosf(angleRad);
                 float camZ = _playerCar.z - camDistance * cosf(angleRad) - camSide * sinf(angleRad);
                 
                 gl4duLookAtf(camX, _playerCar.y + camHeight, camZ, 
                           _playerCar.x, _playerCar.y + 0.1f, _playerCar.z,
                           0.0f, 1.0f, 0.0f);
             }
             break;
             
         case 1: /* Vue face */
             {
                 float camHeight = 1.0f;
                 float camDistance = 5.0f;
                 gl4duLookAtf(
                     _playerCar.x, _playerCar.y + camHeight, _playerCar.z + camDistance,
                     _playerCar.x, _playerCar.y, _playerCar.z,
                     0.0f, 1.0f, 0.0f
                 );
             }
             break;
         case 2: /* Vue arrière */
             {
                 float camHeight = 0.5f;
                 float camDistance = 3.0f;
                 gl4duLookAtf(
                     _playerCar.x, _playerCar.y + camHeight, _playerCar.z - camDistance,
                     _playerCar.x, _playerCar.y, _playerCar.z,
                     0.0f, 1.0f, 0.0f
                 );
             }
             break;
         case 3: /* Vue du dessus */
             {
                 float camHeight = 5.0f;
                 gl4duLookAtf(
                     _playerCar.x, _playerCar.y + camHeight, _playerCar.z,
                     _playerCar.x, _playerCar.y, _playerCar.z,
                     0.0f, 0.0f, 1.0f /* Le "haut" est maintenant l'axe Z */
                 );
             }
             break;
     }
 }
 
 /* Dessin du circuit */
 void drawTrack(void) {
     gl4duBindMatrix("model");
     gl4duLoadIdentityf();
     gl4duTranslatef(0.0f, 0.0f, 0.0f);
     gl4duScalef(20.0f, 1.0f, 20.0f);
     gl4duSendMatrices();
     
     /* Circuit gris foncé avec texture d'asphalte */
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.15f, 0.15f, 0.15f, 1.0f);
     gl4dgDraw(_track);
     
     /* Lignes de départ/arrivée */
     for(int i = 0; i < 8; i++) {
         gl4duBindMatrix("model");
         gl4duLoadIdentityf();
         gl4duTranslatef(-1.5f + i * 0.4f, 0.002f, 3.0f);
         gl4duScalef(0.2f, 0.01f, 0.5f);
         gl4duSendMatrices();
         
         /* Effet damier noir et blanc */
         if(i % 2 == 0)
             glUniform4f(glGetUniformLocation(_pId, "couleur"), 1.0f, 1.0f, 1.0f, 1.0f);
         else
             glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 0.0f, 1.0f);
             
         gl4dgDraw(_carBody);
     }
 }
 
 /* Fonction pour dessiner les points de passage (pour l'instant vide, à implémenter plus tard) */
 void drawWaypoints(void) {
     /* Cette fonctionnalité sera implémentée ultérieurement */
     /* Pour l'instant, elle est laissée vide pour éviter les erreurs de linking */
 }
 
 /* Dessin d'une voiture */
 void drawCar(Car *car) {
     gl4duBindMatrix("model");
     gl4duLoadIdentityf();
     gl4duTranslatef(car->x, car->y, car->z);
     gl4duRotatef(car->ry, 0.0f, 1.0f, 0.0f);
 
     /* **** MONOCOQUE PRINCIPALE **** */
     
     /* Châssis principal - plus fin et aérodynamique */
     gl4duPushMatrix();
     gl4duScalef(0.22f, 0.03f, 0.8f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 
                car->r, car->g, car->b, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
 
     /* Cockpit - plus étroit avec une forme plus authentique */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.05f, -0.05f);
     gl4duScalef(0.12f, 0.06f, 0.3f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 0.0f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     /* Halo de protection - modèle plus précis */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.11f, -0.05f);
     gl4duScalef(0.12f, 0.01f, 0.12f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.2f, 0.2f, 0.2f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     /* Support central du halo */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.08f, 0.0f);
     gl4duScalef(0.01f, 0.05f, 0.01f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.2f, 0.2f, 0.2f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     /* Casque du pilote */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.11f, -0.05f);
     gl4duScalef(0.04f, 0.04f, 0.04f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.2f, 0.8f, 1.0f);
     gl4dgDraw(_sphere);
     gl4duPopMatrix();
 
     /* **** AILERONS **** */
     
     /* Aileron avant - plus large et plus fin */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.02f, 0.68f);
     gl4duScalef(0.4f, 0.008f, 0.08f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.1f, 0.1f, 0.1f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     /* Éléments d'aileron avant (ailettes) */
     for(int i = 0; i < 3; i++) {
         gl4duPushMatrix();
         gl4duTranslatef(0.0f - 0.1f + i * 0.1f, 0.02f, 0.72f);
         gl4duRotatef(10.0f, 0.0f, 1.0f, 0.0f);
         gl4duScalef(0.005f, 0.02f, 0.04f);
         gl4duSendMatrices();
         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.9f, 0.9f, 0.9f, 1.0f);
         gl4dgDraw(_carBody);
         gl4duPopMatrix();
     }
 
     /* Aileron arrière - DRS style plus réaliste */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.15f, -0.6f);
     gl4duScalef(0.32f, 0.008f, 0.05f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.1f, 0.1f, 0.1f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
 
     /* Support central aileron arrière */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.09f, -0.6f);
     gl4duScalef(0.03f, 0.12f, 0.02f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.1f, 0.1f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
 
     /* **** PONTONS ET DÉTAILS LATÉRAUX **** */
     
     /* Pontons latéraux avec forme plus organique */
     gl4duPushMatrix();
     gl4duTranslatef(0.16f, 0.04f, -0.15f);
     gl4duScalef(0.07f, 0.035f, 0.5f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), car->r * 0.9f, car->g * 0.9f, car->b * 0.9f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
 
     gl4duPushMatrix();
     gl4duTranslatef(-0.16f, 0.04f, -0.15f);
     gl4duScalef(0.07f, 0.035f, 0.5f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), car->r * 0.9f, car->g * 0.9f, car->b * 0.9f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     /* Entrées d'air (NACA ducts) */
     gl4duPushMatrix();
     gl4duTranslatef(0.16f, 0.06f, 0.1f);
     gl4duScalef(0.04f, 0.007f, 0.08f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 0.0f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     gl4duPushMatrix();
     gl4duTranslatef(-0.16f, 0.06f, 0.1f);
     gl4duScalef(0.04f, 0.007f, 0.08f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 0.0f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
 
     /* Bargeboard (déflecteurs latéraux) */
     gl4duPushMatrix();
     gl4duTranslatef(0.14f, 0.05f, 0.3f);
     gl4duRotatef(15.0f, 0.0f, 1.0f, 0.0f);
     gl4duScalef(0.008f, 0.05f, 0.12f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.9f, 0.9f, 0.9f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     gl4duPushMatrix();
     gl4duTranslatef(-0.14f, 0.05f, 0.3f);
     gl4duRotatef(-15.0f, 0.0f, 1.0f, 0.0f);
     gl4duScalef(0.008f, 0.05f, 0.12f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.9f, 0.9f, 0.9f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
 
     /* **** GARDE-BOUE POUR CACHER PARTIELLEMENT LES ROUES **** */
     
     /* Garde-boue avant gauche - plus petit pour montrer les roues */
     gl4duPushMatrix();
     gl4duTranslatef(0.13f, 0.025f, 0.43f);
     gl4duScalef(0.04f, 0.04f, 0.12f); /* Réduit pour moins couvrir */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), car->r, car->g, car->b, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     /* Garde-boue avant droit */
     gl4duPushMatrix();
     gl4duTranslatef(-0.13f, 0.025f, 0.43f);
     gl4duScalef(0.04f, 0.04f, 0.12f); /* Réduit pour moins couvrir */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), car->r, car->g, car->b, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     /* Garde-boue arrière gauche */
     gl4duPushMatrix();
     gl4duTranslatef(0.13f, 0.025f, -0.43f);
     gl4duScalef(0.04f, 0.04f, 0.12f); /* Réduit pour moins couvrir */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), car->r, car->g, car->b, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     /* Garde-boue arrière droit */
     gl4duPushMatrix();
     gl4duTranslatef(-0.13f, 0.025f, -0.43f);
     gl4duScalef(0.04f, 0.04f, 0.12f); /* Réduit pour moins couvrir */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), car->r, car->g, car->b, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
 
     /* **** ROUES AMÉLIORÉES **** */
     
     /* Roue avant gauche */
    gl4duPushMatrix();
    gl4duTranslatef(0.25f, 0.0f, 0.43f);  /* Plus éloignée (0.25), au niveau du sol (0.0) */
    gl4duRotatef(90.0f, 1.0f, 0.0f, 0.0f); /* Changé: rotation sur l'axe X au lieu de Z */
    gl4duRotatef(car->wheelRotation, 0.0f, 1.0f, 0.0f); /* Changé: rotation sur l'axe Y au lieu de Z */
    gl4duScalef(0.08f, 0.03f, 0.08f);  /* Beaucoup plus grande (0.08) */
    gl4duSendMatrices();
    glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 0.0f, 1.0f);
    gl4dgDraw(_wheel);
    gl4duPopMatrix();

     /* Jante avant gauche - face 1 */
     gl4duPushMatrix();
     gl4duTranslatef(0.19f, 0.06f, 0.465f); /* Alignée avec la roue */
     gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
     gl4duRotatef(car->wheelRotation, 1.0f, 0.0f, 0.0f);
     gl4duScalef(0.035f, 0.035f, 0.035f); /* Plus grande */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.8f, 0.8f, 1.0f); /* Gris plus clair */
     gl4dgDraw(_disk);
     gl4duPopMatrix();
 
     /* Jante avant gauche - face 2 */
     gl4duPushMatrix();
     gl4duTranslatef(0.19f, 0.06f, 0.395f); /* Alignée avec la roue */
     gl4duRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
     gl4duRotatef(car->wheelRotation, -1.0f, 0.0f, 0.0f);
     gl4duScalef(0.035f, 0.035f, 0.035f); /* Plus grande */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.8f, 0.8f, 1.0f); /* Gris plus clair */
     gl4dgDraw(_disk);
     gl4duPopMatrix();
 
     /* Roue avant droite */
    gl4duPushMatrix();
    gl4duTranslatef(-0.25f, 0.0f, 0.43f);  /* Plus éloignée (-0.25), au niveau du sol (0.0) */
    gl4duRotatef(90.0f, 1.0f, 0.0f, 0.0f); /* Changé: rotation sur l'axe X au lieu de Z */
    gl4duRotatef(car->wheelRotation, 0.0f, 1.0f, 0.0f); /* Changé: rotation sur l'axe Y au lieu de Z */
    gl4duScalef(0.08f, 0.03f, 0.08f);  /* Beaucoup plus grande (0.08) */
    gl4duSendMatrices();
    glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 0.0f, 1.0f);
    gl4dgDraw(_wheel);
    gl4duPopMatrix();
    
     /* Jante avant droite - face 1 */
     gl4duPushMatrix();
     gl4duTranslatef(-0.19f, 0.06f, 0.465f); /* Alignée avec la roue */
     gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
     gl4duRotatef(car->wheelRotation, 1.0f, 0.0f, 0.0f);
     gl4duScalef(0.035f, 0.035f, 0.035f); /* Plus grande */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.8f, 0.8f, 1.0f); /* Gris plus clair */
     gl4dgDraw(_disk);
     gl4duPopMatrix();
 
     /* Jante avant droite - face 2 */
     gl4duPushMatrix();
     gl4duTranslatef(-0.19f, 0.06f, 0.395f); /* Alignée avec la roue */
     gl4duRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
     gl4duRotatef(car->wheelRotation, -1.0f, 0.0f, 0.0f);
     gl4duScalef(0.035f, 0.035f, 0.035f); /* Plus grande */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.8f, 0.8f, 1.0f); /* Gris plus clair */
     gl4dgDraw(_disk);
     gl4duPopMatrix();
 
     /* Roue arrière gauche - légèrement plus grande */
    gl4duPushMatrix();
    gl4duTranslatef(0.25f, 0.0f, -0.43f);  /* Plus éloignée (0.25), au niveau du sol (0.0) */
    gl4duRotatef(90.0f, 1.0f, 0.0f, 0.0f); /* Changé: rotation sur l'axe X au lieu de Z */
    gl4duRotatef(car->wheelRotation, 0.0f, 1.0f, 0.0f); /* Changé: rotation sur l'axe Y au lieu de Z */
    gl4duScalef(0.09f, 0.04f, 0.09f);  /* Encore plus grande (0.09) */
    gl4duSendMatrices();
    glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 0.0f, 1.0f);
    gl4dgDraw(_wheel);
    gl4duPopMatrix();
 
     /* Jante arrière gauche - face 1 */
     gl4duPushMatrix();
     gl4duTranslatef(0.19f, 0.06f, -0.39f); /* Alignée avec la roue */
     gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
     gl4duRotatef(car->wheelRotation, 1.0f, 0.0f, 0.0f);
     gl4duScalef(0.04f, 0.04f, 0.04f); /* Plus grande */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.8f, 0.8f, 1.0f); /* Gris plus clair */
     gl4dgDraw(_disk);
     gl4duPopMatrix();
 
     /* Jante arrière gauche - face 2 */
     gl4duPushMatrix();
     gl4duTranslatef(0.19f, 0.06f, -0.47f); /* Alignée avec la roue */
     gl4duRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
     gl4duRotatef(car->wheelRotation, -1.0f, 0.0f, 0.0f);
     gl4duScalef(0.04f, 0.04f, 0.04f); /* Plus grande */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.8f, 0.8f, 1.0f); /* Gris plus clair */
     gl4dgDraw(_disk);
     gl4duPopMatrix();
 
     /* Roue arrière droite */
    gl4duPushMatrix();
    gl4duTranslatef(-0.25f, 0.0f, -0.43f);  /* Plus éloignée (-0.25), au niveau du sol (0.0) */
    gl4duRotatef(90.0f, 1.0f, 0.0f, 0.0f); /* Changé: rotation sur l'axe X au lieu de Z */
    gl4duRotatef(car->wheelRotation, 0.0f, 1.0f, 0.0f); /* Changé: rotation sur l'axe Y au lieu de Z */
    gl4duScalef(0.09f, 0.04f, 0.09f);  /* Encore plus grande (0.09) */
    gl4duSendMatrices();
    glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 0.0f, 1.0f);
    gl4dgDraw(_wheel);
    gl4duPopMatrix();
    
     /* Jante arrière droite - face 1 */
     gl4duPushMatrix();
     gl4duTranslatef(-0.19f, 0.06f, -0.39f); /* Alignée avec la roue */
     gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
     gl4duRotatef(car->wheelRotation, 1.0f, 0.0f, 0.0f);
     gl4duScalef(0.04f, 0.04f, 0.04f); /* Plus grande */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.8f, 0.8f, 1.0f); /* Gris plus clair */
     gl4dgDraw(_disk);
     gl4duPopMatrix();
 
     /* Jante arrière droite - face 2 */
     gl4duPushMatrix();
     gl4duTranslatef(-0.19f, 0.06f, -0.47f); /* Alignée avec la roue */
     gl4duRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
     gl4duRotatef(car->wheelRotation, -1.0f, 0.0f, 0.0f);
     gl4duScalef(0.04f, 0.04f, 0.04f); /* Plus grande */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.8f, 0.8f, 1.0f); /* Gris plus clair */
     gl4dgDraw(_disk);
     gl4duPopMatrix();
 
     /* Suspensions visibles */
     /* Suspension avant gauche */
     gl4duPushMatrix();
     gl4duTranslatef(0.1f, 0.04f, 0.43f); /* Centré entre le châssis et la roue */
     gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
     gl4duScalef(0.005f, 0.005f, 0.09f); /* Plus longue pour atteindre la roue */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.6f, 0.6f, 0.6f, 1.0f);
     gl4dgDraw(_cylinder);
     gl4duPopMatrix();
 
     /* Suspension avant droite */
     gl4duPushMatrix();
     gl4duTranslatef(-0.1f, 0.04f, 0.43f); /* Centré entre le châssis et la roue */
     gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
     gl4duScalef(0.005f, 0.005f, 0.09f); /* Plus longue pour atteindre la roue */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.6f, 0.6f, 0.6f, 1.0f);
     gl4dgDraw(_cylinder);
     gl4duPopMatrix();
 
     /* Suspension arrière gauche */
     gl4duPushMatrix();
     gl4duTranslatef(0.1f, 0.04f, -0.43f); /* Centré entre le châssis et la roue */
     gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
     gl4duScalef(0.005f, 0.005f, 0.09f); /* Plus longue pour atteindre la roue */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.6f, 0.6f, 0.6f, 1.0f);
     gl4dgDraw(_cylinder);
     gl4duPopMatrix();
 
     /* Suspension arrière droite */
     gl4duPushMatrix();
     gl4duTranslatef(-0.1f, 0.04f, -0.43f); /* Centré entre le châssis et la roue */
     gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
     gl4duScalef(0.005f, 0.005f, 0.09f); /* Plus longue pour atteindre la roue */
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.6f, 0.6f, 0.6f, 1.0f);
     gl4dgDraw(_cylinder);
     gl4duPopMatrix();
      
     /* Logo de l'écurie (cercle blanc sur le nez) */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.07f, 0.4f);
     gl4duRotatef(90.0f, 1.0f, 0.0f, 0.0f);
     gl4duScalef(0.02f, 0.02f, 0.005f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 1.0f, 1.0f, 1.0f, 1.0f);
     gl4dgDraw(_disk);
     gl4duPopMatrix();
      
     /* Numéro du pilote */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.05f, -0.35f);
     gl4duScalef(0.05f, 0.02f, 0.05f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 1.0f, 1.0f, 1.0f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
      
     /* Diffuseur arrière avec lames multiples */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.015f, -0.65f);
     gl4duScalef(0.2f, 0.02f, 0.08f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.05f, 0.05f, 0.05f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
      
     /* Lames du diffuseur */
     for(int i = 0; i < 3; i++) {
         gl4duPushMatrix();
         gl4duTranslatef(-0.1f + i * 0.1f, 0.015f, -0.65f);
         gl4duScalef(0.01f, 0.02f, 0.08f);
         gl4duSendMatrices();
         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.2f, 0.2f, 0.2f, 1.0f);
         gl4dgDraw(_carBody);
         gl4duPopMatrix();
     }
      
     /* T-Wing (élément aérodynamique) */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.12f, -0.5f);
     gl4duScalef(0.12f, 0.004f, 0.02f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.9f, 0.9f, 0.9f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
      
     /* Caméra obligatoire sur le dessus */
     gl4duPushMatrix();
     gl4duTranslatef(0.0f, 0.12f, 0.1f);
     gl4duScalef(0.01f, 0.01f, 0.01f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 0.0f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
      
     /* Ailettes latérales (mini-ailerons sur les côtés) */
     gl4duPushMatrix();
     gl4duTranslatef(0.22f, 0.05f, 0.0f);
     gl4duScalef(0.01f, 0.03f, 0.2f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.1f, 0.1f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
     
     gl4duPushMatrix();
     gl4duTranslatef(-0.22f, 0.05f, 0.0f);
     gl4duScalef(0.01f, 0.03f, 0.2f);
     gl4duSendMatrices();
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.8f, 0.1f, 0.1f, 1.0f);
     gl4dgDraw(_carBody);
     gl4duPopMatrix();
 }