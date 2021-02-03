#include "pch.h"
#include "DirectX11_GameMain.h"
#include "Common\DirectXHelper.h"
using namespace Microsoft::WRL;
using namespace DirectX;

using namespace DirectX11_Game;
using namespace DirectX::SimpleMath;

using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
DirectX11_GameMain::DirectX11_GameMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources),
	m_xOff(1),
	m_yOff(1),
	m_displayVal(),
	m_drawBackdrop(true)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);
	
	LoadResources();

	// TODO: Replace this with your app's content initialization.
	//m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources, m_xOff, m_yOff));
	m_gameRenderer = std::unique_ptr<GameRenderer>(new GameRenderer(m_deviceResources, m_xOff, m_yOff));
    m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	//assign the public variable these values
	m_fpsTextRenderer->SetPublicVariable(m_displayVal[0]);
	m_fpsTextRenderer->SetPublicVariable(m_displayVal[1]);
	m_fpsTextRenderer->SetPublicVariable(m_displayVal[2]);
	m_fpsTextRenderer->SetPublicVariable(m_userInput);

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	
}

void DirectX11_GameMain::LoadResources() 
{
	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto viewport = m_deviceResources->GetScreenViewport();
	// Load the CAT as a texture
	m_spriteBatch = std::make_unique<SpriteBatch>(context);
	m_states = std::make_unique<CommonStates>(device); //data for alpha

	ComPtr<ID3D11Texture2D> cat;

	DX::ThrowIfFailed(
		CreateWICTextureFromFile(device, L"cat.png", nullptr,
			m_texture.ReleaseAndGetAddressOf()));

	//docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-textures-how-to
	ComPtr<ID3D11Resource> resource;
	//get the resource first
	m_texture->GetResource(&resource);
	//then query the resource for the information
	DX::ThrowIfFailed(resource->QueryInterface(IID_ID3D11Texture2D, (void**)&cat));

	CD3D11_TEXTURE2D_DESC catDesc;
	cat->GetDesc(&catDesc);

	//offset the image by half the size of itself
	m_origin.x = float(catDesc.Width / 2);
	m_origin.y = float(catDesc.Height / 2);
	//display the image at the center of the screen
	m_screenPos.x = viewport.Width / 2.f;
	m_screenPos.y = viewport.Height / 2.f;

}

DirectX11_GameMain::~DirectX11_GameMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void DirectX11_GameMain::CreateWindowSizeDependentResources() 
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	//m_sceneRenderer->CreateWindowSizeDependentResources();
	m_gameRenderer->CreateWindowSizeDependentResources();
	
	//m_deviceResources->GetScreenViewport().Width, 
	//m_deviceResources->GetScreenViewport().Height);
	m_screenPos.x = m_deviceResources->GetScreenViewport().Width / 2.f;
	m_screenPos.y = m_deviceResources->GetScreenViewport().Height / 2.f;
}

// Updates the application state once per frame.
void DirectX11_GameMain::Update() 
{
	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
		//m_sceneRenderer->Update(m_timer);
		m_gameRenderer->Update(m_timer);
		
		m_fpsTextRenderer->Update(m_timer,
			m_deviceResources->GetScreenViewport().Width, 
			m_deviceResources->GetScreenViewport().Height);
	});
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool DirectX11_GameMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[2] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(2, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::DimGray);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	float time = float(m_timer.GetTotalSeconds());

	//CatBackdrop
	if (m_drawBackdrop) 
	{
		//draw the premultiplied states, add the alpha (transparency)
			//if no states are available no problem, don't display them
		if (m_states == nullptr)
			m_spriteBatch->Begin();
		else
			m_spriteBatch->Begin( SpriteSortMode_Deferred, m_states->NonPremultiplied() );
	
		//drawing a background image - https://github.com/microsoft/DirectXTK/wiki/Sprites-and-textures
		float r = cosf(time) * 2.f;
		float g = tanf(time) * 2.f;
		float b = sinf(time) * 2.f;
		XMVECTORF32 color = {{{ r, g, b, 1.f }}}; //Colors::White;
		//update the values attached to the LinkedInt, being displayed by the FPS renderer		
		m_displayVal[0] = sinf(time) * 10 + 10;
		m_displayVal[1] = cosf(time) * 10 + 10;
		m_displayVal[2] = tanf(time) * 10 + 10;
	
		//texture, position, sourceRect, color, rotation, origin, scaling
		m_spriteBatch->Draw(m_texture.Get(), m_screenPos, nullptr, color,
			cosf(time) * 4.f, m_origin, cosf(time) * 4.f, SpriteEffects::SpriteEffects_None, 0.f);
	
		m_spriteBatch->End();
	}

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	//m_sceneRenderer->Render();
	m_gameRenderer->Render();
	m_fpsTextRenderer->Render();

	return true;
}

// Notifies renderers that device resources need to be released.
void DirectX11_GameMain::OnDeviceLost()
{
	m_texture.Reset();
	m_spriteBatch.reset();
	m_states.reset();
	
	//m_sceneRenderer->ReleaseDeviceDependentResources();
	m_gameRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void DirectX11_GameMain::OnDeviceRestored()
{
	auto context = m_deviceResources->GetD3DDevice();
		
	LoadResources();

	//creates a single device resource for a single mesh
	//m_sceneRenderer->CreateDeviceDependentResources(0,0);
	m_gameRenderer->CreateDeviceDependentResources(0,0);
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void DirectX11_GameMain::SetPlayerInputs(int input) 
{
	m_userInput = input;
	//m_fpsTextRenderer->SetPublicVariable(input);
	//m_sceneRenderer->ModifyDegreesPerSecond(5, input);
	m_gameRenderer->ModifyDegreesPerSecond(5, input);

	//play audio track based off the input provided
}