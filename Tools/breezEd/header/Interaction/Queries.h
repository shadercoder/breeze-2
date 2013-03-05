#ifndef QUERIES_H
#define QUERIES_H

#include "breezEd.h"

#include <beEntitySystem/beEntities.h>

#include <beScene/bePerspective.h>
#include <beScene/beRenderer.h>
#include <beScene/beRenderingController.h>

class SceneDocument;

/// Retrieve the object ID under the given cursor position.
uint4 objectIDUnderCursor(beScene::Renderer &renderer, beScene::RenderingController &scene, const QPointF &relativePos, const beScene::PerspectiveDesc &perspective);

/// Retrieve the entity under the given cursor position.
beEntitySystem::Entity* entityUnderCursor(SceneDocument &document, const QPointF &relativePos, const beScene::PerspectiveDesc &perspective);

#endif
