/*
 * Collision detector file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionDetector.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


/*
 * Internal comparision functions
 */

bool cmpPickedObjects(SPickingContact &obj1, SPickingContact &obj2)
{
    return obj1.DistanceSq < obj2.DistanceSq;
}


/*
 * SObjectLocation structure
 */

void CollisionDetector::SObjectLocation::update(SceneNode* Object)
{
    Matrix  = Object->getTransformation(true);
    Matrix.getInverse(MatrixInv);
    
    PosMat  = Object->getPositionMatrix(true);
    RotMat  = Object->getRotationMatrix(true);
    SclMat  = Object->getScaleMatrix(true);
    
    Pos     = PosMat.getPosition();
    Rot     = RotMat.getRotation();
    Scl     = SclMat.getScale();
}


/*
 * CollisionDetector class
 */

CollisionDetector::CollisionDetector()
{
    isAutoSort_ = true;
}
CollisionDetector::~CollisionDetector()
{
    /* Delete each collision/ picking object */
    MemoryManager::deleteList(CollisionList_);
    MemoryManager::deleteList(PickObjectList_);
    MemoryManager::deleteList(CollObjectList_);
}

Collision* CollisionDetector::createCollision()
{
    Collision* NewColl = MemoryManager::createMemory<Collision>();
    CollisionList_.push_back(NewColl);
    return NewColl;
}

void CollisionDetector::deleteCollision(Collision* &CollisionObject)
{
    MemoryManager::removeElement(CollisionList_, CollisionObject, true);
}

SCollisionObject* CollisionDetector::addCollisionObject(
    SceneNode* Object, Collision* CollisionObject, f32 Radius, const dim::aabbox3df &BoundingBox)
{
    /* Check if there is already a combination of these collision-object */
    foreach (SCollisionObject* CollObj, CollObjectList_)
    {
        if (CollObj->CollisionHandle == CollisionObject && CollObj->Object == Object)
        {
            /* Update the configuration */
            CollObj->Radius       = Radius;
            CollObj->BoundingBox  = BoundingBox;
            CollObj->deleteTriangleData();
            return CollObj;
        }
    }
    
    /* Create a new collision-object */
    SCollisionObject* NewCollObj = MemoryManager::createMemory<SCollisionObject>();
    
    /* Configure the collision-object */
    NewCollObj->Object          = Object;
    NewCollObj->Radius          = Radius;
    NewCollObj->BoundingBox     = BoundingBox;
    NewCollObj->CollisionHandle = CollisionObject;
    NewCollObj->LastPosition    = Object->getPosition(true);
    
    /* Add the collision-object to both lists */
    CollisionObject->ObjectList_.push_back(NewCollObj);
    CollObjectList_.push_back(NewCollObj);
    
    return NewCollObj;
}

SCollisionObject* CollisionDetector::addCollisionMesh(Mesh* Mesh, Collision* CollisionObject, const video::EFaceTypes FaceType)
{
    /* Check if there is already a combination of these collision-object */
    foreach (SCollisionObject* CollObj, CollObjectList_)
    {
        if (CollObj->CollisionHandle == CollisionObject && CollObj->Object == Mesh)
        {
            /* Recreate the triangle data */
            CollObj->createTriangleData();
            CollObj->FaceType         = FaceType;
            CollObj->CollisionHandle  = CollisionObject;
            return CollObj;
        }
    }
    
    /* Create a new collision-object */
    SCollisionObject* NewCollObj = MemoryManager::createMemory<SCollisionObject>();
    
    /* Configure the collision-object */
    NewCollObj->Object          = Mesh;
    NewCollObj->Mesh            = Mesh;
    NewCollObj->FaceType        = FaceType;
    NewCollObj->CollisionHandle = CollisionObject;
    NewCollObj->LastPosition    = Mesh->getPosition(true);
    
    NewCollObj->createTriangleData();
    
    /* Add the collision-object to both lists */
    CollisionObject->ObjectList_.push_back(NewCollObj);
    CollObjectList_.push_back(NewCollObj);
    
    return NewCollObj;
}

SCollisionObject* CollisionDetector::getCollisionObject(SceneNode* Object)
{
    for (std::list<SCollisionObject*>::iterator it = CollObjectList_.begin(), itc; it != CollObjectList_.end(); ++it)
    {
        if ((*it)->Object == Object)
            return *it;
    }
    return 0;
}

void CollisionDetector::removeCollisionObject(SCollisionObject* Object)
{
    for (std::list<SCollisionObject*>::iterator it = CollObjectList_.begin(), itc; it != CollObjectList_.end(); ++it)
    {
        if (*it == Object)
        {
            /* Remove the collision-object from the collision's list */
            for (itc = (*it)->CollisionHandle->ObjectList_.begin(); itc != (*it)->CollisionHandle->ObjectList_.end(); ++itc)
            {
                if (*itc == *it)
                {
                    (*it)->CollisionHandle->ObjectList_.erase(itc);
                    break;
                }
            }
            
            /* Remove the collision-object from the main list */
            MemoryManager::deleteMemory(Object);
            CollObjectList_.erase(it);
            
            break;
        }
    }
}


SPickingObject* CollisionDetector::addPickingObject(
    SceneNode* Object, const EPickingTypes Type, f32 Radius, const dim::aabbox3df &BoundingBox)
{
    /* Check if there is already this picking object */
    foreach (SPickingObject* PickObj, PickObjectList_)
    {
        if (PickObj->Object == Object)
        {
            /* Update the configuration */
            PickObj->Type         = Type;
            PickObj->Radius       = Radius;
            PickObj->BoundingBox  = BoundingBox;
            PickObj->deleteTriangleData();
            
            return PickObj;
        }
    }
    
    /* Create a new picking-object */
    SPickingObject* NewPickObject = MemoryManager::createMemory<SPickingObject>();
    
    /* Configure the picking-object */
    NewPickObject->Type         = Type;
    NewPickObject->Object       = Object;
    NewPickObject->Radius       = Radius;
    NewPickObject->BoundingBox  = BoundingBox;
    
    /* Add the collision-picking to list */
    PickObjectList_.push_back(NewPickObject);
    
    return NewPickObject;
}

