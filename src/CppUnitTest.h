#ifndef  CPPUNIT_TEST_H
#define  CPPUNIT_TEST_H

#include <cppunit/TestAssert.h>
#include "NiTE.h"

class CPPunitTest
{
    public :
    
    /*!\brief Fonction qui teste si l'initialisation des effets sonore s'est bien déroulée (si returnValue == 0) tous s'est bien passé */
    void Test_ReturnValue(int returnValue)
    {
        CPPUNIT_ASSERT_EQUAL(returnValue, 0);
    }
    
    /*!\brief Fonction qui teste si l'initialisation du device, et des librairies NITE et OpenNI . (si returnValue == openni::STATUS_OK) tous s'est bien passé */
    void Test_DeviceInit(openni::Status returnValue)
    {
        CPPUNIT_ASSERT_EQUAL(returnValue, openni::STATUS_OK);
    }
};

#endif
