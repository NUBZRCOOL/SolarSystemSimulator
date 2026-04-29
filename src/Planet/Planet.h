#include "../Object/Object.h"
#include <string>
#include <functional>
#include "../ParametricCurve/ParametricCurve.h"


struct OrbitalParameters {
    double r;
    double a;
    double ecc;
    double i;
    double L;
    double w;
    double O;
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
    double de;
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


class Planet {
public:
    // Constructor declaration
    Planet(const char *path, InitialParameters initParams, OrbitalDerivatives derivs);
    // Member variable declaration
    Object& getPlanet();
    void solveEccAnom();
    void updateParams(double T);
    void updatePos();
    void updateRot();
    void update();
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
    
    ParametricCurve curve;
    Object planet;
private:

};