SPickingObject* CollisionDetector::getPickingObject(SceneNode* Object)
{
    foreach (SPickingObject* PickObj, PickObjectList_)
    {
        if (PickObj->Object == Object)
            return PickObj;
    }
    return 0;
}

SPickingObject* CollisionDetector::addPickingMesh(Mesh* Mesh)
{
    /* Check if there is already this picking object */
    foreach (SPickingObject* PickObj, PickObjectList_)
    {
        if (PickObj->Object == Mesh)
        {
            /* Update the configuration */
            PickObj->Type = PICKMODE_POLYGON;
            PickObj->createTriangleData();
            
            return PickObj;
        }
    }
    
    /* Create a new picking-object */
    SPickingObject* NewPickObject = MemoryManager::createMemory<SPickingObject>();
    
    /* Configure the picking-object */
    NewPickObject->Type     = PICKMODE_POLYGON;
    NewPickObject->Object   = Mesh;
    NewPickObject->Mesh     = Mesh;
    
    Mesh->PickRef_ = NewPickObject;
    
    NewPickObject->createTriangleData();
    
    /* Add the collision-picking to list */
    PickObjectList_.push_back(NewPickObject);
    
    return NewPickObject;
}

void CollisionDetector::removePickingObject(SPickingObject* Object)
{
    MemoryManager::removeElement(PickObjectList_, Object, true);
}


/*
 * Updates the scene
 * -> all collision detection functions are processed here
 */

void CollisionDetector::updateScene()
{
    
    /* Temporary variables */
    
    std::list<SCollisionObject*>::iterator it, itDest;
    std::list<Collision::SCollisionMaterial>::iterator itMat;
    
    Collision* CurColl = 0, * DestColl = 0;
    
    /* Loop for each collision-object */
    
    for (it = CollObjectList_.begin(); it != CollObjectList_.end(); ++it)
    {
        
        (*it)->Object->CollisionContactList_.clear();
        
        if ( !(*it)->Object->getVisible() || ( (*it)->isAutoFreeze && (*it)->LastPosition.equal((*it)->Object->getPosition(true)) ) )
            continue;
        
        CurColl = (*it)->CollisionHandle;
        CurLocation_.update((*it)->Object);
        
        /* Loop for each collision-material (destination combinations) */
        
        for (itMat = CurColl->CollMaterialList_.begin(); itMat != CurColl->CollMaterialList_.end(); ++itMat)
        {
            
            DestColl = itMat->DestCollision;
            
            /* Select the combination type */
            switch (itMat->Type)
            {
                case COLLISION_SPHERE_TO_SPHERE:
                {
                    for (itDest = DestColl->ObjectList_.begin(); itDest != DestColl->ObjectList_.end(); ++itDest)
                    {
                        if (*it != *itDest && (*itDest)->Object->getVisible() && (*it)->Object != (*itDest)->Object)
                        {
                            DestLocation_.update((*itDest)->Object);
                            checkCollisionSphereToSphere(*it, *itDest);
                        }
                    }
                }
                break;
                
                case COLLISION_SPHERE_TO_BOX:
                {
                    for (itDest = DestColl->ObjectList_.begin(); itDest != DestColl->ObjectList_.end(); ++itDest)
                    {
                        if (*it != *itDest && (*itDest)->Object->getVisible() && (*it)->Object != (*itDest)->Object)
                        {
                            DestLocation_.update((*itDest)->Object);
                            checkCollisionSphereToBox(*it, *itDest);
                        }
                    }
                }
                break;
                
                case COLLISION_SPHERE_TO_POLYGON:
                {
                    for (itDest = DestColl->ObjectList_.begin(); itDest != DestColl->ObjectList_.end(); ++itDest)
                    {
                        if (*it != *itDest && (*itDest)->Object->getVisible() && (*it)->Object != (*itDest)->Object && (*itDest)->Mesh)
                        {
                            DestLocation_.update((*itDest)->Object);
                            if ((*itDest)->Mesh->getOctTreeRoot())
                                checkCollisionSphereToPolygonTree(*it, *itDest);
                            else
                                checkCollisionSphereToPolygon(*it, *itDest);
                        }
                    }
                }
                break;
                
                case COLLISION_BOX_TO_POLYGON:
                {
                    for (itDest = DestColl->ObjectList_.begin(); itDest != DestColl->ObjectList_.end(); ++itDest)
                    {
                        if (*it != *itDest && (*itDest)->Object->getVisible() && (*it)->Object != (*itDest)->Object)
                        {
                            DestLocation_.update((*itDest)->Object);
                            //if ((*itDest)->Mesh->getOctTreeRoot())
                            //    checkCollisionBoxToPolygonTree(*it, *itDest);
                            //else
                                checkCollisionBoxToPolygon(*it, *itDest);
                        }
                    }
                }
                break;
                
                default:
                    break;
            } // /switch
            
        } // next collision-material
        
        /* Get the last position & make collision contact list unique */
        (*it)->LastPosition = (*it)->Object->getPosition(true);
        
        (*it)->Object->CollisionContactList_.unique();
        
    } // next collision-object
    
}


/*
 * Picking intersection functions
 */

std::list<SPickingContact> CollisionDetector::pickIntersection(
    const dim::line3df &Line, const video::EFaceTypes FaceType)
{
    /* Clear the contact list */
    PickContactList_.clear();
    
    /* Loop for each picking object */
    
    PickingStart_ = Line.Start;
    
    switch (FaceType)
    {
        case video::FACE_FRONT:
            processPicking(dim::line3df(Line.Start, Line.End)); break;
            
        case video::FACE_BACK:
            processPicking(dim::line3df(Line.End, Line.Start)); break;
        
        case video::FACE_BOTH:
            processPicking(dim::line3df(Line.Start, Line.End));
            processPicking(dim::line3df(Line.End, Line.Start));
            break;
    }
    
    /* Sort intersection list */
    if (isAutoSort_)
    {
        PickContactList_.unique();
        PickContactList_.sort(cmpPickedObjects);
    }
    
    /* Return the intersection contact list */
    return PickContactList_;
}

