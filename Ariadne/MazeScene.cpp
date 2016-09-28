#include "MazeScene.h"

MazeScene::MazeScene() : 
		vs(VertexShader(L"SpotLight.hlsl")),
		ps(PixelShader(L"SpotLight.hlsl")),
		pos(Vec2(5, 6)),
		groundMesh(Mesh(MeshData::Plane(300, 300, { 20, 20 }))),
		ceilingMesh(Mesh(MeshData::Plane(300, 300, { 100, 100 })).translated(0, WALL_HEIGHT, 0)),
		level(Level(L"a.csv")) {};


void MazeScene::init() {
	TextureAsset::Register(L"wall", L"Example/Brick.jpg", TextureDesc::For3D);
	TextureAsset::Register(L"ground", L"Example/Ground.jpg", TextureDesc::For3D);

	Graphics::SetBackground(ColorF(0.02, 0.02, 0.05));
	Graphics3D::SetLightForward(0, Light::None());
	Graphics3D::SetAmbientLightForward(ColorF(0.05));

	spotLightCB->attenuation = 0.01f;
	spotLightCB->diffuseColor = Float3(1, 0.9, 0.3);
	spotLightCB->exponent = 32.0f;
	spotLightCB->cutoff = static_cast<float>(30_deg);
}

void MazeScene::update() {
	const auto pointingDirection = (Mouse::PosF() - Window::Center()) / (Window::Width() * 10);
	lookAtPhi -= pointingDirection.x;
	lookAtTheta += pointingDirection.y;

	if (Input::MouseL.pressed || Input::MouseR.pressed) {
		const auto tempPos = pos + PLAYER_SPEED * (Input::MouseL.pressed - Input::MouseR.pressed) * Vec2::UnitX.rotated(lookAtPhi);
		if (!level.intersects(tempPos)) {
			pos = tempPos;
		}
	}

	// キーボード操作
	/*lookAtTheta += (Input::KeyDown.pressed - Input::KeyUp.pressed) * 3_deg;
	lookAtPhi += (Input::KeyLeft.pressed - Input::KeyRight.pressed) * 3_deg;

	auto tempPos = pos + SPEED * ((Input::KeyW.pressed - Input::KeyS.pressed) * Vec2::UnitX + (Input::KeyA.pressed - Input::KeyD.pressed) * Vec2::UnitY).rotated(lookAtPhi);
	if (!level.intersects(tempPos)) {
		pos = tempPos;
	}*/

	camera.pos = Vec3(pos.x, 2, pos.y);
	const auto direction = Vec3(Spherical(1, lookAtTheta, lookAtPhi)).normalized();
	camera.lookat = camera.pos + direction;
	Graphics3D::SetCamera(camera);

	spotLightCB->position = camera.pos;
	spotLightCB->direction = direction;
}

void MazeScene::draw() const {
	Graphics3D::SetConstantForward(ShaderStage::Pixel, 1, spotLightCB);
	Graphics3D::BeginVSForward(vs);
	Graphics3D::BeginPSForward(ps);

	groundMesh.drawForward(TextureAsset(L"ground"));
	ceilingMesh.drawForward(TextureAsset(L"wall"));
	level.drawForward();

	Graphics3D::EndVSForward();
	Graphics3D::EndPSForward();
}