#include "../Object/Object.h"
#include <string>
#include <functional>
#include "../ParametricCurve/ParametricCurve.h"


class Planet {
public:
    // Constructor declaration
    Planet(const char *path, float semiMaj, float ecc, float meanAnom, float eccAnom);
    // Member variable declaration
    Object& getPlanet();
    void solveEccAnom();
    void updatePos();
    void updateRot();
    void update();
    std::array<std::function<float(float)>, 3> getP();
    std::array<std::function<float(float)>, 3> p;
    
    float semiMaj;
    float ecc;
    float meanAnom;
    float eccAnom;
    float semiMin;
    ParametricCurve curve;
    Object planet;
private:

};