bool CollisionDetector::pickIntersection(const dim::line3df &Line, SPickingContact &Contact)
{
    pickIntersection(Line);
    
    if (PickContactList_.size())
    {
        Contact = *PickContactList_.begin();
        return true;
    }
    
    return false;
}

bool CollisionDetector::pickIntersection(const dim::vector3df &PosA, const dim::vector3df &PosB)
{
    /* Picking line */
    dim::line3df Line(PosA, PosB);
    
    /* Make intersection tests with each object */
    foreach (SPickingObject* PickObj, PickObjectList_)
    {
        if (!PickObj->Object->getVisible() && !PickObj->IgnoreVisibility)
            continue;
        
        switch (PickObj->Type)
        {
            case PICKMODE_SPHERE:
            {
                if (checkInviewLineSphere(PickObj, Line))
                    return false;
            }
            break;
            
            case PICKMODE_BOX:
            {
                if (checkInviewLineBox(PickObj, Line))
                    return false;
            }
            break;
            
            case PICKMODE_POLYGON:
            {
                if (PickObj->Mesh->getOctTreeRoot())
                {
                    if (checkInviewLinePolygonTree(PickObj, Line))
                        return false;
                }
                else
                {
                    if (checkInviewLinePolygon(PickObj, Line))
                        return false;
                }
            }
            break;
            
            default:
                break;
        }
    }
    
    /* The joints are inview */
    return true;
}

void CollisionDetector::clearScene(bool isDeleteCollision, bool isDeletePicking)
{
    if (isDeleteCollision)
    {
        MemoryManager::deleteList(CollisionList_);
        MemoryManager::deleteList(CollObjectList_);
    }
    if (isDeletePicking)
        MemoryManager::deleteList(PickObjectList_);
}


/*
 * Mesh-mesh intersection
 */

bool CollisionDetector::meshIntersection(Mesh* MeshA, Mesh* MeshB)
{
    if (!MeshA || !MeshB)
        return false;
    
    /*OcTreeNode* TreeA = MeshA->getOctTreeRoot();
    OcTreeNode* TreeB = MeshB->getOctTreeRoot();
    
    if (TreeA && TreeB)
        return checkIntersectionMeshMesh(MeshA, MeshB, TreeA, TreeB);
    else if (TreeA)
        return checkIntersectionMeshMesh(MeshA, MeshB, TreeA);
    else if (TreeB)
        return checkIntersectionMeshMesh(MeshB, MeshA, TreeB);
    */
    return checkIntersectionMeshMesh(MeshA, MeshB);
}


/*
 * ========== Private: ==========
 */

SCollisionContactData CollisionDetector::getPolygonCollisionContact(
    SCollisionObject* CurObject, SCollisionObject* DestObject,
    s32 TriangleListIndex, const dim::triangle3df &Triangle, const dim::vector3df &CollisionPoint)
{
    SCollisionContactData Contact;
    {
        Contact.Object          = DestObject->Object;
        Contact.Mesh            = DestObject->Mesh;
        
        Contact.Triangle        = Triangle;
        Contact.Point           = CollisionPoint;
        Contact.Normal          = Triangle.getNormal();
        
        Contact.SurfaceIndex    = DestObject->TriangleList[TriangleListIndex].Surface;
        Contact.TriangleIndex   = DestObject->TriangleList[TriangleListIndex].Index;
        
        Contact.Type            = COLLISION_SPHERE_TO_POLYGON;
        Contact.CollisionHandle = DestObject->CollisionHandle;
    }
    return Contact;
}

SCollisionContactData CollisionDetector::getPolygonCollisionContact(
    SCollisionObject* CurObject, SCollisionObject* DestObject,
    std::list<STreeNodeTriangleData>::iterator TriangleIterator,
    const dim::triangle3df &Triangle, const dim::vector3df &CollisionPoint)
{
    SCollisionContactData Contact;
    {
        Contact.Object          = DestObject->Object;
        Contact.Mesh            = DestObject->Mesh;
        
        Contact.Triangle        = Triangle;
        Contact.Point           = CollisionPoint;
        Contact.Normal          = Triangle.getNormal();
        
        Contact.SurfaceIndex    = TriangleIterator->Surface;
        Contact.TriangleIndex   = TriangleIterator->Index;
        
        Contact.Type            = COLLISION_SPHERE_TO_POLYGON;
        Contact.CollisionHandle = DestObject->CollisionHandle;
    }
    return Contact;
}


SPickingContact CollisionDetector::getPolygonPickingContact(
    SPickingObject* Object, s32 TriangleListIndex,
    const dim::triangle3df &Triangle, const dim::vector3df &Intersection)
{
    SPickingContact Contact;
    {
        Contact.Object          = Object->Object;
        Contact.Mesh            = Object->Mesh;
        
        Contact.Triangle        = Triangle;
        Contact.Point           = Intersection;
        Contact.Normal          = Triangle.getNormal();
        
        Contact.SurfaceIndex    = Object->TriangleList[TriangleListIndex].Surface;
        Contact.TriangleIndex   = Object->TriangleList[TriangleListIndex].Index;
        
        Contact.Type            = PICKMODE_POLYGON;
        Contact.DistanceSq      = math::getDistanceSq(PickingStart_, Intersection);
    }
    return Contact;
}

SPickingContact CollisionDetector::getPolygonPickingContact(
    SPickingObject* Object, std::list<STreeNodeTriangleData>::iterator TriangleIterator,
    const dim::triangle3df &Triangle, const dim::vector3df &Intersection)
{
    SPickingContact Contact;
    {
        Contact.Object          = Object->Object;
        Contact.Mesh            = Object->Mesh;
        
        Contact.Triangle        = Triangle;
        Contact.Point           = Intersection;
        Contact.Normal          = Triangle.getNormal();
        
        Contact.SurfaceIndex    = TriangleIterator->Surface;
        Contact.TriangleIndex   = TriangleIterator->Index;
        
        Contact.Type            = PICKMODE_POLYGON;
        Contact.DistanceSq      = math::getDistanceSq(PickingStart_, Intersection);
    }
    return Contact;
}


