#pragma once
#include <Siv3D.hpp>
#include "Common.h"
#include "Level.h"

struct SpotLight
{
	Float3 position;
	float attenuation;
	Float3 diffuseColor;
	float exponent;
	Float3 direction;
	float cutoff;
};

class MazeScene : public MyApp::Scene {
public:
	MazeScene();
	void init();
	void update();
	void draw() const;

private:
	Level level;
	Camera camera;
	Vec2 pos;
	double lookAtTheta = 90_deg, lookAtPhi = 0;

	Mesh groundMesh;
	TransformedMesh ceilingMesh;

	const VertexShader vs;
	const PixelShader ps;
	ConstantBuffer<SpotLight> spotLightCB;
};

