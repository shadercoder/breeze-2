#ifndef QUERIES_H
#define QUERIES_H

#include "breezEd.h"

#include <beEntitySystem/beEntity.h>

#include <beScene/bePerspective.h>
#include <beScene/beRenderer.h>
#include <beScene/beSceneController.h>

class SceneDocument;

/// Retrieve the object ID under the given cursor position.
uint4 objectIDUnderCursor(beScene::Renderer &renderer, beScene::SceneController &scene, const QPointF &relativePos, const beScene::PerspectiveDesc &perspective);

/// Retrieve the entity under the given cursor position.
beEntitySystem::Entity* entityUnderCursor(SceneDocument &document, const QPointF &relativePos, const beScene::PerspectiveDesc &perspective);

#endif
