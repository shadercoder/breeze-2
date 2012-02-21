#include "stdafx.h"
#include "DeviceManager.h"

#include <QtCore/QSettings>

#include <beGraphics/beAdapters.h>

namespace
{

/// Creates a device from the given settings.
lean::resource_ptr<beGraphics::Device, true> createGraphicsDevice(void *pWindowHandle, QSettings &settings, bool *pVSync)
{
	uint4 adapterID = settings.value("graphicsDevice/adapterID", 0).toUInt();

	if (pVSync)
		*pVSync = settings.value("graphicsDevice/vSync", true).toBool();

	return beGraphics::GetGraphics()->CreateDevice(beGraphics::DeviceDesc(pWindowHandle), nullptr, adapterID);
}

} // namespace

// Constructor.
DeviceManager::DeviceManager(void *pWindowHandle, QSettings &settings)
	: m_bVSync(true),
	m_pGraphicsDevice( createGraphicsDevice(pWindowHandle, settings, &m_bVSync) ),
	m_pGraphicsResources( beScene::CreateResourceManager(m_pGraphicsDevice, "EffectCache", "Effects", "Textures", "Materials", "Meshes") )
{
}

// Destructor.
DeviceManager::~DeviceManager()
{
}
