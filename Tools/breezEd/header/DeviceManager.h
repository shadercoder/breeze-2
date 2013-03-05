#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "breezEd.h"
#include <lean/tags/noncopyable.h>
#include <lean/smart/resource_ptr.h>

#include <beGraphics/beDevice.h>
#include <bePhysics/beDevice.h>

#include <QtGui/QWindowDefs.h>

class QSettings;

class DeviceManager : public lean::noncopyable
{
private:
	// Graphics
	bool m_bVSync;
	lean::resource_ptr<beGraphics::Device> m_pGraphicsDevice;
	lean::resource_ptr<bePhysics::Device> m_pPhysicsDevice;

public:
	/// Constructor.
	DeviceManager(WId windowHandle, QSettings &settings);
	/// Destructor.
	~DeviceManager();

	/// Gets the graphics device.
	LEAN_INLINE beGraphics::Device* graphicsDevice() { return m_pGraphicsDevice; }
	/// Gets the graphics device.
	LEAN_INLINE const beGraphics::Device* graphicsDevice() const { return m_pGraphicsDevice; }
	
	/// Gets the physics device.
	LEAN_INLINE bePhysics::Device* physicsDevice() { return m_pPhysicsDevice; }
	/// Gets the physics device.
	LEAN_INLINE const bePhysics::Device* physicsDevice() const { return m_pPhysicsDevice; }
};

#endif
