
#pragma once

#include "core/singleton.h"
#include "renderTexture2D.h"

#include <d3d9.h>

_NAMESPACE_BEGIN

class GearPlatformUtil : public Singleton<GearPlatformUtil>
{
public:

	_DECLARE_SINGLETON(GearPlatformUtil);

public:

	explicit							GearPlatformUtil();

	virtual								~GearPlatformUtil();

	// System
	virtual void						correctCurrentDir(void);

	virtual void						popPathSpec(char *path);

	virtual bool						openWindow(uint32& width, uint32& height,
											const char* title,bool fullscreen);

	virtual void						update();

	virtual bool						closeWindow();
	
	virtual bool						hasFocus() const;
	
	virtual void						setFocus(bool b);
	
	virtual bool						isOpen();
	
	virtual uint64						getWindowHandle();
	
	virtual void						setWindowSize(uint32 width, uint32 height);
	
	virtual void						getWindowSize(uint32& width, uint32& height);
	
	virtual void						getTitle(char *title, uint32 maxLength) const;
	
	virtual void						setTitle(const char *title);
	
	virtual void						recenterCursor(scalar& deltaMouseX, scalar& deltaMouseY);
	
	virtual void*						initializeD3D9();
	
	virtual bool						isD3D9ok();
	
	virtual uint32						initializeD3D9Display(void * presentParameters, 
											char* m_deviceName, uint32& width, 
											uint32& height,void * m_d3dDevice_out);
	
	virtual uint32						D3D9Present();
	
	virtual void						D3D9BlockUntilNotBusy(void * resource);
	
	virtual void						D3D9DeviceBlockUntilIdle();
	
	virtual uint64						getD3D9TextureFormat(RenderTexture2D::Format format);
	
	virtual void						postRenderRelease();
	
	virtual void						preRenderSetup();
	
	virtual void						postRenderSetup();

protected:

	IDirect3D9*							m_d3d;
	
	IDirect3DDevice9*					m_d3dDevice;
	
	HWND								m_hwnd;
	
	HMODULE		                        m_library;
	
	bool								m_isHandlingMessages;
	
	bool								m_destroyWindow;
	
	bool								m_hasFocus;
};

_NAMESPACE_END