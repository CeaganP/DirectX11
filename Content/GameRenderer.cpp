//http://users.skynet.be/fquake/
//https://github.com/microsoft/Windows-universal-samples/tree/master/Samples/Simple3DGameDX
//https://github.com/Microsoft/Windows-appsample-marble-maze

#include "pch.h"
#include "GameRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace DirectX11_Game;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
GameRenderer::GameRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources, float xOff, float yOff) :
	m_loadingComplete(false),
	m_degreesPerSecond(15),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources(xOff, yOff);

	// Initialize view for objects
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void GameRenderer::CreateWindowSizeDependentResources()
{
	//pass through an empty buffer, this will fill the buffer with the information
	ModelViewProjectionConstantBuffer* modelBuffer = nullptr;
	//fixed size of 2
	for (int i = 0; i < m_dataBufferSize; i++)
		//for (ModelViewProjectionConstantBuffer* modelBuffer : { &m_constantBufferData2 }) 
	{
		modelBuffer = &m_dataBuffers[i];

		Size outputSize = m_deviceResources->GetOutputSize();
		float aspectRatio = outputSize.Width / outputSize.Height;
		float fovAngleY = 70.0f * XM_PI / 180.0f;

		// This is a simple example of change that can be made when the app is in
		// portrait or snapped view.
		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		// Note that the OrientationTransform3D matrix is post-multiplied here
		// in order to correctly orient the scene to match the display orientation.
		// This post-multiplication step is required for any draw calls that are
		// made to the swap chain render target. For draw calls to other targets,
		// this transform should not be applied.

		// This sample makes use of a right-handed coordinate system using row-major matrices.
		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, 100.0f);

		XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(
			&modelBuffer->projection,
			XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

		// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
		static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
		static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
		static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		XMStoreFloat4x4(&modelBuffer->view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	}

}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void GameRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Perform General Calculations
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		double rotation = fmod(totalRotation, 2);

		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		for (int i = 0; i < m_dataBufferSize; i++)
		{
			float rad = radians;
			float rot = rotation / 2;

			//once the shape gets to as large as it will be, reduce it back to original
			if (rotation > 1)
			{
				rot = 1 - rot;
			}
			rot = 1;
			TranslateScaleRotate(rad, rot, XMFLOAT3({ (float)i * rot, 0, (float)i }), &m_dataBuffers[i]);
		}

	}
}

// Rotate the 3D cube model a set amount of radians.
void GameRenderer::Rotate(float radians, ModelViewProjectionConstantBuffer* modelBuffer)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&modelBuffer->model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

// Rotate the 3D cube model a set amount of radians.
void GameRenderer::Scale(float scaleAmt, ModelViewProjectionConstantBuffer* modelBuffer)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&modelBuffer->model, XMMatrixTranspose(XMMatrixScaling(scaleAmt, scaleAmt, scaleAmt)));
}

void GameRenderer::ScaleRotate(float radians, float scaleAmt, ModelViewProjectionConstantBuffer* modelBuffer)
{
	XMStoreFloat4x4(&modelBuffer->model, XMMatrixTranspose(XMMatrixScaling(scaleAmt, scaleAmt, scaleAmt) * XMMatrixRotationY(radians)));
}

void GameRenderer::Translate(XMFLOAT3 axis, ModelViewProjectionConstantBuffer* modelBuffer)
{
	XMStoreFloat4x4(&modelBuffer->model, XMMatrixTranspose(XMMatrixTranslation(axis.x, axis.y, axis.z)));
}

void GameRenderer::TranslateScaleRotate(float radians, float scaleAmt, XMFLOAT3 axis, ModelViewProjectionConstantBuffer* modelBuffer)
{
	XMStoreFloat4x4(&modelBuffer->model, XMMatrixTranspose(XMMatrixTranslation(axis.x, axis.y, axis.z) * XMMatrixRotationY(radians) * XMMatrixScaling(scaleAmt, scaleAmt, scaleAmt)));
}

void GameRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void GameRenderer::TrackingUpdate(float positionX, ModelViewProjectionConstantBuffer* modelBuffer)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians, modelBuffer);
	}
}

void GameRenderer::StopTracking()
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void GameRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}


	//for each buffer stored in the data buffers draw the value stored
	for (ModelViewProjectionConstantBuffer modelBuffer : m_dataBuffers)
	{
		DrawObject(modelBuffer);
	}
}

void GameRenderer::DrawObject(ModelViewProjectionConstantBuffer& modelBuffer)
{
	//this is where multiple objects are drawn
	auto context = m_deviceResources->GetD3DDeviceContext();

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &modelBuffer, 0, 0, 0);

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Each index is one 16-bit unsigned integer (short).
	//Input assembly -  You need to attach each vertex and index buffer for each object in the scene
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);
}

//Shapes
//https://github.com/microsoft/Windows-appsample-marble-maze/blob/master/C%2B%2B/Shared/BasicShapes.cpp 

//Initialize the shaders and shapes to be used later
void GameRenderer::CreateDeviceDependentResources(float xOff, float yOff)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
		});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
		});


	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then(
		[this]()
		{
			CreateCube();
		});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]() {
		m_loadingComplete = true;
		});
}

HRESULT GameRenderer::CreateCube()
{
	HRESULT hr = S_OK;

	// Use the Direct3D device to load resources into graphics memory.
	ID3D11Device* device = m_deviceResources->GetD3DDevice();

	// Cube Geometry.
	VertexPositionColor CubeVertices[] = {
			{DirectX::XMFLOAT3(-0.5f,-0.5f,-0.5f), DirectX::XMFLOAT3(0,   0,   0),},
			{DirectX::XMFLOAT3(-0.5f,-0.5f, 0.5f), DirectX::XMFLOAT3(0,   0,   1),},
			{DirectX::XMFLOAT3(-0.5f, 0.5f,-0.5f), DirectX::XMFLOAT3(0,   1,   0),},
			{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), DirectX::XMFLOAT3(0,   1,   1),},
			{DirectX::XMFLOAT3(0.5f,-0.5f,-0.5f), DirectX::XMFLOAT3(1,   0,   0),},
			{DirectX::XMFLOAT3(0.5f,-0.5f, 0.5f), DirectX::XMFLOAT3(1,   0,   1),},
			{DirectX::XMFLOAT3(0.5f, 0.5f,-0.5f), DirectX::XMFLOAT3(1,   1,   0),},
			{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), DirectX::XMFLOAT3(1,   1,   1),}
	};

	/*VertexPositionColor CubeVertices[] = {
			{DirectX::XMFLOAT3(-0.5f,-0.5f,-0.5f), DirectX::XMFLOAT3(0,   0,   0),},
			{DirectX::XMFLOAT3(-0.5f,-0.5f, 0.5f), DirectX::XMFLOAT3(0,   0,   0.5),},
			{DirectX::XMFLOAT3(-0.5f, 0.5f,-0.5f), DirectX::XMFLOAT3(0,   0.5,   0),},
			{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), DirectX::XMFLOAT3(0,   0.5,   0.5),},
			{DirectX::XMFLOAT3(0.5f,-0.5f,-0.5f), DirectX::XMFLOAT3(0.5,   0,   0),},
			{DirectX::XMFLOAT3(0.5f,-0.5f, 0.5f), DirectX::XMFLOAT3(0.5,   0,   0.5),},
			{DirectX::XMFLOAT3(0.5f, 0.5f,-0.5f), DirectX::XMFLOAT3(0.5,   0.5,   0),},
			{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), DirectX::XMFLOAT3(0.5,   0.5,   0.5),}
	};*/

	/*	Cube
	VertexPositionColor ver1 = { XMFLOAT3(xOff + -0.5f, yOff + -0.5f, -0.5f),	XMFLOAT3(0.0f, 0.0f, 0.0f) };
	VertexPositionColor ver2 = { XMFLOAT3(xOff + -0.5f, yOff + -0.5f,  0.5f),	XMFLOAT3(0.0f, 0.0f, 1.0f) };
	VertexPositionColor ver3 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f, -0.5f),	XMFLOAT3(0.0f, 1.0f, 0.0f) };
	VertexPositionColor ver4 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f,  0.5f),	XMFLOAT3(0.0f, 1.0f, 1.0f) };
	VertexPositionColor ver5 = { XMFLOAT3(xOff + 0.5f,	yOff + -0.5f, -0.5f),	XMFLOAT3(1.0f, 0.0f, 0.0f) };
	VertexPositionColor ver6 = { XMFLOAT3(xOff + 0.5f,	yOff + -0.5f, 0.5f),	XMFLOAT3(1.0f, 0.0f, 1.0f) };
	VertexPositionColor ver7 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, -0.5f),	XMFLOAT3(1.0f, 1.0f, 0.0f) };
	VertexPositionColor ver8 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, 0.5f),		XMFLOAT3(1.0f, 1.0f, 1.0f) };
	*/

	/* Half Triangle?
	VertexPositionColor ver1 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f, -0.5f),	XMFLOAT3(0.0f, 1.0f, 0.0f) };
	VertexPositionColor ver2 = { XMFLOAT3(xOff + -0.5f, yOff + -0.5f,  0.5f),	XMFLOAT3(0.0f, 0.0f, 1.0f) };
	VertexPositionColor ver3 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f, -0.5f),	XMFLOAT3(0.0f, 1.0f, 0.0f) };
	VertexPositionColor ver4 = { XMFLOAT3(xOff + -0.5f, yOff + 0.5f,  0.5f),	XMFLOAT3(0.0f, 1.0f, 1.0f) };
	VertexPositionColor ver5 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, -0.5f),	XMFLOAT3(1.0f, 1.0f, 0.0f) };
	VertexPositionColor ver6 = { XMFLOAT3(xOff + 0.5f,	yOff + -0.5f, 0.5f),	XMFLOAT3(1.0f, 0.0f, 1.0f) };
	VertexPositionColor ver7 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, -0.5f),	XMFLOAT3(1.0f, 1.0f, 0.0f) };
	VertexPositionColor ver8 = { XMFLOAT3(xOff + 0.5f,	yOff + 0.5f, 0.5f),		XMFLOAT3(1.0f, 1.0f, 1.0f) };
	*/

	// Create vertex buffer:

	CD3D11_BUFFER_DESC vDesc(
		sizeof(CubeVertices),
		D3D11_BIND_VERTEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
	vData.pSysMem = CubeVertices;
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(
		&vDesc,
		&vData,
		&m_vertexBuffer
	);

	// Create index buffer:
	unsigned short CubeIndices[] =
	{
		0,2,1, // -x
		1,2,3,

		4,5,6, // +x
		5,7,6,

		0,1,5, // -y
		0,5,4,

		2,6,7, // +y
		2,7,3,

		0,4,6, // -z
		0,6,2,

		1,3,7, // +z
		1,7,5,
	};

	m_indexCount = ARRAYSIZE(CubeIndices);

	CD3D11_BUFFER_DESC iDesc(
		sizeof(CubeIndices),
		D3D11_BIND_INDEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA iData;
	ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
	iData.pSysMem = CubeIndices;
	iData.SysMemPitch = 0;
	iData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(
		&iDesc,
		&iData,
		&m_indexBuffer
	);

	return hr;
}

void GameRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

void GameRenderer::ModifyDegreesPerSecond(float amount, int modType)
{
	//LIMIT=360
	//if the degrees per second modulus the LIMIT is less than
	//	
	if (fmod(m_degreesPerSecond, 360) < m_degreesPerSecond)
	{
		m_degreesPerSecond = (float)fmod(m_degreesPerSecond, 360.0f);
	}

	switch (modType)
	{
	default:
	case 0:
		m_degreesPerSecond += amount;
		break;
	case 1:
		m_degreesPerSecond -= amount;
		break;
	}
}
