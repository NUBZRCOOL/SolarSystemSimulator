#include "Planet.h"

Planet::Planet(const char *path, float semiMajor, float eccen, float meanAno, float eccAno) : planet(path) {
    semiMaj = semiMajor;
    ecc = eccen;
    meanAnom = meanAno;
    eccAnom = eccAno;
    semiMin = semiMaj * sqrt(1 - pow(ecc, 2));
    p = {
        [&](float f) { return semiMaj * (cos(f) - ecc); },
        [&](float f)  { return planet.getPosition().y; },
        [&](float f) { return semiMin * sin(f); }
    };
    ParametricCurve curve;
    
}
Object& Planet::getPlanet() {
    return planet;
}

std::array<std::function<float(float)>, 3> Planet::getP() {
    return p;
}


void Planet::solveEccAnom() {
    float delta = 1.0f;
    float precision = 0.00001f;
    int iterations = 0;
    int maxIterations = 100;

    while (std::abs(delta) > precision && iterations < maxIterations) {
        float f_E = eccAnom - (ecc * sin(eccAnom)) - meanAnom;
        float fPrime_E = 1.0f - (ecc * cos(eccAnom));
        
        delta = f_E / fPrime_E;
        eccAnom = eccAnom - delta;
        
        iterations++;
    }
}

void Planet::updatePos() {
    semiMin = semiMaj * sqrt(1 - pow(ecc, 2));
    planet.setPosition(glm::vec3(
        p[0](eccAnom),
        p[1](0.0f),
        p[2](eccAnom)
    ));
}

void Planet::updateRot() {
    planet.setRotation(glm::vec3(
        planet.getRotationEuler().x,
        -glfwGetTime()*(180/AI_MATH_PI),
        planet.getRotationEuler().z
    ));
}

void Planet::update() {
    updatePos();
    updateRot();
}