void CollisionDetector::processContact(
    SCollisionObject* CurObject, const SCollisionContactData &Contact, const dim::vector3df &Position)
{
    /* Add the contact to the collision-contact list */
    CurObject->Object->CollisionContactList_.push_back(Contact);
    
    if (CurObject->isCollidable)
    {
        /* Set the new position */
        CurObject->Object->setPosition(Position, true);
        
        /* Update the location of the primary object */
        CurLocation_.update(CurObject->Object);
    }
}

bool CollisionDetector::checkFaceSide(
    video::EFaceTypes FaceType, const dim::triangle3df &Triangle, const dim::vector3df &Position)
{
    return
        FaceType == video::FACE_BOTH ||
        ( ( FaceType == video::FACE_FRONT && dim::plane3df(Triangle).isPointFrontSide(Position) ) ||
          ( FaceType == video::FACE_BACK ) );
}

void CollisionDetector::processPicking(const dim::line3df &Line)
{
    foreach (SPickingObject* PickObj, PickObjectList_)
    {
        if (!PickObj->Object->getVisible() && !PickObj->IgnoreVisibility)
            continue;
        
        switch (PickObj->Type)
        {
            case PICKMODE_SPHERE:
                checkIntersectionLineSphere(PickObj, Line);
                break;
                
            case PICKMODE_BOX:
                checkIntersectionLineBox(PickObj, Line);
                break;
                
            case PICKMODE_POLYGON:
                if (PickObj->Mesh->getOctTreeRoot())
                    checkIntersectionLinePolygonTree(PickObj, Line);
                else
                    checkIntersectionLinePolygon(PickObj, Line);
                break;
                
            default:
                break;
        }
    } // next picking object
}


/*
 * Sphere-to-sphere collision
 */

void CollisionDetector::checkCollisionSphereToSphere(SCollisionObject* CurObject, SCollisionObject* DestObject)
{
    
    /* Compute the square distance between both objects */
    dim::vector3df Distance = DestLocation_.Pos - CurLocation_.Pos;
    
    /* Compare the square distance with the square raiduses */
    if (Distance.getLengthSq() < math::Pow2(CurObject->Radius + DestObject->Radius))
    {
        Distance.normalize();
        
        /* Fill the contact data */
        SCollisionContactData Contact;
        {
            Contact.Object          = DestObject->Object;//CurObject->Object;
            Contact.Point           = DestLocation_.Pos - Distance * CurObject->Radius;
            Contact.Normal          = Distance;
            Contact.Type            = COLLISION_SPHERE_TO_SPHERE;
            Contact.CollisionHandle = DestObject->CollisionHandle;
        }
        
        /* Add new contact */
        processContact(
            CurObject, Contact, DestLocation_.Pos - Distance * ( CurObject->Radius + DestObject->Radius )
        );
    }
    
}


/*
 * Sphere-to-box collision
 */

void CollisionDetector::checkCollisionSphereToBox(SCollisionObject* CurObject, SCollisionObject* DestObject)
{
    
    /* First settings */
    
    dim::aabbox3df* Box = &DestObject->BoundingBox;
    
    /* Get the inverse transformation of the destination object */
    
    CollMatrix_ = (DestLocation_.PosMat * DestLocation_.RotMat * DestLocation_.SclMat).getInverse();
    
    dim::vector3df Pos = CollMatrix_ * CurLocation_.Pos;
    
    /* Check if the sphere is inside the box-area */
    
    if (Pos.X > Box->Min.X / DestLocation_.Scl.X - CurObject->Radius && 
        Pos.Y > Box->Min.Y / DestLocation_.Scl.Y - CurObject->Radius && 
        Pos.Z > Box->Min.Z / DestLocation_.Scl.Z - CurObject->Radius && 
        Pos.X < Box->Max.X / DestLocation_.Scl.X + CurObject->Radius &&
        Pos.Y < Box->Max.Y / DestLocation_.Scl.Y + CurObject->Radius &&
        Pos.Z < Box->Max.Z / DestLocation_.Scl.Z + CurObject->Radius)
    {
        
        /* Get the closet point to the box and check the collsion */
        
        dim::vector3df Normal;
        dim::vector3df CollisionPoint = math::CollisionLibrary::getClosestPoint(*Box, Pos, Normal);
        
        dim::vector3df Distance = CollisionPoint - Pos;
        
        /* Compute the distance between the collision point and the current position */
        
        if (Distance.getLengthSq() < math::Pow2(DestObject->Radius))
        {
            /* Fill the contact data */
            SCollisionContactData Contact;
            {
                Contact.Object          = DestObject->Object;//CurObject->Object;
                Contact.Point           = CollisionPoint;
                Contact.Normal          = DestLocation_.RotMat * Normal;
                Contact.Type            = COLLISION_SPHERE_TO_BOX;
                Contact.CollisionHandle = DestObject->CollisionHandle;
            }
            
            CollMatrix_ = DestLocation_.PosMat * DestLocation_.RotMat * DestLocation_.SclMat;
            Pos         = CollisionPoint - Distance.normalize() * CurObject->Radius / DestLocation_.Scl;
            
            /* Add new contact */
            processContact(CurObject, Contact, CollMatrix_ * Pos);
        } // fi
        
    } // fi
    
}


/*
 * Sphere-to-polygon collision
 */

