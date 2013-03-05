#include "stdafx.h"
#include "DeviceManager.h"

#include <QtCore/QSettings>

#include <beGraphics/beAdapters.h>

namespace
{

/// Creates a device from the given settings.
lean::resource_ptr<beGraphics::Device, true> createGraphicsDevice(WId windowHandle, QSettings &settings, bool *pVSync)
{
	uint4 adapterID = settings.value("graphicsDevice/adapterID", 0).toUInt();

	if (pVSync)
		*pVSync = settings.value("graphicsDevice/vSync", true).toBool();

	beGraphics::DeviceDesc desc((HWND) windowHandle);
//	desc.Debug = true;
	return beGraphics::GetGraphics()->CreateDevice(desc, nullptr, adapterID);
}

/// Creates a device from the given settings.
lean::resource_ptr<bePhysics::Device, true> createPhysicsDevice(QSettings &settings)
{
	float lengthScale = settings.value("physicsDevice/lengthScale", 1.0f).toFloat();
	float massScale = settings.value("physicsDevice/massScale", 1000.0f).toFloat();
	float speedScale = settings.value("physicsDevice/speedScale", 10.0f).toFloat();

	return bePhysics::CreateDevice(bePhysics::DeviceDesc(lengthScale, massScale, speedScale));
}

} // namespace

// Constructor.
DeviceManager::DeviceManager(WId windowHandle, QSettings &settings)
	: m_bVSync(true),
	m_pGraphicsDevice( createGraphicsDevice(windowHandle, settings, &m_bVSync) ),
	m_pPhysicsDevice( createPhysicsDevice(settings) )
{
}

// Destructor.
DeviceManager::~DeviceManager()
{
}
