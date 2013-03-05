#include "stdafx.h"
#include "Editor.h"

#include "Windows/MainWindow.h"
#include <QtWidgets/QApplication>

#include <beLauncher/beInitEngine.h>
#include <beCore/beFileSystem.h>

#include <beAssets/beAssets.h>

int main(int argc, char *argv[])
{
	beAssets::Link();

	QApplication a(argc, argv);

#ifdef QT_STYLE_WINDOWSVISTA
	QApplication::setStyle(new QWindowsVistaStyle());
#endif

	beLauncher::InitializeLog("Logs/breezEd.log");
	beLauncher::InitializeFilesystem();

	// Default directory configuration
	if (!beCore::FileSystem::Get().HasLocation("Effects"))
		beCore::FileSystem::Get().AddPath("Effects", "Data/Effects/2.0");
	if (!beCore::FileSystem::Get().HasLocation("EffectCache"))
		beCore::FileSystem::Get().AddPath("EffectCache", "Data/Effects/Cache/2.0");
	if (!beCore::FileSystem::Get().HasLocation("Textures"))
		beCore::FileSystem::Get().AddPath("Textures", "Data/Textures");
	if (!beCore::FileSystem::Get().HasLocation("Materials"))
		beCore::FileSystem::Get().AddPath("Materials", "Data/Materials");
	if (!beCore::FileSystem::Get().HasLocation("Meshes"))
		beCore::FileSystem::Get().AddPath("Meshes", "Data/Meshes");
	if (!beCore::FileSystem::Get().HasLocation("PhysicsMaterials"))
		beCore::FileSystem::Get().AddPath("PhysicsMaterials", "Data/Materials");
	if (!beCore::FileSystem::Get().HasLocation("PhysicsShapes"))
		beCore::FileSystem::Get().AddPath("PhysicsShapes", "Data/Meshes");
	if (!beCore::FileSystem::Get().HasLocation("Maps"))
		beCore::FileSystem::Get().AddPath("Maps", "Data/Maps");

	Editor e;
	QObject::connect(&a, &QApplication::focusChanged, e.mainWindow(), &MainWindow::focusChanged);
	e.mainWindow()->show();
	
	return a.exec();
}
