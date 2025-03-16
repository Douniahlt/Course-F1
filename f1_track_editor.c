/*!\file f1_track_editor.c
 * \brief Implémentation de l'éditeur de circuits pour le jeu F1
 */

 #include <GL4D/gl4duw_SDL2.h>
 #include <GL4D/gl4dg.h>
 #include <GL4D/gl4du.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 
 #include "f1_types.h"
 #include "f1_track.h"
 #include "f1_render.h"
 
 /* Constantes pour l'interface utilisateur */
 #define BUTTON_WIDTH 120
 #define BUTTON_HEIGHT 30
 #define PANEL_WIDTH 250
 #define SLIDER_HEIGHT 20
 #define SLIDER_WIDTH 150
 #define LABEL_WIDTH 80
 #define SPACING 10
 
 /* Variables locales */
 static int _editorActive = 0;              /* Indique si l'éditeur est actif */
 static GLuint _editorProgram = 0;          /* Programme GLSL pour l'éditeur */
 static GLuint _gridTexture = 0;            /* Texture pour la grille */
 static GLuint _gridMesh = 0;               /* Mesh pour la grille */
 static int _dragMode = 0;                  /* Mode de glisser-déposer */
 static int _dragStartX = 0, _dragStartY = 0; /* Position initiale du drag */
 static float _dragStartWorldX = 0.0f, _dragStartWorldZ = 0.0f; /* Position initiale du drag en 3D */
 static Track* _clipboardSegment = NULL;    /* Segment copié/coupé */
 
 /* Prototypes de fonctions locales */
 static void renderEditorUI(void);
 static void renderEditorGrid(void);
 static void renderEditorTrack(Track* track);
 static void renderEditorSegmentsList(Track* track);
 static void renderEditorPropertiesPanel(Track* track);
 static int handleEditorButton(int x, int y, int w, int h, const char* label, int active);
 static float handleEditorSlider(int x, int y, int w, int h, const char* label, float value, float min, float max);
 static void handleEditorSegmentClick(Track* track, int index);
 static void handleEditorWaypointClick(Track* track, int index);
 static void addNewSegment(Track* track, TrackSegmentType type);
 static void duplicateSegment(Track* track, int index);
 static void copySegment(Track* track, int index);
 static void pasteSegment(Track* track, int index);
 static void cutSegment(Track* track, int index);
 
 /* Initialisation de l'éditeur de circuit */
 void initTrackEditor(Track* track) {
     /* Initialisation des variables de l'éditeur */
     _editor.currentTrack = track;
     _editor.editMode = 0;  /* Mode édition de segments */
     _editor.selectedSegment = -1;
     _editor.selectedWaypoint = -1;
     _editor.isGridVisible = 1;
     _editor.gridSize = 2.0f;
     _editor.cameraZoom = 20.0f;
     _editor.cameraX = 0.0f;
     _editor.cameraZ = 0.0f;
     
     /* Création du programme GLSL pour l'éditeur si nécessaire */
     if (_editorProgram == 0) {
         _editorProgram = gl4duCreateProgram("<vs>shaders/editor.vs", "<fs>shaders/editor.fs", NULL);
     }
     
     /* Création de la texture et du mesh pour la grille */
     if (_gridMesh == 0) {
         _gridMesh = gl4dgGenQuadf();
     }
     
     if (_gridTexture == 0) {
         /* Création d'une texture simple pour la grille */
         GLuint tex;
         glGenTextures(1, &tex);
         glBindTexture(GL_TEXTURE_2D, tex);
         
         /* Texture de grille 64x64 */
         unsigned char gridData[64 * 64 * 4];
         for (int y = 0; y < 64; y++) {
             for (int x = 0; x < 64; x++) {
                 int index = (y * 64 + x) * 4;
                 if (x == 0 || y == 0 || x == 63 || y == 63) {
                     /* Bords de la cellule */
                     gridData[index + 0] = 200; /* R */
                     gridData[index + 1] = 200; /* G */
                     gridData[index + 2] = 200; /* B */
                     gridData[index + 3] = 255; /* A */
                 } else {
                     /* Intérieur */
                     gridData[index + 0] = 50; /* R */
                     gridData[index + 1] = 50; /* G */
                     gridData[index + 2] = 50; /* B */
                     gridData[index + 3] = 100; /* A */
                 }
             }
         }
         
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, gridData);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
         
         _gridTexture = tex;
     }
     
     /* Initialisation du mode de drag */
     _dragMode = 0;
     
     /* Activation de l'éditeur */
     _editorActive = 1;
 }
 
 /* Rendu de l'éditeur de circuit */
 void renderTrackEditor(void) {
     if (!_editorActive || !_editor.currentTrack) return;
     
     /* Effacement du buffer */
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     
     /* Configuration OpenGL pour l'éditeur */
     glEnable(GL_DEPTH_TEST);
     glEnable(GL_BLEND);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     
     /* Configuration de la caméra de l'éditeur (vue du dessus) */
     gl4duBindMatrix("view");
     gl4duLoadIdentityf();
     gl4duLookAtf(_editor.cameraX, _editor.cameraZoom, _editor.cameraZ,
                _editor.cameraX, 0.0f, _editor.cameraZ,
                0.0f, 0.0f, -1.0f);
     
     /* Configuration de la projection */
     gl4duBindMatrix("proj");
     gl4duLoadIdentityf();
     float aspect = (float)_ww / _wh;
     gl4duFrustumf(-0.5f * aspect, 0.5f * aspect, -0.5f, 0.5f, 1.0f, 1000.0f);
     
     /* Rendu de la grille si activée */
     if (_editor.isGridVisible) {
         renderEditorGrid();
     }
     
     /* Rendu du circuit */
     renderEditorTrack(_editor.currentTrack);
     
     /* Rendu de l'interface utilisateur */
     renderEditorUI();
 }
 
 /* Rendu de la grille de l'éditeur */
 static void renderEditorGrid(void) {
     gl4duBindMatrix("model");
     gl4duLoadIdentityf();
     gl4duTranslatef(_editor.cameraX, -0.1f, _editor.cameraZ);
     gl4duScalef(100.0f, 1.0f, 100.0f);
     gl4duSendMatrices();
     
     /* Activation de la texture de grille */
     glUseProgram(_editorProgram);
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_2D, _gridTexture);
     glUniform1i(glGetUniformLocation(_editorProgram, "textureSampler"), 0);
     glUniform1f(glGetUniformLocation(_editorProgram, "gridSize"), _editor.gridSize);
     
     /* Rendu de la grille */
     gl4dgDraw(_gridMesh);
 }
 
 /* Rendu du circuit dans l'éditeur */
 static void renderEditorTrack(Track* track) {
     if (!track || track->waypointCount < 2) return;
     
     /* Utilisation du programme par défaut */
     glUseProgram(_pId);
     
     /* Rendu du mesh principal du circuit */
     if (track->trackMesh) {
         gl4duBindMatrix("model");
         gl4duLoadIdentityf();
         gl4duSendMatrices();
         
         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.3f, 0.3f, 0.3f, 1.0f);
         gl4dgBindMesh(track->trackMesh);
         gl4dgDrawMesh();
     }
     
     /* Rendu des waypoints */
     for (int i = 0; i < track->waypointCount; i++) {
         gl4duBindMatrix("model");
         gl4duLoadIdentityf();
         gl4duTranslatef(track->waypoints[i].x, 0.2f, track->waypoints[i].z);
         gl4duScalef(0.3f, 0.3f, 0.3f);
         gl4duSendMatrices();
         
         /* Couleur selon sélection */
         if (i == _editor.selectedWaypoint) {
             glUniform4f(glGetUniformLocation(_pId, "couleur"), 1.0f, 1.0f, 0.0f, 1.0f);
         } else {
             glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.7f, 1.0f, 1.0f);
         }
         
         gl4dgDraw(_carBody); /* Utilisation du cube de voiture comme marqueur */
     }
     
     /* Rendu d'indicateurs spéciaux pour les segments */
     if (_editor.editMode == 0) {
         for (int i = 0; i < track->segmentCount; i++) {
             /* Trouver le waypoint correspondant à ce segment */
             int wpIndex = 0;
             float segmentLength = 0.0f;
             for (int j = 0; j < i; j++) {
                 segmentLength += track->segments[j].length;
             }
             
             /* Trouver le waypoint le plus proche du début du segment */
             float bestDist = 1000000.0f;
             for (int j = 0; j < track->waypointCount; j++) {
                 float dist = fabs(j * WAYPOINT_SPACING - segmentLength);
                 if (dist < bestDist) {
                     bestDist = dist;
                     wpIndex = j;
                 }
             }
             
             /* Marqueur de segment */
             gl4duBindMatrix("model");
             gl4duLoadIdentityf();
             gl4duTranslatef(track->waypoints[wpIndex].x, 0.5f, track->waypoints[wpIndex].z);
             gl4duScalef(0.5f, 0.5f, 0.5f);
             gl4duSendMatrices();
             
             /* Couleur selon sélection */
             if (i == _editor.selectedSegment) {
                 glUniform4f(glGetUniformLocation(_pId, "couleur"), 1.0f, 0.0f, 0.0f, 1.0f);
             } else {
                 /* Couleur selon type de segment */
                 switch (track->segments[i].type) {
                     case TRACK_STRAIGHT:
                         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 1.0f, 0.0f, 1.0f);
                         break;
                     case TRACK_CURVE_LEFT:
                     case TRACK_CURVE_RIGHT:
                         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.0f, 0.0f, 1.0f, 1.0f);
                         break;
                     case TRACK_HAIRPIN_LEFT:
                     case TRACK_HAIRPIN_RIGHT:
                         glUniform4f(glGetUniformLocation(_pId, "couleur"), 1.0f, 0.5f, 0.0f, 1.0f);
                         break;
                 }
             }
             
             gl4dgDraw(_sphere); /* Utilisation d'une sphère comme marqueur */
         }
     }
 }
 
 /* Rendu de l'interface utilisateur de l'éditeur */
 static void renderEditorUI(void) {
     /* Désactivation de la 3D pour l'interface 2D */
     glDisable(GL_DEPTH_TEST);
     
     /* Rendu du panneau latéral */
     /* TODO: Implémenter le rendu du panneau UI avec SDL_ttf ou une bibliothèque d'interface */
     
     /* Pour l'instant, nous allons simplement dessiner des zones colorées */
     glUseProgram(_pId);
     
     /* Panneau principal */
     gl4duBindMatrix("model");
     gl4duLoadIdentityf();
     gl4duTranslatef(_ww - PANEL_WIDTH / 2, _wh / 2, 0.0f);
     gl4duScalef(PANEL_WIDTH, _wh, 1.0f);
     gl4duSendMatrices();
     
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.2f, 0.2f, 0.2f, 0.8f);
     gl4dgDraw(_carBody);
     
     /* Rendu des onglets */
     int tabWidth = PANEL_WIDTH / 3;
     
     for (int i = 0; i < 3; i++) {
         gl4duBindMatrix("model");
         gl4duLoadIdentityf();
         gl4duTranslatef(_ww - PANEL_WIDTH + tabWidth * i + tabWidth / 2, _wh - 20, 0.0f);
         gl4duScalef(tabWidth - 4, 30, 1.0f);
         gl4duSendMatrices();
         
         if (i == _editor.editMode) {
             glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.5f, 0.5f, 0.8f, 1.0f);
         } else {
             glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.3f, 0.3f, 0.5f, 1.0f);
         }
         
         gl4dgDraw(_carBody);
     }
     
     /* Rendu des boutons et contrôles selon le mode */
     switch (_editor.editMode) {
         case 0: /* Mode segments */
             renderEditorSegmentsList(_editor.currentTrack);
             break;
         case 1: /* Mode waypoints */
             /* Non implémenté pour l'instant */
             break;
         case 2: /* Mode propriétés */
             renderEditorPropertiesPanel(_editor.currentTrack);
             break;
     }
     
     /* Réactivation de la 3D pour le prochain frame */
     glEnable(GL_DEPTH_TEST);
 }
 
 /* Rendu de la liste des segments */
 static void renderEditorSegmentsList(Track* track) {
     /* Titre */
     /* TODO: Afficher le texte "Segments" */
     
     /* Boutons d'actions */
     int buttonY = _wh - 60;
     int selectedChange = 0;
     
     /* Bouton Ajouter droit */
     if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH / 2 - 5, BUTTON_HEIGHT, "Add Straight", 0)) {
         addNewSegment(track, TRACK_STRAIGHT);
     }
     
     /* Bouton Ajouter virage gauche */
     if (handleEditorButton(_ww - PANEL_WIDTH + BUTTON_WIDTH / 2 + 10, buttonY, BUTTON_WIDTH / 2 - 5, BUTTON_HEIGHT, "Add Left", 0)) {
         addNewSegment(track, TRACK_CURVE_LEFT);
     }
     
     buttonY -= BUTTON_HEIGHT + SPACING;
     
     /* Bouton Ajouter virage droit */
     if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH / 2 - 5, BUTTON_HEIGHT, "Add Right", 0)) {
         addNewSegment(track, TRACK_CURVE_RIGHT);
     }
     
     /* Bouton Ajouter épingle */
     if (handleEditorButton(_ww - PANEL_WIDTH + BUTTON_WIDTH / 2 + 10, buttonY, BUTTON_WIDTH / 2 - 5, BUTTON_HEIGHT, "Add Hairpin", 0)) {
         addNewSegment(track, TRACK_HAIRPIN_LEFT);
     }
     
     buttonY -= BUTTON_HEIGHT + SPACING;
     
     /* Bouton Supprimer */
     if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH / 2 - 5, BUTTON_HEIGHT, "Delete", _editor.selectedSegment >= 0)) {
         if (_editor.selectedSegment >= 0 && _editor.selectedSegment < track->segmentCount) {
             removeTrackSegment(track, _editor.selectedSegment);
             if (_editor.selectedSegment >= track->segmentCount) {
                 _editor.selectedSegment = track->segmentCount - 1;
             }
             selectedChange = 1;
         }
     }
     
     /* Bouton Dupliquer */
     if (handleEditorButton(_ww - PANEL_WIDTH + BUTTON_WIDTH / 2 + 10, buttonY, BUTTON_WIDTH / 2 - 5, BUTTON_HEIGHT, "Duplicate", _editor.selectedSegment >= 0)) {
         if (_editor.selectedSegment >= 0 && _editor.selectedSegment < track->segmentCount) {
             duplicateSegment(track, _editor.selectedSegment);
             selectedChange = 1;
         }
     }
     
     buttonY -= BUTTON_HEIGHT + SPACING;
     
     /* Bouton Copier */
     if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH / 2 - 5, BUTTON_HEIGHT, "Copy", _editor.selectedSegment >= 0)) {
         if (_editor.selectedSegment >= 0 && _editor.selectedSegment < track->segmentCount) {
             copySegment(track, _editor.selectedSegment);
         }
     }
     
     /* Bouton Coller */
     if (handleEditorButton(_ww - PANEL_WIDTH + BUTTON_WIDTH / 2 + 10, buttonY, BUTTON_WIDTH / 2 - 5, BUTTON_HEIGHT, "Paste", _clipboardSegment != NULL)) {
         if (_editor.selectedSegment >= 0 && _editor.selectedSegment < track->segmentCount) {
             pasteSegment(track, _editor.selectedSegment);
             selectedChange = 1;
         }
     }
     
     buttonY -= BUTTON_HEIGHT + SPACING;
     
     /* Bouton Couper */
     if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH / 2 - 5, BUTTON_HEIGHT, "Cut", _editor.selectedSegment >= 0)) {
         if (_editor.selectedSegment >= 0 && _editor.selectedSegment < track->segmentCount) {
             cutSegment(track, _editor.selectedSegment);
             selectedChange = 1;
         }
     }
     
     buttonY -= BUTTON_HEIGHT + SPACING * 2;
     
     /* Propriétés du segment sélectionné */
     if (_editor.selectedSegment >= 0 && _editor.selectedSegment < track->segmentCount) {
         TrackSegment* segment = &track->segments[_editor.selectedSegment];
         
         /* Type de segment */
         /* TODO: Bouton radio pour changer le type */
         
         /* Longueur */
         float newLength = handleEditorSlider(_ww - PANEL_WIDTH + 10, buttonY, SLIDER_WIDTH, SLIDER_HEIGHT, "Length", segment->length, MIN_SEGMENT_LENGTH, MAX_SEGMENT_LENGTH);
         if (newLength != segment->length) {
             segment->length = newLength;
             track->isModified = 1;
             selectedChange = 1;
         }
         
         buttonY -= SLIDER_HEIGHT + SPACING;
         
         /* Courbure (seulement pour les virages) */
         if (segment->type != TRACK_STRAIGHT) {
             float newCurvature = handleEditorSlider(_ww - PANEL_WIDTH + 10, buttonY, SLIDER_WIDTH, SLIDER_HEIGHT, "Curve", segment->curvature, MIN_CURVATURE, MAX_CURVATURE);
             if (newCurvature != segment->curvature) {
                 segment->curvature = newCurvature;
                 track->isModified = 1;
                 selectedChange = 1;
             }
             
             buttonY -= SLIDER_HEIGHT + SPACING;
         }
         
         /* Largeur */
         float newWidth = handleEditorSlider(_ww - PANEL_WIDTH + 10, buttonY, SLIDER_WIDTH, SLIDER_HEIGHT, "Width", segment->width, MIN_TRACK_WIDTH, MAX_TRACK_WIDTH);
         if (newWidth != segment->width) {
             segment->width = newWidth;
             track->isModified = 1;
             selectedChange = 1;
         }
     }
     
     /* Si le circuit a été modifié, régénérer les waypoints et le mesh */
     if (selectedChange && track->isModified) {
         generateWaypoints(track);
         generateTrackMesh(track);
     }
 }
 
 /* Rendu du panneau de propriétés */
 static void renderEditorPropertiesPanel(Track* track) {
     /* Titre */
     /* TODO: Afficher le texte "Propriétés" */
     
     int buttonY = _wh - 60;
     
     /* Bouton Nouveau circuit */
     if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "New Track", 1)) {
         /* TODO: Boîte de dialogue de nom */
         Track* newTrack = createEmptyTrack("Nouveau circuit");
         if (newTrack) {
             if (_editor.currentTrack) {
                 destroyTrack(_editor.currentTrack);
             }
             _editor.currentTrack = newTrack;
             _editor.selectedSegment = -1;
             _editor.selectedWaypoint = -1;
         }
     }
     
     buttonY -= BUTTON_HEIGHT + SPACING;
     
     /* Bouton Charger circuit */
     if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "Load Track", 1)) {
         /* TODO: Boîte de dialogue de fichier */
         Track* loadedTrack = loadTrackFromFile("circuit.track");
         if (loadedTrack) {
             if (_editor.currentTrack) {
                 destroyTrack(_editor.currentTrack);
             }
             _editor.currentTrack = loadedTrack;
             _editor.selectedSegment = -1;
             _editor.selectedWaypoint = -1;
         }
     }
     
     buttonY -= BUTTON_HEIGHT + SPACING;
     
     /* Bouton Sauvegarder circuit */
     if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "Save Track", track != NULL)) {
         /* TODO: Boîte de dialogue de fichier */
         if (track) {
             saveTrackToFile(track, "circuit.track");
         }
     }
     
     buttonY -= BUTTON_HEIGHT + SPACING;
     
     /* Bouton Générer circuit aléatoire */
     if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "Random Track", 1)) {
         /* TODO: Boîte de dialogue de paramètres */
         Track* randomTrack = generateRandomTrack("Circuit aléatoire", 8 + rand() % 5);
         if (randomTrack) {
             if (_editor.currentTrack) {
                 destroyTrack(_editor.currentTrack);
             }
             _editor.currentTrack = randomTrack;
             _editor.selectedSegment = -1;
             _editor.selectedWaypoint = -1;
         }
     }
     
     buttonY -= BUTTON_HEIGHT + SPACING * 2;
     
     /* Propriétés globales du circuit */
     if (track) {
         /* Nom du circuit */
         /* TODO: Champ de texte */
         
         /* Affichage grille */
         if (handleEditorButton(_ww - PANEL_WIDTH + 10, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "Toggle Grid", 1)) {
             _editor.isGridVisible = !_editor.isGridVisible;
         }
         
         buttonY -= BUTTON_HEIGHT + SPACING;
         
         /* Taille de la grille */
         float newGridSize = handleEditorSlider(_ww - PANEL_WIDTH + 10, buttonY, SLIDER_WIDTH, SLIDER_HEIGHT, "Grid Size", _editor.gridSize, 1.0f, 10.0f);
         if (newGridSize != _editor.gridSize) {
             _editor.gridSize = newGridSize;
         }
     }
 }
 
 /* Traitement des entrées pour l'éditeur */
 void handleTrackEditorInput(SDL_Event* event) {
     if (!_editorActive) return;
     
     Track* track = _editor.currentTrack;
     if (!track) return;
     
     switch (event->type) {
         case SDL_MOUSEBUTTONDOWN:
             if (event->button.button == SDL_BUTTON_LEFT) {
                 int x = event->button.x;
                 int y = event->button.y;
                 
                 /* Vérifier si le clic est dans le panneau latéral */
                 if (x > _ww - PANEL_WIDTH) {
                     /* Gestion des onglets */
                     if (y > _wh - 40) {
                         int tabWidth = PANEL_WIDTH / 3;
                         int tabIndex = (x - (_ww - PANEL_WIDTH)) / tabWidth;
                         if (tabIndex >= 0 && tabIndex <= 2) {
                             _editor.editMode = tabIndex;
                         }
                     }
                     
                     /* Les clics dans l'interface sont gérés par les fonctions d'interface */
                     return;
                 }
                 
                 /* Clic dans la vue 3D */
                 _dragMode = 1;
                 _dragStartX = x;
                 _dragStartY = y;
                 
                 /* Convertir les coordonnées écran en coordonnées monde */
                 float worldX, worldY, worldZ;
                 screenToWorld(x, y, &worldX, &worldY, &worldZ);
                 _dragStartWorldX = worldX;
                 _dragStartWorldZ = worldZ;
                 
                 /* Vérifier si on clique sur un waypoint */
                 if (_editor.editMode == 1) {
                     float minDist = 1.0f;
                     int closestWP = -1;
                     
                     for (int i = 0; i < track->waypointCount; i++) {
                         float dx = track->waypoints[i].x - worldX;
                         float dz = track->waypoints[i].z - worldZ;
                         float dist = sqrtf(dx * dx + dz * dz);
                         
                         if (dist < minDist) {
                             minDist = dist;
                             closestWP = i;
                         }
                     }
                     
                     if (closestWP >= 0) {
                         handleEditorWaypointClick(track, closestWP);
                     } else {
                         _editor.selectedWaypoint = -1;
                     }
                 } 
                 /* Vérifier si on clique sur un segment */
                 else if (_editor.editMode == 0) {
                     float minDist = 2.0f;
                     int closestSeg = -1;
                     
                     /* Pour simplifier, on utilise les waypoints comme approximation des segments */
                     for (int i = 0; i < track->segmentCount; i++) {
                         /* Trouver le waypoint correspondant à ce segment */
                         int wpIndex = 0;
                         float segmentLength = 0.0f;
                         for (int j = 0; j < i; j++) {
                             segmentLength += track->segments[j].length;
                         }
                         
                         /* Trouver le waypoint le plus proche du début du segment */
                         float bestDist = 1000000.0f;
                         for (int j = 0; j < track->waypointCount; j++) {
                             float dist = fabs(j * WAYPOINT_SPACING - segmentLength);
                             if (dist < bestDist) {
                                 bestDist = dist;
                                 wpIndex = j;
                             }
                         }
                         
                         float dx = track->waypoints[wpIndex].x - worldX;
                         float dz = track->waypoints[wpIndex].z - worldZ;
                         float dist = sqrtf(dx * dx + dz * dz);
                         
                         if (dist < minDist) {
                             minDist = dist;
                             closestSeg = i;
                         }
                     }
                     
                     if (closestSeg >= 0) {
                         handleEditorSegmentClick(track, closestSeg);
                     } else {
                         _editor.selectedSegment = -1;
                     }
                 }
             } else if (event->button.button == SDL_BUTTON_RIGHT) {
                 /* Clic droit pour le déplacement de la caméra */
                 _dragMode = 2;
                 _dragStartX = event->button.x;
                 _dragStartY = event->button.y;
                 _dragStartWorldX = _editor.cameraX;
                 _dragStartWorldZ = _editor.cameraZ;
             } else if (event->button.button == SDL_BUTTON_MIDDLE) {
                 /* Clic molette pour le zoom */
                 _dragMode = 3;
                 _dragStartY = event->button.y;
             }
             break;
             
         case SDL_MOUSEBUTTONUP:
             _dragMode = 0;
             break;
             
         case SDL_MOUSEMOTION:
             /* Gestion du drag & drop */
             if (_dragMode == 1) {
                 /* Déplacement d'un waypoint sélectionné */
                 if (_editor.editMode == 1 && _editor.selectedWaypoint >= 0) {
                     float worldX, worldY, worldZ;
                     screenToWorld(event->motion.x, event->motion.y, &worldX, &worldY, &worldZ);
                     
                     /* Mise à jour de la position */
                     track->waypoints[_editor.selectedWaypoint].x = worldX;
                     track->waypoints[_editor.selectedWaypoint].z = worldZ;
                     
                     /* Régénération du mesh */
                     generateTrackMesh(track);
                     track->isModified = 1;
                 }
             } else if (_dragMode == 2) {
                 /* Déplacement de la caméra */
                 int dx = event->motion.x - _dragStartX;
                 int dy = event->motion.y - _dragStartY;
                 
                 /* Calculer le déplacement dans l'espace 3D */
                 float scale = _editor.cameraZoom / 10.0f;
                 _editor.cameraX = _dragStartWorldX - dx * scale;
                 _editor.cameraZ = _dragStartWorldZ + dy * scale;
             } else if (_dragMode == 3) {
                 /* Zoom */
                 int dy = event->motion.y - _dragStartY;
                 _editor.cameraZoom += dy * 0.1f;
                 
                 /* Limites de zoom */
                 if (_editor.cameraZoom < 5.0f) _editor.cameraZoom = 5.0f;
                 if (_editor.cameraZoom > 50.0f) _editor.cameraZoom = 50.0f;
                 
                 _dragStartY = event->motion.y;
             }
             break;
             
         case SDL_MOUSEWHEEL:
             /* Zoom avec la molette */
             _editor.cameraZoom -= event->wheel.y * 1.0f;
             
             /* Limites de zoom */
             if (_editor.cameraZoom < 5.0f) _editor.cameraZoom = 5.0f;
             if (_editor.cameraZoom > 50.0f) _editor.cameraZoom = 50.0f;
             break;
             
         case SDL_KEYDOWN:
             switch (event->key.keysym.sym) {
                 case SDLK_ESCAPE:
                     /* Quitter l'éditeur */
                     exitTrackEditor();
                     break;
                     
                 case SDLK_s:
                     /* Sauvegarder (avec Ctrl) */
                     if (event->key.keysym.mod & KMOD_CTRL) {
                         if (track) {
                             saveTrackToFile(track, "circuit.track");
                         }
                     }
                     break;
                     
                 case SDLK_n:
                     /* Nouveau (avec Ctrl) */
                     if (event->key.keysym.mod & KMOD_CTRL) {
                         Track* newTrack = createEmptyTrack("Nouveau circuit");
                         if (newTrack) {
                             if (_editor.currentTrack) {
                                 destroyTrack(_editor.currentTrack);
                             }
                             _editor.currentTrack = newTrack;
                             _editor.selectedSegment = -1;
                             _editor.selectedWaypoint = -1;
                         }
                     }
                     break;
                     
                 case SDLK_DELETE:
                     /* Supprimer le segment sélectionné */
                     if (_editor.editMode == 0 && _editor.selectedSegment >= 0) {
                         removeTrackSegment(track, _editor.selectedSegment);
                         if (_editor.selectedSegment >= track->segmentCount) {
                             _editor.selectedSegment = track->segmentCount - 1;
                         }
                         generateWaypoints(track);
                         generateTrackMesh(track);
                     }
                     break;
             }
             break;
     }
 }
 
 /* Passage du jeu à l'éditeur de circuit */
 void enterTrackEditor(void) {
     /* Initialiser l'éditeur avec le circuit actuel */
     initTrackEditor(_currentTrack);
     
     /* Remplacer temporairement les fonctions de callback */
     gl4duwDisplayFunc(renderTrackEditor);
     gl4duwKeyDownFunc(NULL); /* Désactiver les contrôles du jeu */
     gl4duwKeyUpFunc(NULL);
     gl4duwMouseFunc(handleTrackEditorInput);
 }
 
 /* Sortie de l'éditeur de circuit */
 void exitTrackEditor(void) {
     /* Désactiver l'éditeur */
     _editorActive = 0;
     
     /* Restaurer les fonctions de callback du jeu */
     gl4duwDisplayFunc(draw);
     gl4duwKeyDownFunc(keydown);
     gl4duwKeyUpFunc(keyup);
     gl4duwMouseFunc(NULL);
     
     /* Mettre à jour le circuit courant dans le jeu */
     if (_editor.currentTrack) {
         /* Copier vers _currentTrack ou remplacer directement */
         _currentTrack = _editor.currentTrack;
     }
 }
 
 /* Conversion d'une position 3D en coordonnées d'écran 2D */
 void worldToScreen(float worldX, float worldY, float worldZ, int* screenX, int* screenY) {
     /* Cette fonction nécessite l'accès aux matrices de projection et de vue */
     /* Utilisation de gluProject pour simplifier */
     double modelview[16], projection[16];
     int viewport[4] = {0, 0, _ww, _wh};
     double winX, winY, winZ;
     
     gl4duGetMatrixf("view", (float*)modelview);
     gl4duGetMatrixf("proj", (float*)projection);
     
     gluProject(worldX, worldY, worldZ, modelview, projection, viewport, &winX, &winY, &winZ);
     
     *screenX = (int)winX;
     *screenY = _wh - (int)winY; /* Inversion Y car OpenGL et SDL ont des origines Y différentes */
 }
 
 /* Conversion de coordonnées d'écran 2D en position 3D */
 void screenToWorld(int screenX, int screenY, float* worldX, float* worldY, float* worldZ) {
     /* Cette fonction nécessite l'accès aux matrices de projection et de vue */
     /* Utilisation de gluUnProject pour simplifier */
     double modelview[16], projection[16];
     int viewport[4] = {0, 0, _ww, _wh};
     double objX, objY, objZ;
     
     gl4duGetMatrixf("view", (float*)modelview);
     gl4duGetMatrixf("proj", (float*)projection);
     
     /* En supposant que le plan du circuit est à y=0 */
     gluUnProject(screenX, _wh - screenY, 0.0, modelview, projection, viewport, &objX, &objY, &objZ);
     
     *worldX = (float)objX;
     *worldY = 0.0f; /* Circuit toujours à hauteur 0 */
     *worldZ = (float)objZ;
 }
 
 /* Gestion d'un bouton de l'interface */
 static int handleEditorButton(int x, int y, int w, int h, const char* label, int active) {
     /* Vérifier si la souris est sur le bouton */
     int mouseX, mouseY;
     Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
     
     int hover = (mouseX >= x && mouseX < x + w && mouseY >= y && mouseY < y + h);
     
     /* Dessiner le bouton */
     gl4duBindMatrix("model");
     gl4duLoadIdentityf();
     gl4duTranslatef(x + w/2, y + h/2, 0.0f);
     gl4duScalef(w, h, 1.0f);
     gl4duSendMatrices();
     
     /* Couleur selon état */
     if (!active) {
         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.5f, 0.5f, 0.5f, 0.5f);
     } else if (hover) {
         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.6f, 0.6f, 0.9f, 1.0f);
     } else {
         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.4f, 0.4f, 0.7f, 1.0f);
     }
     
     gl4dgDraw(_carBody);
     
     /* TODO: Afficher le texte du bouton */
     
     /* Vérifier si le bouton a été cliqué */
     return active && hover && (mouseState & SDL_BUTTON(1)) && !_dragMode;
 }
 
 /* Gestion d'un slider de l'interface */
 static float handleEditorSlider(int x, int y, int w, int h, const char* label, float value, float min, float max) {
     /* Vérifier si la souris est sur le slider */
     int mouseX, mouseY;
     Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
     
     int hover = (mouseX >= x && mouseX < x + w && mouseY >= y && mouseY < y + h);
     
     /* Position normalisée */
     float norm = (value - min) / (max - min);
     if (norm < 0.0f) norm = 0.0f;
     if (norm > 1.0f) norm = 1.0f;
     
     /* Dessiner le fond du slider */
     gl4duBindMatrix("model");
     gl4duLoadIdentityf();
     gl4duTranslatef(x + w/2, y + h/2, 0.0f);
     gl4duScalef(w, h, 1.0f);
     gl4duSendMatrices();
     
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.3f, 0.3f, 0.3f, 1.0f);
     gl4dgDraw(_carBody);
     
     /* Dessiner la partie remplie */
     gl4duBindMatrix("model");
     gl4duLoadIdentityf();
     gl4duTranslatef(x + (w * norm)/2, y + h/2, 0.0f);
     gl4duScalef(w * norm, h, 1.0f);
     gl4duSendMatrices();
     
     glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.4f, 0.6f, 0.9f, 1.0f);
     gl4dgDraw(_carBody);
     
     /* Dessiner le curseur */
     gl4duBindMatrix("model");
     gl4duLoadIdentityf();
     gl4duTranslatef(x + w * norm, y + h/2, 0.0f);
     gl4duScalef(h, h, 1.0f);
     gl4duSendMatrices();
     
     if (hover) {
         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.7f, 0.7f, 1.0f, 1.0f);
     } else {
         glUniform4f(glGetUniformLocation(_pId, "couleur"), 0.6f, 0.6f, 0.9f, 1.0f);
     }
     
     gl4dgDraw(_carBody);
     
     /* TODO: Afficher le texte du label */
     
     /* Vérifier si le slider est manipulé */
     if (hover && (mouseState & SDL_BUTTON(1))) {
         float newNorm = (float)(mouseX - x) / w;
         if (newNorm < 0.0f) newNorm = 0.0f;
         if (newNorm > 1.0f) newNorm = 1.0f;
         
         return min + newNorm * (max - min);
     }
     
     return value;
 }
 
 /* Gestion du clic sur un segment */
 static void handleEditorSegmentClick(Track* track, int index) {
     if (index >= 0 && index < track->segmentCount) {
         _editor.selectedSegment = index;
         _editor.selectedWaypoint = -1;
     }
 }
 
 /* Gestion du clic sur un waypoint */
 static void handleEditorWaypointClick(Track* track, int index) {
     if (index >= 0 && index < track->waypointCount) {
         _editor.selectedWaypoint = index;
         _editor.selectedSegment = -1;
     }
 }
 
 /* Ajout d'un nouveau segment */
 static void addNewSegment(Track* track, TrackSegmentType type) {
     if (!track) return;
     
     /* Ajouter après le segment sélectionné ou à la fin */
     int insertPos = (_editor.selectedSegment >= 0) ? _editor.selectedSegment + 1 : track->segmentCount;
     
     /* Créer un segment temporaire */
     TrackSegment newSegment;
     newSegment.type = type;
     newSegment.length = (type == TRACK_STRAIGHT) ? 20.0f : 15.0f;
     newSegment.curvature = (type == TRACK_STRAIGHT) ? 0.0f : 0.2f;
     newSegment.width = DEFAULT_TRACK_WIDTH;
     newSegment.textureId = 0;
     
     /* Insérer le segment */
     TrackSegment* newSegments = (TrackSegment*)realloc(track->segments, 
                                                     (track->segmentCount + 1) * sizeof(TrackSegment));
     if (!newSegments) {
         fprintf(stderr, "Erreur d'allocation mémoire pour l'ajout d'un segment\n");
         return;
     }
     
     track->segments = newSegments;
     
     /* Décaler les segments après le point d'insertion */
     for (int i = track->segmentCount; i > insertPos; i--) {
         track->segments[i] = track->segments[i - 1];
     }
     
     /* Insérer le nouveau segment */
     track->segments[insertPos] = newSegment;
     track->segmentCount++;
     _editor.selectedSegment = insertPos;
     
     /* Régénérer les waypoints et le mesh */
     generateWaypoints(track);
     generateTrackMesh(track);
     track->isModified = 1;
 }
 
 /* Duplication d'un segment */
 static void duplicateSegment(Track* track, int index) {
     if (!track || index < 0 || index >= track->segmentCount) return;
     
     /* Créer un segment temporaire comme copie */
     TrackSegment copySegment = track->segments[index];
     
     /* Insérer le segment */
     TrackSegment* newSegments = (TrackSegment*)realloc(track->segments, 
                                                     (track->segmentCount + 1) * sizeof(TrackSegment));
     if (!newSegments) {
         fprintf(stderr, "Erreur d'allocation mémoire pour la duplication d'un segment\n");
         return;
     }
     
     track->segments = newSegments;
     
     /* Décaler les segments après le point d'insertion */
     for (int i = track->segmentCount; i > index + 1; i--) {
         track->segments[i] = track->segments[i - 1];
     }
     
     /* Insérer la copie */
     track->segments[index + 1] = copySegment;
     track->segmentCount++;
     _editor.selectedSegment = index + 1;
     
     /* Régénérer les waypoints et le mesh */
     generateWaypoints(track);
     generateTrackMesh(track);
     track->isModified = 1;
 }
 
 /* Copie d'un segment */
 static void copySegment(Track* track, int index) {
     if (!track || index < 0 || index >= track->segmentCount) return;
     
     /* Libérer le segment dans le presse-papier si existant */
     if (_clipboardSegment) {
         free(_clipboardSegment);
     }
     
     /* Allouer et copier le segment */
     _clipboardSegment = (Track*)malloc(sizeof(Track));
     if (!_clipboardSegment) {
         fprintf(stderr, "Erreur d'allocation mémoire pour la copie d'un segment\n");
         return;
     }
     
     /* Initialiser la structure */
     _clipboardSegment->segmentCount = 1;
     _clipboardSegment->segments = (TrackSegment*)malloc(sizeof(TrackSegment));
     if (!_clipboardSegment->segments) {
         fprintf(stderr, "Erreur d'allocation mémoire pour la copie d'un segment\n");
         free(_clipboardSegment);
         _clipboardSegment = NULL;
         return;
     }
     
     /* Copier le segment */
     _clipboardSegment->segments[0] = track->segments[index];
     
     /* Autres champs non utilisés pour le presse-papier */
     _clipboardSegment->waypointCount = 0;
     _clipboardSegment->waypoints = NULL;
     _clipboardSegment->trackMesh = 0;
     _clipboardSegment->trackTexture = 0;
     _clipboardSegment->isModified = 0;
 }
 
 /* Coller un segment */
 static void pasteSegment(Track* track, int index) {
     if (!track || !_clipboardSegment || index < 0 || index >= track->segmentCount) return;
     
     /* Insérer le segment */
     TrackSegment* newSegments = (TrackSegment*)realloc(track->segments, 
                                                     (track->segmentCount + 1) * sizeof(TrackSegment));
     if (!newSegments) {
         fprintf(stderr, "Erreur d'allocation mémoire pour le collage d'un segment\n");
         return;
     }
     
     track->segments = newSegments;
     
     /* Décaler les segments après le point d'insertion */
     for (int i = track->segmentCount; i > index + 1; i--) {
         track->segments[i] = track->segments[i - 1];
     }
     
     /* Insérer la copie */
     track->segments[index + 1] = _clipboardSegment->segments[0];
     track->segmentCount++;
     _editor.selectedSegment = index + 1;
     
     /* Régénérer les waypoints et le mesh */
     generateWaypoints(track);
     generateTrackMesh(track);
     track->isModified = 1;
 }
 
 /* Couper un segment */
 static void cutSegment(Track* track, int index) {
     if (!track || index < 0 || index >= track->segmentCount) return;
     
     /* Copier d'abord le segment */
     copySegment(track, index);
     
     /* Puis le supprimer */
     removeTrackSegment(track, index);
     
     /* Mettre à jour la sélection */
     if (index >= track->segmentCount) {
         _editor.selectedSegment = track->segmentCount - 1;
     } else {
         _editor.selectedSegment = index;
     }
     
     /* Les waypoints ont déjà été régénérés par removeTrackSegment */
 }