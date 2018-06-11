#include "Window.h"
#include "Device.h"
#include "Logger.h"
#include <GL4D/gl4du.h>
#include "Mocap.h"

openni::Device        m_device;

/*!\brief Fonction qui cahrge la kinect, ainsi que les libraires OpenNI et Nite */
openni::Status initNiteAndOpenNi()
{
    //Initialisation de OpenNi
    openni::Status rc = openni::OpenNI::initialize();
    if (rc != openni::STATUS_OK)
    {
        printf("Failed to initialize OpenNI\n%s\n", openni::OpenNI::getExtendedError());
        return rc;
    }
    LOG4CXX_INFO(GlobalLogger, "Librairies OpenNI initialisee avec succes ! ");
    
    //Récupération du Device de capture
    const char* deviceUri = openni::ANY_DEVICE;
    
    //Chargement du Device de capture
    LOG4CXX_INFO(GlobalLogger, "Recherche d'une camera Kinect en cours ... ");
    rc = m_device.open(deviceUri);
    if (rc == openni::STATUS_OK)
    {
        LOG4CXX_INFO(GlobalLogger, "Camera Kinect initialisee avec succes ! ");
    }
    else
    {
        printf("Failed to open device\n%s\n", openni::OpenNI::getExtendedError());
        return rc;
    }
    
    //Initialisation de Nite
    nite::NiTE::initialize();
    m_pUserTracker = new nite::UserTracker;
    if(m_pUserTracker->create(&m_device) == nite::STATUS_OK)
    {
        LOG4CXX_INFO(GlobalLogger, "Librairies NITE initialisees avec succes ! ");
    }
    else
        return openni::STATUS_ERROR;
    
    //m_pUserTracker = m_pUserTracker.create();
    return openni::STATUS_OK;
}

/*!\brief Fonction qui libère la kinect ainsi que les librairies Nite et OpenNi */
void freeDevice()
{
    LOG4CXX_INFO(GlobalLogger, "Destruction du contexte NITE !");
    nite::NiTE::shutdown();
    LOG4CXX_INFO(GlobalLogger, "Destruction du contexte OPENNI !");
    openni::OpenNI::shutdown();
    LOG4CXX_INFO(GlobalLogger, "Fermeture de la Kinect !");
}