void CollisionDetector::checkCollisionSphereToPolygon(SCollisionObject* CurObject, SCollisionObject* DestObject)
{
    
    /* Temporary variables */
    
    CollMatrix_ = DestLocation_.Matrix;
    
    dim::vector3df Pos(CurLocation_.Pos);
    
    dim::vector3df Distance, CollisionPoint;
    dim::triangle3df Triangle;
    
    dim::line3df Line(CurObject->LastPosition, Pos);
    
    const f32 RadiusSq = math::Pow2(CurObject->Radius);
    
    /*
     * Check if the last movement was to fast that intersection tests are need to proceeded
     */
    if (math::getDistanceSq(Line.Start, Line.End) > RadiusSq)
    {
        
        /* Loop for each triangle (make intersection tests first) */
        
        for (u32 i = 0; i < DestObject->TriangleCount; ++i)
        {
            
            /* Get the current triangle */
            Triangle = CollMatrix_ * DestObject->TriangleList[i].Triangle;
            
            /* Line-trianlge intersection test */
            if (math::CollisionLibrary::checkLineTriangleIntersection(Triangle, Line, CollisionPoint))
            {
                SCollisionContactData Contact = getPolygonCollisionContact(CurObject, DestObject, i, Triangle, CollisionPoint);
                
                Pos = CollisionPoint + Contact.Normal * CurObject->Radius;
                
                /* Add new contact */
                processContact(CurObject, Contact, Pos);
            }
            
        } // next triangle
        
    } // fi
    
    /*
     * Loop for each triangle (make closest normal point computations)
     */
    for (u32 i = 0; i < DestObject->TriangleCount; ++i)
    {
        
        /* Get the current triangle */
        Triangle = CollMatrix_ * DestObject->TriangleList[i].Triangle;
        
        /* Check triangle face side */
        if (!checkFaceSide(DestObject->FaceType, Triangle, Pos))
            continue;
        
        /* Get the closest normal to the current triangle */
        if (math::CollisionLibrary::getClosestPointStraight(Triangle, Pos, CollisionPoint))
        {
            Distance = CollisionPoint - Pos;
            
            if (Distance.getLengthSq() < RadiusSq)
            {
                Pos = CollisionPoint - Distance.normalize() * CurObject->Radius;
                
                /* Add new contact */
                processContact(
                    CurObject,
                    getPolygonCollisionContact(CurObject, DestObject, static_cast<s32>(i), Triangle, CollisionPoint),
                    Pos
                );
            }
        } // fi
        
    } // next triangle
    
    /*
     * Loop for each triangle (make final colsest point computations)
     */
    for (u32 i = 0; i < DestObject->TriangleCount; ++i)
    {
        
        /* Get the current triangle */
        Triangle = CollMatrix_ * DestObject->TriangleList[i].Triangle;
        
        /* Check the triangle side */
        if (!checkFaceSide(DestObject->FaceType, Triangle, Pos))
            continue;
        
        /* Get the closest point to the current triangle */
        CollisionPoint = math::CollisionLibrary::getClosestPoint(Triangle, Pos);
        
        Distance = CollisionPoint - Pos;
        
        if (Distance.getLengthSq() < RadiusSq)
        {
            Pos = CollisionPoint - Distance.normalize() * CurObject->Radius;
            
            /* Add new contact */
            processContact(
                CurObject,
                getPolygonCollisionContact(CurObject, DestObject, static_cast<s32>(i), Triangle, CollisionPoint),
                Pos
            );
        }
        
    } // next triangle
    
}


/*
 * Sphere-to-polygon collision (optimized by OctTrees)
 */

void CollisionDetector::checkCollisionSphereToPolygonTree(SCollisionObject* CurObject, SCollisionObject* DestObject)
{
    
    /* Temporary variables */
    
    CollMatrix_ = DestLocation_.Matrix;
    TreeMatrix_ = DestLocation_.MatrixInv;
    
    dim::vector3df Pos(CurLocation_.Pos);
    
    dim::vector3df Distance, CollisionPoint;
    dim::triangle3df Triangle;
    
    dim::line3df Line(CurObject->LastPosition, Pos);
    
    const f32 RadiusSq = math::Pow2(CurObject->Radius);
    
    /* Get OctTree nodes */
    
    std::list<const OcTreeNode*> TreeNodeList;
    std::list<STreeNodeTriangleData>::iterator it;
    std::list<STreeNodeTriangleData>* TriangleList;
    
    dim::vector3df TreeNodePos(TreeMatrix_ * Pos);
    dim::vector3df TreeNodeRadius(dim::vector3df(CurObject->Radius) / DestLocation_.Scl);
    
    DestObject->Mesh->getOctTreeRoot()->findTreeNodes(TreeNodeList, TreeNodePos, TreeNodeRadius);
    
    /*
     * Check if the last movement was to fast that intersection tests are need to proceeded
     */
    if (math::getDistanceSq(Line.Start, Line.End) > RadiusSq)
    {
        
        /* Loop for each triangle (make intersection tests first) */
        
        for (std::list<const OcTreeNode*>::iterator itTree = TreeNodeList.begin(); itTree != TreeNodeList.end(); ++itTree)
        {
            TriangleList = (std::list<STreeNodeTriangleData>*)(*itTree)->getUserData();
            
            for (it = TriangleList->begin(); it != TriangleList->end(); ++it)
            {
                
                /* Get the current triangle */
                Triangle = CollMatrix_ * it->Triangle;
                
                /* Line-trianlge intersection test */
                if (math::CollisionLibrary::checkLineTriangleIntersection(Triangle, Line, CollisionPoint))
                {
                    SCollisionContactData Contact = getPolygonCollisionContact(CurObject, DestObject, it, Triangle, CollisionPoint);
                    
                    Pos = CollisionPoint + Contact.Normal * CurObject->Radius;
                    
                    /* Add new contact */
                    processContact(CurObject, Contact, Pos);
                }
                
            } // next triangle
        } // next tree node
        
    } // fi
    
    /*
     * Loop for each triangle (make closest normal point computations)
     */
    for (std::list<const OcTreeNode*>::iterator itTree = TreeNodeList.begin(); itTree != TreeNodeList.end(); ++itTree)
    {
        TriangleList = (std::list<STreeNodeTriangleData>*)(*itTree)->getUserData();
        
        for (it = TriangleList->begin(); it != TriangleList->end(); ++it)
        {
            
            /* Get the current triangle */
            Triangle = CollMatrix_ * it->Triangle;
            
            /* Check triangle face side */
            if (!checkFaceSide(DestObject->FaceType, Triangle, Pos))
                continue;
            
            /* Get the closest normal to the current triangle */
            if (math::CollisionLibrary::getClosestPointStraight(Triangle, Pos, CollisionPoint))
            {
                Distance = CollisionPoint - Pos;
                
                if (Distance.getLengthSq() < RadiusSq)
                {
                    Pos = CollisionPoint - Distance.normalize() * CurObject->Radius;
                    
                    /* Add new contact */
                    processContact(
                        CurObject,
                        getPolygonCollisionContact(CurObject, DestObject, it, Triangle, CollisionPoint),
                        Pos
                    );
                }
            } // fi
            
        } // next triangle
    } // next tree node
    
    /*
     * Loop for each triangle (make final colsest point computations)
     */
    for (std::list<const OcTreeNode*>::iterator itTree = TreeNodeList.begin(); itTree != TreeNodeList.end(); ++itTree)
    {
        TriangleList = (std::list<STreeNodeTriangleData>*)(*itTree)->getUserData();
        
        for (it = TriangleList->begin(); it != TriangleList->end(); ++it)
        {
            
            /* Get the current triangle */
            Triangle = CollMatrix_ * it->Triangle;
            
            /* Check the triangle side */
            if (!checkFaceSide(DestObject->FaceType, Triangle, Pos))
                continue;
            
            /* Get the closest point to the current triangle */
            CollisionPoint = math::CollisionLibrary::getClosestPoint(Triangle, Pos);
            
            Distance = CollisionPoint - Pos;
            
            if (Distance.getLengthSq() < RadiusSq)
            {
                Pos = CollisionPoint - Distance.normalize() * CurObject->Radius;
                
                /* Add new contact */
                processContact(
                    CurObject,
                    getPolygonCollisionContact(CurObject, DestObject, it, Triangle, CollisionPoint),
                    Pos
                );
            }
            
        } // next triangle
    } // next tree node
    
}


