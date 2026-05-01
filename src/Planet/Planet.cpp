#include "Planet.h"

Planet::Planet(const char *path, InitialParameters initParams, OrbitalDerivatives derivs) : planet(path) {

    this->initParams = initParams;
    this->derivs = derivs;
    
    planet.setScale(glm::vec3(initParams.r));

    double a_0 = initParams.a_0;
    double ecc_0 = initParams.ecc_0;
    double i_0 = glm::radians(initParams.i_0);
    double L_0 = glm::radians(initParams.L_0);
    double w_0 = glm::radians(initParams.w_0);
    double O_0 = glm::radians(initParams.O_0);

    params.a = a_0 + derivs.da;
    params.ecc = ecc_0 + derivs.decc;
    params.i = i_0 + derivs.di;
    params.L = L_0 + derivs.dL;
    params.w = w_0 + derivs.dw;
    params.O = O_0 + derivs.dO;

    semiMaj = params.a;
    semiMin = semiMaj * sqrt(1 - pow(ecc, 2));
    ecc = params.ecc;
    p = {
        [&](float f) { return semiMaj * (cos(f) - ecc); },
        [&](float f)  { return 0; },
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

void Planet::calcMeanAnom(double T) {
    double t_0 = 946684800;
    double T_real = T;
    double t_elapsed = T_real - t_0;
    double t_cent = t_elapsed / (60 * 60 * 24 * 365.25 * 100);
    meanAnom = glm::mod(initParams.L_0 + derivs.dL*t_cent - (initParams.w_0 + derivs.dw * t_cent), (2 * glm::pi<double>()));
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

void Planet::updateParams(double T) {

    double t_0 = 946684800;
    double T_real = T;
    double t_elapsed = T_real - t_0;
    double t_cent = t_elapsed / (60 * 60 * 24 * 365.25 * 100);

    double a_0 = initParams.a_0;
    double ecc_0 = initParams.ecc_0;
    double i_0 = glm::radians(initParams.i_0);
    double L_0 = glm::radians(initParams.L_0);
    double w_0 = glm::radians(initParams.w_0);
    double O_0 = glm::radians(initParams.O_0);

    params.a = a_0 + derivs.da * t_cent;
    params.ecc = ecc_0 + derivs.decc * t_cent;
    params.i = i_0 + derivs.di * t_cent;
    params.L = L_0 + derivs.dL * t_cent;
    params.w = w_0 + derivs.dw * t_cent;
    params.O = O_0 + derivs.dO * t_cent;

    M_0 = initParams.L_0 - w_0;
    omega = params.w - params.O;
    dM = derivs.dL - derivs.dw;

    semiMaj = params.a;
    semiMin = semiMaj * sqrt(1 - pow(ecc, 2));
    ecc = params.ecc;
}

void Planet::updatePos() {
    semiMin = semiMaj * sqrt(1 - pow(ecc, 2));
    glm::vec3 rawPos = glm::vec3(
        p[0](eccAnom),
        p[1](0.0),
        p[2](eccAnom)
    );
    glm::mat4 z1 = glm::rotate(glm::mat4(1.0f), (float)(-params.O), glm::vec3(0, 1, 0));
    glm::mat4 x1 = glm::rotate(glm::mat4(1.0f), (float)(-params.i), glm::vec3(1, 0, 0));
    glm::mat4 z2 = glm::rotate(glm::mat4(1.0f), (float)(-omega), glm::vec3(0, 1, 0));

    glm::mat4 combined = z1 * x1 * z2;
    planet.setPosition(glm::vec3(combined * glm::vec4(rawPos, 1.0f)));
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