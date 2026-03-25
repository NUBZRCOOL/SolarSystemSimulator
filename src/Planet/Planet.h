#include "../Object/Object.h"
#include <string>
#include <functional>
#include "../ParametricCurve/ParametricCurve.h"


struct OrbitalParameters {
    double r;
    double a;
    double ecc;
    double i;
    double L_0;
    double w;
    double O;
};


class Planet {
public:
    // Constructor declaration
    Planet(const char *path, OrbitalParameters params);
    // Member variable declaration
    Object& getPlanet();
    void solveEccAnom();
    void updatePos();
    void updateRot();
    void update();
    void drawCurve(Shader& curveShader, glm::mat4 viewMat, glm::mat4 proj, glm::vec2 aspect);
    std::array<std::function<double(double)>, 3> getP();
    std::array<std::function<double(double)>, 3> p;
    
    double meanAnom;
    double eccAnom;

    double semiMaj;
    double semiMin;
    double ecc;
    OrbitalParameters params;
    
    ParametricCurve curve;
    Object planet;
private:

};