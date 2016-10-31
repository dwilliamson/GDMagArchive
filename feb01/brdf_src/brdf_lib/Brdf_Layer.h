#ifndef BRDF_LAYER_H
#define BRDF_LAYER_H

#include "global.h"
#include "Brdf_Image.h"

class ifstream;
class ofstream;

// Stores a layer of a BRDF Map (to use in conjunction with OpenGL)
// There are: 3D BRDF-Layers for isotropic BRDFs
//               a Layer consists of a 2D (size w*h) and a 1D-Texture (size h).
//            4D BRDF-Layers for anisotropic BRDFs
//               a Layer consists of two 2D (size w*h)

class Brdf_Layer {
public:
    enum Brdf_Texture {
        TEXTURE1 = 0,
	TEXTURE2 = 1,
	TEXTURE_AMBIENT = 2
    };

    Brdf_Layer(int width, int height);
    Brdf_Layer(char *filename);
    Brdf_Layer(ifstream &stream);
    ~Brdf_Layer();

    // Query

    bool is_loaded();

    Brdf_Image *get_image(Brdf_Texture t) { return tex[t]; }

    // These may only be queried after makeBiasedLayer()
    // or after calculateBiasFactors()!
    Double getAlpha1() const { return alpha1; }
    Double getAlpha2() const { return alpha2; }
    Double getBeta1()  const { return beta1; }
    Double getBeta2()  const { return beta2; }
    Double getLambda() const { return lambda; }
    Double getDelta()  const { return delta; }

    // Ops

    void setLambda( Double l ) { lambda = l; delta = l; }

    void convertToGLSphereLayer();
    void convertToAngleSphereLayer();
    void convertToDeltaLayer();
    void makeBiasedLayer();
    void makeBiasedLayer(Double a1, Double a2, Double b1, Double b2);
    void shuffleLambdaToTextures();
    void calculateBiasFactors();

    bool save(char *header);
    bool save(ofstream &out);

    Brdf_Image *tex[3];

    double ambient_lambda;

    Double lambda;
    Double delta;

  protected:
    enum SphereType { GLSphere=1, AngleSphere=2  };
    void convertToSphereLayer( SphereType t );

    Double alpha1, alpha2;
    Double beta1, beta2;
};

#endif // BRDF_LAYER_H