/*
 * Box-to-polygon collision
 */

// !!! INCOMPLETE !!!

void CollisionDetector::checkCollisionBoxToPolygon(SCollisionObject* CurObject, SCollisionObject* DestObject)
{
    
    /* Temporary variables */
    
    CollMatrix_ = DestLocation_.Matrix;
    
    dim::vector3df Pos(CurLocation_.Pos);
    
    dim::vector3df CollisionPoint;
    dim::triangle3df Triangle;
    dim::plane3df Plane;
    
    f32 r, s;
    dim::obbox3df box(CurObject->BoundingBox.Min, CurObject->BoundingBox.Max);
    
    /* Transformt the bounding box */
    dim::matrix4f BoxMat(CurLocation_.RotMat * CurLocation_.SclMat);
    
    box.Axis.X = BoxMat * box.Axis.X;
    box.Axis.Y = BoxMat * box.Axis.Y;
    box.Axis.Z = BoxMat * box.Axis.Z;
    
    box.Center = CurLocation_.PosMat * box.Center;
    
    box.HalfSize = CurObject->BoundingBox.getSize() * 0.5f;
    
    /* Loop for each triangle (make box-triangle intersection tests) */
    
    for (u32 i = 0; i < DestObject->TriangleCount; ++i)
    {
        
        /* Get the current triangle and its plane */
        Triangle    = CollMatrix_ * DestObject->TriangleList[i].Triangle;
        Plane       = dim::plane3df(Triangle);
        Plane.Normal.normalize();
        
        /* Compute the projection interval radius of box onto L(t) = box.Cetner + t * Plane.Normal */
        r = box.HalfSize.X * math::Abs(Plane.Normal.dot(box.Axis.X)) +
            box.HalfSize.Y * math::Abs(Plane.Normal.dot(box.Axis.Y)) +
            box.HalfSize.Z * math::Abs(Plane.Normal.dot(box.Axis.Z));
        
        s = Plane.getPointDistance(box.Center);
        
        /* Check the box-plane intersection */
        if (math::Abs(s) <= r)
        {
            CollisionPoint = box.Center - Plane.Normal * s;
            
            /* Check then triangle intersection */
            if (Triangle.isPointInside(CollisionPoint))
            {
                SCollisionContactData Contact = getPolygonCollisionContact(
                    CurObject, DestObject, static_cast<s32>(i), Triangle, CollisionPoint
                );
                
                Pos = box.Center + Contact.Normal * (r - s);
                
                /* Add new contact */
                processContact(CurObject, Contact, Pos);
            }
        }
        
    } // next triangle
    
    #if 0 // !!!
    
    for (u32 i = 0; i < DestObject->TriangleCount; ++i)
    {
        
        /* Get the current triangle and its plane */
        Triangle    = CollMatrix_ * DestObject->TriangleList[i].Triangle;
        Plane       = dim::plane3df(Triangle);
        //Plane.Normal.normalize();
        
        dim::vector3df ClosestPoint(math::CollisionLibrary::getClosestPoint(Triangle, box.Center));
        Plane.Normal = (box.Center - ClosestPoint).normalize();
        
        /* Compute the projection interval radius of box onto L(t) = box.Cetner + t * Plane.Normal */
        r = box.HalfSize.X * math::Abs(Plane.Normal.dot(box.Axis.X)) +
            box.HalfSize.Y * math::Abs(Plane.Normal.dot(box.Axis.Y)) +
            box.HalfSize.Z * math::Abs(Plane.Normal.dot(box.Axis.Z));
        
        s = (box.Center - ClosestPoint).getLength();//Plane.getPointDistance(box.Center);
        
        /* Check the box-plane intersection */
        if (math::Abs(s) <= r)
        {
            CollisionPoint = box.Center - Plane.Normal * s;
            
            /* Check then triangle intersection */
            //if (Triangle.isPointInside(CollisionPoint))
            {
                SCollisionContactData Contact = getPolygonCollisionContact(CurObject, DestObject, i, Triangle, CollisionPoint);
                
                Pos = box.Center + Contact.Normal * (r - s);
                
                /* Add new contact */
                processContact(CurObject, Contact, Pos);
            }
        }
        
    } // next triangle
    
    #endif
    
}


/*
 * Line-sphere intersection
 */

