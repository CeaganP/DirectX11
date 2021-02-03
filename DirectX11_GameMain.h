#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\GameRenderer.h"
#include "Content\SampleFpsTextRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace DirectX11_Game
{
	class DirectX11_GameMain : public DX::IDeviceNotify
	{
	public:
		DirectX11_GameMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~DirectX11_GameMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();
		void SetPlayerInputs(int input);
		void LoadResources();
		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		//Adding a texture
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<GameRenderer> m_gameRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		// Rendering loop timer.
		DX::StepTimer m_timer;
		int m_displayVal[3];
		int m_userInput;
		float m_xOff;
		float m_yOff;
		bool m_drawBackdrop;
		std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
		std::unique_ptr<DirectX::CommonStates> m_states;
		DirectX::SimpleMath::Vector2 m_screenPos;
		DirectX::SimpleMath::Vector2 m_origin;
	};
}