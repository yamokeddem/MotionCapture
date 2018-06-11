
#ifndef WINDOW_H
#define WINDOW_H

#include <GL4D/gl4du.h>

#define CALIBRATION             0
#define MENU                    1
#define MOCAP                   2
#define GAME                    3

extern int _windowWidth;
extern int _windowHeight;
extern GLuint _pIdFBX ;
extern GLuint _pId;

typedef struct cam_t cam_t;
struct cam_t {
    GLfloat x,y, z;
    GLfloat theta;
    GLfloat phi;
};
extern cam_t _cam;

struct SLayout
{
    GLuint surface;
    int x;
    int y;
    int width;
    int height;
};
typedef struct SLayout *pLayout;

extern int MODE;
extern void quit(void);
extern void adaptTheLandmark();


//PAGE PRINCIPALE DE LA DOCUMENTION DOXYGEN

/*! \mainpage Projet LZW - Algorithmique - Documentation
 *
 * \section intro_sec Introduction
 *
 * Bienvenue sur la documentation du programme Motion Capture Kinect.
 *
 * Ce programme a été réalisée par Nehari Mohamed, et Mokedemm Yanis dans le cadre d'un projet tuteuré de Master 1 Informatique à l'Université Paris 8,
 * <BR>
 *
 * \section fonctions Fonctionalitées du programme
 * Ce programme comporte plusieurs étapes :
 * - Calibration : première étape du programme permettant à l'utilisateur d'être détécté et calibré par la caméra
 * - Menu principal : Menu offrant à l'utilisateur la possibilité de choisir entre les deux modes du programmes
 * - Motion Capture : Interface chargant plusieurs modèles 3D FBX. Les modèles reproduisent les mouvements de l'utilisateur dans un repère deux dimensions
 * - Jeux d'avion : Interface chargant un avion manipulable par les gestes de l'utilisateur.
 * <BR><BR>
 *
 * \section calibration Calibration
 * Il est nécéssaire pour la détéction de l'utilisateur par la caméra que celui-ci soit intégralement visible par la caméra de la tête au pied.
 * Pour ce faire, placez-vous dans un endroit éspacé en essayant de ne pas inclure d'objet dans l'espace de détection, tels que des chaises, ou encore d'autres utilisateurs.
 *
 * Lors de l'étape de calibration, patientez devant la caméra en effectuant de léger déplacement et de légère rotation des bras afin que la caméra vous détècte.
 * Une fois détécté, le corp de l'utilisateur se colorera en vert, et débutera alors l'étape de calibration du squelette. Patientez alors dans une position fixe en écartant
 * légèrement les bras le long du corp, afin que les membres soit détéctés. Une fois tous les membres calibrés,  déplacez votre main sur le bouton "Next" qui s'affichera en
 * haut à droite de l'écran afin d'accéder au menu principal.
 * <BR><BR>
 *
 * \section menu Menu principal
 * Le menu principal offre la possibilité à l'utilisateur de choisir entre deux différents mode : La motion capture avec Avatar 3D, ou la manipulation d'un avion via les
 * gestes de l'utilisateur
 * Afin de choisir l'un des deux modes, déplacez vos deux mains sur un des boutons, et patientez durant le compte à rebours de 3 seconde
 * <BR><BR>
 *
 * \section motion Motion Capture
 * Le mode Avatar Motion Capture permet à l'utilisateur de voir ses mouvements reproduis par des modèle 3D sur un repère en deux dimensions. (Les mouvements en profondeur
 * n'étant pas fonctionnel pour l'instant ils ne sont pas présentdans cette version du programme).
 * <BR><BR>
 *
 * \section jeux Airplane Game
 * Le mode airplane game permet à l'utilisateur de contrôler un avion avec ses bras en effectuant des rotations de droites à gauche. Afin de contrôle l'avion, placez votre
 * corp dans une position verticale, puis levez les deux bras horizontalement. Enfin éfféctuez des mouvements de tous le buste de droite à gauche afin de faire pivoter
 * l'avion.
 * <BR><BR>
 *
 * \section retour Retour à la page précédente
 * Afin de retourner au Menu principal, effectuez avec vos bras un mouvement en forme de X duarnt deux seconde.
 * <BR><BR>
 *
 * \section exit Quitter le programme
 * Afin de quitter le programme, retourner au menu principal, puis effectuez avec vos bras un mouvement en forme de X durant deux seconde.
 * <BR><BR>
 *
 * \section remerciement Remerciements
 * Nous tenons à remercier chaleureusement Monsieur Fares Belhadj pour ses précieux conseils durant ce semestre, qui nous ont permis d'avancer dans l'accomplissement de ce
 * programme et dans la résolutions de certains de nos problèmes.
 *
 * <BR><BR>
 */



#endif