void CollisionDetector::checkIntersectionLineSphere(SPickingObject* CurObject, const dim::line3df &Line)
{
    
    /* Temporary variables */
    
    dim::vector3df Pos(CurObject->Object->getPosition(true));
    dim::vector3df Intersection;
    
    /* Line-sphere intersection */
    
    if (math::CollisionLibrary::checkLineSphereIntersection(Line, Pos, CurObject->Radius, Intersection))
    {
        dim::vector3df Distance = math::getDistanceSq(Line.Start, Intersection);
        
        SPickingContact Contact;
        {
            Contact.Object      = CurObject->Object;
            Contact.Point       = Intersection;
            Contact.Normal      = Distance.normalize();
            Contact.Type        = PICKMODE_SPHERE;
            Contact.DistanceSq  = math::getDistanceSq(PickingStart_, Contact.Point);
        }
        PickContactList_.push_back(Contact);
    }
    
}


/*
 * Line-box intersection
 */

void CollisionDetector::checkIntersectionLineBox(SPickingObject* CurObject, const dim::line3df &Line)
{
    
    /* Transform the ray to the inverse object location */
    
    dim::matrix4f Mat(CurObject->Object->getTransformation(true).getInverse());
    dim::line3df Ray(Line);
    dim::vector3df Intersection;
    
    Ray.Start   = Mat * Line.Start;
    Ray.End     = Mat * Line.End;
    
    /* Line-box intersection */
    
    if (math::CollisionLibrary::checkLineBoxIntersection(Ray, CurObject->BoundingBox, Intersection))
    {
        SPickingContact Contact;
        {
            Contact.Object      = CurObject->Object;
            Contact.Point       = Mat.getInverse() * Intersection;
            //Contact.Normal      = ;
            Contact.Type        = PICKMODE_BOX;
            Contact.DistanceSq  = math::getDistanceSq(PickingStart_, Contact.Point);
        }
        PickContactList_.push_back(Contact);
    }
    
}


/*
 * Line-triangle intersection
 */

void CollisionDetector::checkIntersectionLinePolygon(SPickingObject* CurObject, const dim::line3df &Line)
{
    
    /* Temporary variables */
    
    dim::triangle3df Triangle;
    dim::vector3df Intersection;
    
    /* Get the object transformation */
    
    CollMatrix_ = CurObject->Object->getTransformation(true);
    
    /* Loop for each triangle */
    
    for (u32 i = 0; i < CurObject->TriangleCount; ++i)
    {
        
        Triangle = CollMatrix_ * CurObject->TriangleList[i].Triangle;
        
        /* Line-triangle intersection test */
        if (math::CollisionLibrary::checkLineTriangleIntersection(Triangle, Line, Intersection))
        {
            PickContactList_.push_back(
                getPolygonPickingContact(CurObject, static_cast<s32>(i), Triangle, Intersection)
            );
        }
        
    }
    
}


/*
 * Line-triangle intersection (optimized by OctTrees)
 */

void CollisionDetector::checkIntersectionLinePolygonTree(SPickingObject* CurObject, const dim::line3df &Line)
{
    
    /* Temporary variables */
    
    dim::triangle3df Triangle;
    dim::vector3df Intersection;
    
    /* Get the object transformation */
    
    CollMatrix_ = CurObject->Object->getTransformation(true);
    CollMatrix_.getInverse(TreeMatrix_);
    
    /* Get OctTree nodes */
    
    std::list<const OcTreeNode*> TreeNodeList;
    std::list<STreeNodeTriangleData>::iterator it;
    std::list<STreeNodeTriangleData>* TriangleList;
    
    dim::line3df TreeNodeLine(TreeMatrix_ * Line.Start, TreeMatrix_ * Line.End);
    
    CurObject->Mesh->getOctTreeRoot()->findTreeNodes(TreeNodeList, TreeNodeLine);
    
    /* Loop for each triangle */
    
    for (std::list<const OcTreeNode*>::iterator itTree = TreeNodeList.begin(); itTree != TreeNodeList.end(); ++itTree)
    {
        TriangleList = (std::list<STreeNodeTriangleData>*)(*itTree)->getUserData();
        
        for (it = TriangleList->begin(); it != TriangleList->end(); ++it)
        {
            
            Triangle = CollMatrix_ * it->Triangle;
            
            /* Line-triangle intersection test */
            if (math::CollisionLibrary::checkLineTriangleIntersection(Triangle, Line, Intersection))
            {
                PickContactList_.push_back(
                    getPolygonPickingContact(CurObject, it, Triangle, Intersection)
                );
            }
            
        } // next triangle
    } // next tree node
    
}


/*
 * Line-sphere intersection
 */

bool CollisionDetector::checkInviewLineSphere(SPickingObject* CurObject, const dim::line3df &Line)
{
    /* Temporary variabels */
    dim::vector3df Intersection;
    
    /* Line-sphere intersection test */
    return math::CollisionLibrary::checkLineSphereIntersection(
        Line, CurObject->Object->getPosition(true), CurObject->Radius, Intersection
    );
}


/*
 * Line-box intersection
 */

bool CollisionDetector::checkInviewLineBox(SPickingObject* CurObject, const dim::line3df &Line)
{
    /* Temporary variabels */
    dim::vector3df Intersection;
    dim::line3df Ray;
    
    /* Transform the ray inverse to the object location */
    dim::matrix4f Mat(CurObject->Object->getTransformation(true).getInverse());
    
    Ray.Start   = Mat * Line.Start;
    Ray.End     = Mat * Line.End;
    
    /* Line-box intersection test */
    return math::CollisionLibrary::checkLineBoxIntersection(Ray, CurObject->BoundingBox, Intersection);
}


/*
 * Line-triangle intersection
 */

bool CollisionDetector::checkInviewLinePolygon(SPickingObject* CurObject, const dim::line3df &Line)
{
    /* Temporary variables */
    dim::vector3df Intersection;
    
    /* Get the object transformation */
    CollMatrix_ = CurObject->Object->getTransformation(true);
    
    /* Loop for each triangle and make an intersection test */
    for (u32 i = 0; i < CurObject->TriangleCount; ++i)
    {
        if ( math::CollisionLibrary::checkLineTriangleIntersection((CollMatrix_ * CurObject->TriangleList[i].Triangle), Line, Intersection) )
            return true;
    }
    
    return false;
}


/*
 * Line-triangle intersection (optimized by OctTrees)
 */

