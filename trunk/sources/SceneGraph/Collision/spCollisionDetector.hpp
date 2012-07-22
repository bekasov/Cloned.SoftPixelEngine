/*
 * Collision detector header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISIONDETECTOR_H__
#define __SP_COLLISIONDETECTOR_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollision.hpp"

#include <vector>


namespace sp
{
namespace scene
{


//! CollisionDetector class for collision- detection and resolving and intersection tests.
class SP_EXPORT CollisionDetector
{
    
    public:
        
        CollisionDetector();
        virtual ~CollisionDetector();
        
        /* === Collision detection === */
        
        /**
        Creates a new collision. Collision objects are used for different collision types
        and different relationships between multiple collisions.
        \return Pointer to the new Collision which has been created.
        */
        Collision* createCollision();
        void deleteCollision(Collision* &CollisionObject);
        
        /**
        Creates a new collision object. For this object many collision detection procedures
        are processed when calling "updateScene".
        \param Object: Node object which is to be added as a collision object.
        When adding an Mesh object (a 3D model) use the "addCollisionMesh" function.
        \param CollisionObject: Collision object which is to be used for this object.
        \param Radius: Specifies the radius when using a sphere-to-sphere collision.
        \param BoundingBox: Specifies the bounding box when using a sphere-to-box collision.
        \return Pointer to the new SCollisionObject which has been created.
        */
        SCollisionObject* addCollisionObject(
            SceneNode* Object, Collision* CollisionObject,
            f32 Radius = 0.5f, const dim::aabbox3df &BoundingBox = dim::aabbox3df()
        );
        
        /**
        Creates a new collision mesh. Use this function when a 3D model is to be added as a
        collision object. This can be used for a sphere-to-polygon collision.
        When using collision-meshes you have to work with OctTrees which can be created easly
        using the "Mesh::createOctTree" function for optimization. Otherwise the collision
        procedures can be very slow with large models.
        \param Mesh: 3D model which is to be added as a collision object.
        \param FaceType: Type of face side which is to be used. If the 3D model uses the
        back-face you should use the same side for collision detection.
        */
        SCollisionObject* addCollisionMesh(
            Mesh* Mesh, Collision* CollisionObject, const video::EFaceTypes FaceType = video::FACE_FRONT
        );
        
        //! \return Pointer to the collision object which has the specified Node object.
        SCollisionObject* getCollisionObject(SceneNode* Object);
        
        void removeCollisionObject(SCollisionObject* Object);
        
        /**
        Updates the whole scene for collision detection. Call this function before rendering the scene.
        All collisions will be updated here.
        */
        void updateScene();
        
        /* === Picking intersection === */
        
        /**
        Creates a new picking object.
        \param Object: Node object which is to be added as a pickable object.
        \param Type: Picking type. If you want to create a polygon-picking object use the "addPickingMesh" function.
        \param Radius: Specifies the radius when using a sphere-picking object.
        \param BoundingBox: Specifies the bounding box when using a box-picking object.
        \return Pointer to the new picking object which has been created.
        */
        SPickingObject* addPickingObject(
            SceneNode* Object, const EPickingTypes Type,
            f32 Radius = 0.5f, const dim::aabbox3df &BoundingBox = dim::aabbox3df()
        );
        
        /**
        Creates a new picking mesh. This is always from the type PICKMODE_POLYGON.
        \param Mesh: 3D model which is to be added as a pickable mesh. Here you should
        use OctTrees as well.
        */
        SPickingObject* addPickingMesh(Mesh* Mesh);
        
        SPickingObject* getPickingObject(SceneNode* Object);
        
        void removePickingObject(SPickingObject* Object);
        
        /**
        Processes a line intersection. Here all pickable objects are checked for intersection tests.
        \param Line: Intersection line/ ray. You can the Camera::getPickingLine function to make
        an intersection test with the cursor.
        \return List to all intersections which has been detected. The intersections are sorted
        by the depth. Nearest intersections to the line start are at the top of the list.
        */
        std::list<SPickingContact> pickIntersection(
            const dim::line3df &Line, const video::EFaceTypes FaceType = video::FACE_FRONT
        );
        bool pickIntersection(const dim::line3df &Line, SPickingContact &Contact);
        bool pickIntersection(const dim::vector3df &PosA, const dim::vector3df &PosB);
        
        //! Deletes all collisions, collision objects and picking objects.
        void clearScene(bool isDeleteCollision = true, bool isDeletePicking = true);
        
        /* === Single intersection tests === */
        
        //! \return True if both objects MeshA and MeshB are intersecting. OctTree optimization here is not supported yet.
        bool meshIntersection(Mesh* MeshA, Mesh* MeshB);
        
        /* === Inline functions === */
        
        //! Enables or disables the automatically sorting for picking procedures. By default it is enabled.
        inline void setAutoSort(bool Enable)
        {
            isAutoSort_ = Enable;
        }
        inline bool getAutoSort() const
        {
            return isAutoSort_;
        }
        
    protected:
        
        /* === Structures === */
        
        struct SObjectLocation
        {
            void update(SceneNode* Object);
            
            dim::matrix4f Matrix, MatrixInv;
            dim::matrix4f PosMat, RotMat, SclMat;
            dim::vector3df Pos, Rot, Scl;
        };
        
        /* === Functions === */
        
        SCollisionContactData getPolygonCollisionContact(
            SCollisionObject* CurObject, SCollisionObject* DestObject, s32 TriangleListIndex,
            const dim::triangle3df &Triangle, const dim::vector3df &CollisionPoint
        );
        SCollisionContactData getPolygonCollisionContact(
            SCollisionObject* CurObject, SCollisionObject* DestObject,
            std::list<STreeNodeTriangleData>::iterator TriangleIterator,
            const dim::triangle3df &Triangle, const dim::vector3df &CollisionPoint
        );
        
        SPickingContact getPolygonPickingContact(
            SPickingObject* Object, s32 TriangleListIndex,
            const dim::triangle3df &Triangle, const dim::vector3df &Intersection
        );
        SPickingContact getPolygonPickingContact(
            SPickingObject* Object, std::list<STreeNodeTriangleData>::iterator TriangleIterator,
            const dim::triangle3df &Triangle, const dim::vector3df &Intersection
        );
        
        void processContact(
            SCollisionObject* CurObject, const SCollisionContactData &Contact, const dim::vector3df &Position
        );
        
        bool checkFaceSide(
            video::EFaceTypes FaceType, const dim::triangle3df &Triangle, const dim::vector3df &Position
        );
        
        void processPicking(const dim::line3df &Line);
        
        /* Collision detection */
        void checkCollisionSphereToSphere       (SCollisionObject* CurObject, SCollisionObject* DestObject);
        void checkCollisionSphereToBox          (SCollisionObject* CurObject, SCollisionObject* DestObject);
        void checkCollisionSphereToPolygon      (SCollisionObject* CurObject, SCollisionObject* DestObject);
        void checkCollisionSphereToPolygonTree  (SCollisionObject* CurObject, SCollisionObject* DestObject);
        
        void checkCollisionBoxToPolygon         (SCollisionObject* CurObject, SCollisionObject* DestObject);
        
        /* Intersection tests */
        void checkIntersectionLineSphere        (SPickingObject* CurObject, const dim::line3df &Line);
        void checkIntersectionLineBox           (SPickingObject* CurObject, const dim::line3df &Line);
        void checkIntersectionLinePolygon       (SPickingObject* CurObject, const dim::line3df &Line);
        void checkIntersectionLinePolygonTree   (SPickingObject* CurObject, const dim::line3df &Line);
        
        /* Inview intersection tests */
        bool checkInviewLineSphere      (SPickingObject* CurObject, const dim::line3df &Line);
        bool checkInviewLineBox         (SPickingObject* CurObject, const dim::line3df &Line);
        bool checkInviewLinePolygon     (SPickingObject* CurObject, const dim::line3df &Line);
        bool checkInviewLinePolygonTree (SPickingObject* CurObject, const dim::line3df &Line);
        
        /* Object intersection tests */
        bool checkIntersectionMeshMesh  (Mesh* MeshA, Mesh* MeshB);
        bool checkIntersectionMeshMesh  (Mesh* MeshA, Mesh* MeshB, OcTreeNode* TreeNodeA);
        bool checkIntersectionMeshMesh  (Mesh* MeshA, Mesh* MeshB, OcTreeNode* TreeNodeA, OcTreeNode* TreeNodeB);
        
        /* === Members === */
        
        std::list<Collision*> CollisionList_;
        std::list<SCollisionObject*> CollObjectList_;
        std::list<SPickingObject*> PickObjectList_;
        
        std::list<SPickingContact> PickContactList_;
        
        dim::matrix4f CollMatrix_, TreeMatrix_;
        
        SObjectLocation CurLocation_, DestLocation_;
        dim::vector3df PickingStart_;
        
        bool isAutoSort_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
