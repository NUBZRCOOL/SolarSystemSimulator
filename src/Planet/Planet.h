#include "../Object/Object.h"
#include <string>
#include <math.h>
#include <functional>
#define GLM_ENABLE_EXPERIMENTAL
#include "../vendor/glm/gtc/quaternion.hpp"
#include "../vendor/glm/gtx/quaternion.hpp"
#include "../ParametricCurve/ParametricCurve.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "../vendor/glm/gtc/quaternion.hpp"
#include "../vendor/glm/gtx/quaternion.hpp"



struct OrbitalParameters {
    double r;
    double a;
    double ecc;
    double i;
    double L;
    double w;
    double O;
};

struct RotationParameters {
    double l0;
    double b0;
    double W0;
    double dW;
};

struct InitialParameters {
    double r;
    double a_0;
    double ecc_0;
    double i_0;
    double L_0;
    double w_0;
    double O_0;
};

struct OrbitalDerivatives {
    double da;
    double decc;
    double di;
    double dL;
    double dw;
    double dO;
    double b0;
    double c0;
    double s0;
    double f0;
};

struct RotParams {
    double l_0;
    double b_0;
    double W_0;
    double dW;
};


class Planet {
public:
    // Constructor declaration
    Planet(const char *path, InitialParameters initParams, OrbitalDerivatives derivs, RotParams rot);
    // Member variable declaration
    Object& getPlanet();
    void solveEccAnom(double T);
    void updateParams(double T);
    void updatePos();
    void updateRot(double T);
    void update(double T, glm::dvec3 cameraPos);
    void calcMeanAnom(double T);
    void drawCurve(Shader& curveShader, glm::mat4 viewMat, glm::mat4 proj, glm::vec2 aspect);
    std::array<std::function<double(double)>, 3> getP();
    std::array<std::function<double(double)>, 3> p;
    
    double meanAnom;
    double eccAnom;

    double omega;
    double M_0;
    double dM;

    double semiMaj;
    double semiMin;
    double ecc;
    InitialParameters initParams;
    OrbitalDerivatives derivs;
    OrbitalParameters params;
    RotParams rot;
    
    ParametricCurve curve;
    Object planet;
private:

};