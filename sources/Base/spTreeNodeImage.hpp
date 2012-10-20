/*
 * Tree node rect header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TREENODEIMAGE_H__
#define __SP_TREENODEIMAGE_H__


#include "Base/spStandard.hpp"


namespace sp
{
namespace scene
{


/**
The ImageTreeNode class is used to - as the name implies - store 2D images in rectangles.
This tree node does not hold any information about its parent. Therefore only a traversal from parent to children is possible.
This class is used in the lightmap generator and to generate font textures.
\tparam T: This class must implement the following functions:
\code
dim::size2di getSize() const;
void setupTreeNode(ImageTreeNode<T>* Node);
\endcode
\since Version 3.2
*/
template <class T> class ImageTreeNode
{
    
    public:
        
        ImageTreeNode() :
            ChildA_ (0),
            ChildB_ (0),
            Image_  (0)
        {
        }
        ImageTreeNode(const dim::size2di &Size) :
            Rect_   (0, 0, Size.Width, Size.Height  ),
            ChildA_ (0                              ),
            ChildB_ (0                              ),
            Image_  (0                              )
        {
        }
        ImageTreeNode(const dim::rect2di &Rect) :
            Rect_   (Rect   ),
            ChildA_ (0      ),
            ChildB_ (0      ),
            Image_  (0      )
        {
        }
        ~ImageTreeNode()
        {
            delete ChildA_;
            delete ChildB_;
        }
        
        /* === Functions === */
        
        /**
        Tries to insert the given image into the tree node.
        \param Image: Pointer to the image object. This must not be null!
        \return Pointer to the final ImageTreeNode object which has been found to hold the given image.
        If no suitable tree node could be found the return value is null.
        \note For the root tree node the rectangle must be set manual! All the other rectangles will be computed automatically.
        */
        ImageTreeNode<T>* insert(T* Image)
        {
            if (!Image)
                return 0;
            
            if (ChildA_)
            {
                /* Try to find a suitable tree node */
                ImageTreeNode<T>* NewNode = ChildA_->insert(Image);
                
                if (NewNode)
                    return NewNode;
                
                return ChildB_->insert(Image);
            }
            
            /* Get image size */
            const dim::size2di ImageSize(Image->getSize());
            
            if (Image_ || ImageSize.Width > Rect_.getWidth() || ImageSize.Height > Rect_.getHeight())
                return 0;
            
            /* Check if image fits exactly into this tree node */
            if (ImageSize == Rect_.getSize())
            {
                Image_ = Image;
                Image_->setupTreeNode(this);
                return this;
            }
            
            /* Create children and split into two spaces */
            ChildA_ = new ImageTreeNode<T>();
            ChildB_ = new ImageTreeNode<T>();
            
            if (Rect_.getWidth() - ImageSize.Width > Rect_.getHeight() - ImageSize.Height)
            {
                ChildA_->Rect_ = dim::rect2di(Rect_.Left, Rect_.Top, Rect_.Left + ImageSize.Width, Rect_.Bottom);
                ChildB_->Rect_ = dim::rect2di(Rect_.Left + ImageSize.Width, Rect_.Top, Rect_.Right, Rect_.Bottom);
            }
            else
            {
                ChildA_->Rect_ = dim::rect2di(Rect_.Left, Rect_.Top, Rect_.Right, Rect_.Top + ImageSize.Height);
                ChildB_->Rect_ = dim::rect2di(Rect_.Left, Rect_.Top + ImageSize.Height, Rect_.Right, Rect_.Bottom);
            }
            
            /* Try to insert the image into the new first child */
            return ChildA_->insert(Image);
        }
        
        //! Deletes the children nodes.
        void deleteChildren()
        {
            delete ChildA_;
            delete ChildB_;
            
            ChildA_ = 0;
            ChildB_ = 0;
        }
        
        /* === Inline functions === */
        
        inline void setRect(const dim::rect2di &Rect)
        {
            Rect_ = Rect;
        }
        inline dim::rect2di getRect() const
        {
            return Rect_;
        }
        
        inline T* getImage() const
        {
            return Image_;
        }
        
        inline ImageTreeNode<T>* getChildA() const
        {
            return ChildA_;
        }
        inline ImageTreeNode<T>* getChildB() const
        {
            return ChildB_;
        }
        
    private:
        
        /* Members */
        
        dim::rect2di Rect_;
        
        ImageTreeNode<T>* ChildA_;
        ImageTreeNode<T>* ChildB_;
        
        T* Image_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
