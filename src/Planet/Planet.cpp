#include "Planet.h"

Planet::Planet(const char *path, OrbitalParameters params) : planet(path) {

    this->params = params;
    planet.setScale(glm::vec3(params.r));
    semiMaj = params.a;
    semiMin = semiMaj * sqrt(1 - pow(ecc, 2));
    ecc = params.ecc;
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

std::array<std::function<double(double)>, 3> Planet::getP() {
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
        p[1](0.0),
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

void Planet::drawCurve(Shader& curveShader, glm::mat4 viewMat, glm::mat4 proj, glm::vec2 aspect) {
    curve.init(0, 2 * AI_MATH_PI_F, 100);
    curve.updateCurve(p);
    curve.render(curveShader, viewMat, proj, 2.0, aspect);
}