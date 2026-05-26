#include "Planet.h"

Planet::Planet(const char *path, InitialParameters initParams, OrbitalDerivatives derivs, RotParams rot) : planet(path) {

    this->initParams = initParams;
    this->derivs = derivs;
    this->rot = rot;
    
    planet.setScale(glm::vec3(initParams.r));

    double a_0 = initParams.a_0;
    double ecc_0 = initParams.ecc_0;
    double i_0 = glm::radians(initParams.i_0);
    double L_0 = glm::radians(initParams.L_0);
    double w_0 = glm::radians(initParams.w_0);
    double O_0 = glm::radians(initParams.O_0);

    this->initParams.i_0 = i_0;
    this->initParams.L_0 = L_0;
    this->initParams.w_0 = w_0;
    this->initParams.O_0 = O_0;

    this->derivs.di = glm::radians(derivs.di);
    this->derivs.dL = glm::radians(derivs.dL);
    this->derivs.dw = glm::radians(derivs.dw);
    this->derivs.dO = glm::radians(derivs.dO);

    M_0 = initParams.L_0 - w_0;
    omega = params.w - params.O;
    dM = derivs.dL - derivs.dw;

    semiMaj = params.a;
    semiMin = semiMaj * sqrt(1 - pow(ecc, 2));
    ecc = params.ecc;
    p = {
        [this](float f) { return (cos(omega)*cos(params.O)-sin(params.O)*sin(omega)*cos(params.i))*(semiMaj * (cos(f) - ecc)) + (-sin(omega)*cos(params.O)-cos(omega)*sin(params.O)*cos(params.i))*(semiMin * sin(f)); },
        [this](float f) { return (sin(omega)*sin(params.i))*(semiMaj * (cos(f) - ecc)) + (cos(omega)*sin(params.i))*(semiMin * sin(f)); },
        [this](float f) { return -((cos(omega)*sin(params.O)+cos(params.O)*sin(omega)*cos(params.i))*(semiMaj * (cos(f) - ecc)) + (-sin(omega)*sin(params.O)+cos(omega)*cos(params.O)*cos(params.i))*(semiMin * sin(f))); }
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
    updateParams(T);
    double t_0 = 946684800;
    double T_real = T;
    double t_elapsed = T_real - t_0;
    double t_cent = t_elapsed / (60 * 60 * 24 * 365.25 * 100);
    meanAnom = glm::mod((initParams.L_0 + derivs.dL*t_cent) - (initParams.w_0 + derivs.dw * t_cent) + (derivs.b0 * pow(t_cent, 2)) + (derivs.c0*cos(glm::radians(derivs.f0)*t_cent)) + (derivs.s0*sin(glm::radians(derivs.f0)*t_cent)) , (2 * glm::pi<double>()));
}


void Planet::solveEccAnom(double T) {

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

    params.a = initParams.a_0 + derivs.da * t_cent;
    params.ecc = initParams.ecc_0 + derivs.decc * t_cent;
    params.i = initParams.i_0 + derivs.di * t_cent;
    params.L = initParams.L_0 + derivs.dL * t_cent;
    params.w = initParams.w_0 + derivs.dw * t_cent;
    params.O = initParams.O_0 + derivs.dO * t_cent;

    omega = params.w - params.O;

    semiMaj = params.a;
    semiMin = semiMaj * sqrt(1 - pow(ecc, 2));
    ecc = params.ecc;
}

void Planet::updatePos() {
    semiMin = semiMaj * sqrt(1 - pow(ecc, 2));
    glm::dvec3 rawPos = glm::dvec3(
        p[0](eccAnom),
        p[1](eccAnom),
        p[2](eccAnom)
    );
    // glm::mat4 z1 = glm::rotate(glm::mat4(1.0f), (float)(-params.O), glm::vec3(0, 1, 0));
    // glm::mat4 x1 = glm::rotate(glm::mat4(1.0f), (float)(-params.i), glm::vec3(1, 0, 0));
    // glm::mat4 z2 = glm::rotate(glm::mat4(1.0f), (float)(-omega), glm::vec3(0, 1, 0));

    // glm::mat4 combined = z1 * x1 * z2;
    // planet.setPosition(glm::vec3(combined * glm::vec4(rawPos, 1.0f)));
    planet.setPosition(glm::vec3(rawPos));
}

void Planet::updateRot(double T) {

    double t_0 = 946684800;
    double currentW = rot.W_0 + ((rot.dW / 86400) * (T-t_0));
    currentW = fmod(currentW, 360.0);

    glm::quat spinQuat = glm::angleAxis(glm::radians((float)currentW - 90), glm::vec3(0, 1, 0));

    glm::vec3 pole;
    float latRad = glm::radians((float)rot.b_0);
    float longRad = glm::radians((float)rot.l_0 + 180.0f);
    pole.x = cos(latRad) * cos(longRad);
    pole.y = sin(latRad);
    pole.z = cos(latRad) * sin(longRad);

    glm::quat tiltQuat = glm::rotation(glm::vec3(0, 1, 0), glm::normalize(pole));

    planet.setRotation(tiltQuat * spinQuat);
}

void Planet::update(double T, glm::dvec3 cameraPos) {

    // Calculate relative position to camera
    glm::dvec3 relPos = glm::dvec3(p[0](eccAnom), p[1](eccAnom), p[2](eccAnom)) - cameraPos; 
    float dist = (float)glm::length(relPos); 

    // 1. Determine the "Visual Radius"
    // This constant (0.002) controls the minimum size of the "star" point.
    float minVisualRadius = dist * 0.0015f; 
    float realRadius = (float)initParams.r;

    float finalScale;
    float pointIntensity = 0.0f;

    // 2. Transition Logic
    if (minVisualRadius > realRadius) {
    finalScale = minVisualRadius;
    
        // Using a power function (like pow(..., 2.0)) makes the "whitewash" 
        // kick in faster as the planet gets smaller on screen.
        float rawIntensity = glm::clamp((minVisualRadius - realRadius) / realRadius, 0.0f, 1.0f);
        pointIntensity = std::pow(rawIntensity, 0.5f); // Square root makes it reach "white" sooner
    } else {
        finalScale = realRadius;
        pointIntensity = 0.0f;
    }

    this->planet.setPosition(glm::vec3(relPos));
    this->planet.setScale(glm::vec3(finalScale));
    
    this->planet.updateShader();
    this->planet.shader->setFloat("pointIntensity", pointIntensity);

    updateRot(T);
    this->planet.updateModelMatrix();
}

void Planet::drawCurve(Shader& curveShader, glm::mat4 viewMat, glm::mat4 proj, glm::vec2 aspect) {
    curve.init(0, 2 * AI_MATH_PI_F, 100);
    curve.updateCurve(getP());
    curve.render(curveShader, viewMat, proj, 0.4, aspect);
}