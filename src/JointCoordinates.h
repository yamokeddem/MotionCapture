
#ifndef  JOINT_COORDINATES_H_
#define  JOINT_COORDINATES_H_
#include <math.h>
#include <string.h>

class JointCoordinates
{
    public:
        float x;
        float y;
        float z;
        char *name;
        const char *member_name;
        float lastAngle;
        float angle;
        float angle3D;
        bool available;
        bool dependenciesAvailable;

    public:
    
    /*!\brief Constructeur */
    JointCoordinates()
    {
        x = 0.0;
        y = 0.0;
        z = 0.0;
        angle = 0.0;
        lastAngle = 0.0;
        angle3D = 0.0;
        available               = false;
        dependenciesAvailable   = false;
    }
    
    /*!\brief Fonction setters qui attribue des coordonnées x, y et z  */
    void setCoordinates(float a, float b, float c)
    {
        x = a;
        y = b;
        z = c;
    }
    
    /*!\brief Fonction qui retourne TRUE si le membre est disponible et FALSE dans le cas contraire */
    bool isAvailable()
    {
        return available;
    }
    
    /*!\brief Fonction qui  analyse si les membres dont dépent le membre courant sont disponible (nécéssaire pour le calcul de l'angle)*/
    bool areDependenciesAvailable(JointCoordinates* a, JointCoordinates* b)
    {
        if(a->available && available && b->available)
            return true;
        return false;
    }
    
    /*!\brief Fonction qui calcule l'angle entre trois membres (fonctionne uniquement en 2D) */
    void calculateAngle(JointCoordinates* a, JointCoordinates* c)
    {
        //Si l'un des points n'est pas disponible
        dependenciesAvailable = areDependenciesAvailable(a, c);
        if(dependenciesAvailable == false)
            return;
        
        JointCoordinates* ab = new JointCoordinates();
        JointCoordinates* cb = new JointCoordinates();
        
        ab->x = this->x - a->x ;
        ab->y = this->y - a->y ;
        
        cb->x = this->x - c->x ;
        cb->y = this->y - c->y ;
        
        float dot = (ab->x * cb->x + ab->y * cb->y);
        float cross = (ab->x * cb->y - ab->y * cb->x);
        float alpha = atan2(cross, dot);
        
        angle = floor(alpha * 180. / M_PI + 0.5);
    }
    
    /*!\brief Fonction qui calcule l'angle entre 3 points (fonctionne en 3D)*/
    void calculateAngle3D(JointCoordinates* a, JointCoordinates* c)
    {
        //Si l'un des points n'est pas disponible
        dependenciesAvailable = areDependenciesAvailable(a, c);
        if(dependenciesAvailable == false)
            return;
        
        float Ax, Ay, Az, Bx, By, Bz, Cx, Cy, Cz;
        
        Ax = a->x;     Ay = a->y;  Az = a->z;
        Bx = this->x;  By = this->y;  Bz = this->z;
        Cx = c->x;     Cy = c->y;     Cz = c->z;
        
        float vector_BA_x = Ax-Bx;
        float vector_BA_y = Ay-By;
        float vector_BA_z = Az-Bz;
        float vector_BC_x = Cx-Bx;
        float vector_BC_y = Cy-By;
        float vector_BC_z = Cz-Bz;
        
        float magnitude_BA = sqrt(vector_BA_x*vector_BA_x + vector_BA_y*vector_BA_y + vector_BA_z*vector_BA_z);
        float magnitude_BC = sqrt(vector_BC_x*vector_BC_x + vector_BC_y*vector_BC_y + vector_BC_z*vector_BC_z);
        
        // (1) normalize vector BA and BC with the product of their magnitude
        float normalized_BA_x = vector_BA_x / magnitude_BA;
        float normalized_BA_y = vector_BA_y / magnitude_BA;
        float normalized_BA_z = vector_BA_z / magnitude_BA;
        float normalized_BC_x = vector_BC_x / magnitude_BC;
        float normalized_BC_y = vector_BC_y / magnitude_BC;
        float normalized_BC_z = vector_BC_z / magnitude_BC;
        
        // (2) calculate the dot product
        float dot = normalized_BA_x * normalized_BC_x  +  normalized_BA_y * normalized_BC_y  +  normalized_BA_z * normalized_BC_z;
        
        // (3) recover the angle
        float anglee = acos(dot);
        angle3D = anglee * 180 / 3.14159265359;        
    }
    
    /*!\brief Fonction qui retourne le nom du membre */
    char* getName()
    {
        return name;
    }
};

extern JointCoordinates*  TETE           ;
extern JointCoordinates*  NUQUE          ;
extern JointCoordinates*  EPAULE_DROITE  ;
extern JointCoordinates*  EPAULE_GAUCHE  ;
extern JointCoordinates*  COUDE_GAUCHE   ;
extern JointCoordinates*  COUDE_DROIT    ;
extern JointCoordinates*  MAIN_GAUCHE    ;
extern JointCoordinates*  MAIN_DROITE    ;
extern JointCoordinates*  TORSE          ;
extern JointCoordinates*  HANCHE_GAUCHE  ;
extern JointCoordinates*  HANCHE_DROITE  ;
extern JointCoordinates*  GENOU_DROIT    ;
extern JointCoordinates*  GENOU_GAUCHE   ;
extern JointCoordinates*  PIED_DROIT     ;
extern JointCoordinates*  PIED_GAUCHE    ;
extern JointCoordinates*  BASE_COLONNE_V ;
















#endif
