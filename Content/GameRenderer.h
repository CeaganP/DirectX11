#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;
using namespace DirectX;

namespace DirectX11_Game
{
	// This sample renderer instantiates a basic rendering pipeline.
	class GameRenderer
	{
	public:
		GameRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources, float xOff, float yOff);
		void CreateDeviceDependentResources(float xOff, float yOff);
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking();
		void TrackingUpdate(float positionX, ModelViewProjectionConstantBuffer* modelBuffer);
		void StopTracking();
		bool IsTracking() { return m_tracking; }
		void ModifyDegreesPerSecond(float amount, int modType);
		HRESULT CreateCube();

		//system resources for the cube resources
		int m_dataBufferSize = 5;
		ModelViewProjectionConstantBuffer m_dataBuffers[5];
	private:
		void Rotate(float radians, ModelViewProjectionConstantBuffer* modelBuffer);
		void Scale(float scaleAmt, ModelViewProjectionConstantBuffer* modelBuffer);
		void ScaleRotate(float radians, float scaleAmt, ModelViewProjectionConstantBuffer* modelBuffer);
		void Translate(XMFLOAT3 axis, ModelViewProjectionConstantBuffer* modelBuffer);
		void TranslateScaleRotate(float radians, float scaleAmt, XMFLOAT3 axis, ModelViewProjectionConstantBuffer* modelBuffer);
		void DrawObject(ModelViewProjectionConstantBuffer& modelBuffer);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		ComPtr<ID3D11InputLayout>	m_inputLayout;
		ComPtr<ID3D11Buffer>		m_vertexBuffer;
		ComPtr<ID3D11Buffer>		m_indexBuffer;
		ComPtr<ID3D11VertexShader>	m_vertexShader;
		ComPtr<ID3D11PixelShader>	m_pixelShader;
		ComPtr<ID3D11Buffer>		m_constantBuffer;
		
		//System resources for cube geometry
		uint32	m_indexCount;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
	};
}

