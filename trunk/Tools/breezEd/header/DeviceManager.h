#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "breezEd.h"
#include <lean/tags/noncopyable.h>
#include <lean/smart/resource_ptr.h>

#include <beGraphics/beDevice.h>
#include <beScene/beResourceManager.h>
#include <bePhysics/beDevice.h>
#include <bePhysics/beResourceManager.h>

class QSettings;

class DeviceManager : public lean::noncopyable
{
private:
	// Graphics
	bool m_bVSync;
	lean::resource_ptr<beGraphics::Device> m_pGraphicsDevice;
	lean::resource_ptr<beScene::ResourceManager> m_pGraphicsResources;

	lean::resource_ptr<bePhysics::Device> m_pPhysicsDevice;
	lean::resource_ptr<bePhysics::ResourceManager> m_pPhysicsResources;

public:
	/// Constructor.
	DeviceManager(void *pWindowHandle, QSettings &settings);
	/// Destructor.
	~DeviceManager();

	/// Gets the graphics device.
	LEAN_INLINE beGraphics::Device* graphicsDevice() { return m_pGraphicsDevice; }
	/// Gets the graphics device.
	LEAN_INLINE const beGraphics::Device* graphicsDevice() const { return m_pGraphicsDevice; }
	
	/// Gets the graphics resource manager.
	LEAN_INLINE beScene::ResourceManager* graphicsResources() { return m_pGraphicsResources; }
	/// Gets the graphics resource manager.
	LEAN_INLINE const beScene::ResourceManager* graphicsResources() const { return m_pGraphicsResources; }

	/// Gets the physics device.
	LEAN_INLINE bePhysics::Device* physicsDevice() { return m_pPhysicsDevice; }
	/// Gets the physics device.
	LEAN_INLINE const bePhysics::Device* physicsDevice() const { return m_pPhysicsDevice; }

	/// Gets the physics resource manager.
	LEAN_INLINE bePhysics::ResourceManager* physicsResources() { return m_pPhysicsResources; }
	/// Gets the physics resource manager.
	LEAN_INLINE const bePhysics::ResourceManager* physicsResources() const { return m_pPhysicsResources; }
};

#endif