bool CollisionDetector::checkInviewLinePolygonTree(SPickingObject* CurObject, const dim::line3df &Line)
{
    
    /* Temporary variables */
    
    dim::vector3df Intersection;
    
    /* Get the object transformation */
    
    CollMatrix_ = CurObject->Object->getTransformation(true);
    CollMatrix_.getInverse(TreeMatrix_);
    
    /* Get OctTree nodes */
    
    std::list<const OcTreeNode*> TreeNodeList;
    std::list<STreeNodeTriangleData>::iterator it;
    std::list<STreeNodeTriangleData>* TriangleList;
    
    dim::line3df TreeNodeLine(TreeMatrix_ * Line.Start, TreeMatrix_ * Line.End);
    
    CurObject->Mesh->getOctTreeRoot()->findTreeNodes(TreeNodeList, TreeNodeLine);
    
    /* Loop for each triangle and make an intersection test */
    
    for (std::list<const OcTreeNode*>::iterator itTree = TreeNodeList.begin(); itTree != TreeNodeList.end(); ++itTree)
    {
        TriangleList = (std::list<STreeNodeTriangleData>*)(*itTree)->getUserData();
        
        for (it = TriangleList->begin(); it != TriangleList->end(); ++it)
        {
            if ( math::CollisionLibrary::checkLineTriangleIntersection((CollMatrix_ * it->Triangle), Line, Intersection) )
                return true;
        }
    }
    
    return false;
    
}


/*
 * Mesh-mesh intersection (normal-normal)
 */

bool CollisionDetector::checkIntersectionMeshMesh(Mesh* MeshA, Mesh* MeshB)
{
    
    #if 0
    
    /* Get the matrix transformations */
    
    dim::matrix4f MatA(MeshA->getTransformation(true)), MatB(MeshB->getTransformation(true));
    dim::matrix4f TmpMat;
    
    /* Make bounding volume intersection test first */
    
    dim::aabbox3df AABBoxA(MeshA->getMeshBoundingBox());
    dim::aabbox3df AABBoxB(MeshB->getMeshBoundingBox());
    
    dim::matrix4f BoxMatA(MatA), BoxMatB(MatB);
    
    BoxMatA.translate(AABBoxA.getCenter());
    BoxMatB.translate(AABBoxB.getCenter());
    
    /* Create OBBox for MeshA */
    
    TmpMat = MeshA->getRotationMatrix(true);
    
    dim::obbox3df OBBoxA(
        BoxMatA.getPosition(), TmpMat * dim::vector3df(1, 0, 0), TmpMat * dim::vector3df(0, 1, 0), TmpMat * dim::vector3df(0, 0, 1)
    );
    OBBoxA.HalfSize *= AABBoxA.getSize() * 0.5 * MeshA->getScale(true);
    
    /* Create OBBox for MeshB */
    
    TmpMat = MeshB->getRotationMatrix(true);
    
    dim::obbox3df OBBoxB(
        BoxMatB.getPosition(), TmpMat * dim::vector3df(1, 0, 0), TmpMat * dim::vector3df(0, 1, 0), TmpMat * dim::vector3df(0, 0, 1)
    );
    OBBoxB.HalfSize *= AABBoxB.getSize() * 0.5 * MeshB->getScale(true);
    
    /* Make OBBox-OBBox intersection test */
    
    if (!math::CollisionLibrary::checkOBBoxOBBoxIntersection(OBBoxA, OBBoxB))
        return false;
    
    /* Temporary variables */
    
    dim::line3df IntersectionLine;
    dim::triangle3df TriangleA, TriangleB;
    std::vector<SMeshTriangle3D>::iterator itTriA, itTriB;
    std::vector<video::MeshBuffer*>::iterator itSurfA, itSurfB;
    
    /* Loop for each triangles of MeshA */
    
    for (itSurfA = MeshA->SurfaceList_->begin(); itSurfA != MeshA->SurfaceList_->end(); ++itSurfA)
    {
        for (itTriA = (*itSurfA)->TriangleList.begin(); itTriA != (*itSurfA)->TriangleList.end(); ++itTriA)
        {
            
            /* Get the first triangle */
            TriangleA = dim::triangle3df(
                MatA * (*itSurfA)->VerticesList[itTriA->a].getPosition(),
                MatA * (*itSurfA)->VerticesList[itTriA->b].getPosition(),
                MatA * (*itSurfA)->VerticesList[itTriA->c].getPosition()
            );
            
            /* Loop for each triangles of MeshB */
            for (itSurfB = MeshB->SurfaceList_->begin(); itSurfB != MeshB->SurfaceList_->end(); ++itSurfB)
            {
                for (itTriB = (*itSurfB)->TriangleList.begin(); itTriB != (*itSurfB)->TriangleList.end(); ++itTriB)
                {
                    
                    /* Get the second triangle */
                    TriangleB = dim::triangle3df(
                        MatB * (*itSurfB)->VerticesList[itTriB->a].getPosition(),
                        MatB * (*itSurfB)->VerticesList[itTriB->b].getPosition(),
                        MatB * (*itSurfB)->VerticesList[itTriB->c].getPosition()
                    );
                    
                    /* Check for an intersection */
                    if (math::CollisionLibrary::checkTriangleTriangleIntersection(TriangleA, TriangleB, IntersectionLine))
                        return true;
                    
                } // next triangle (MeshB)
            } // next surface (MeshB)
            
        } // next triangle (MeshA)
    } // next surface (MeshA)
    
    /* No intersection detected */
    return false;
    
    #endif
    
    return false;
    
}


/*
 * Mesh-mesh intersection (tree-normal)
 */

bool CollisionDetector::checkIntersectionMeshMesh(Mesh* MeshA, Mesh* MeshB, OcTreeNode* TreeNodeA)
{
    
    return false;
}


/*
 * Mesh-mesh intersection (tree-tree)
 */

bool CollisionDetector::checkIntersectionMeshMesh(Mesh* MeshA, Mesh* MeshB, OcTreeNode* TreeNodeA, OcTreeNode* TreeNodeB)
{
    
    return false;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
