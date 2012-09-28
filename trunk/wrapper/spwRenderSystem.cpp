/*
 * Wrapper video driver file
 * 
 * This file is part of the "SoftPixel Engine Wrapper" (Copyright (c) 2008 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "spwStandard.hpp"


SPW_PROC(bool) spwFlip(bool Vsync = false)
{
    g_RenderContext->flipBuffers();
    bool Result = g_Device->updateEvent();
    g_RenderSystem->clearBuffers();
    
    #if defined(SP_PLATFORM_WINDOWS)
    
    if (!Result && g_EndMessage != "")
    {
        int Answer = MessageBox(
            *(HWND*)g_RenderContext->getWindowObject(), TEXT(g_EndMessage.c_str()), TEXT(g_AppTitle.c_str()), MB_OKCANCEL
        );
        
        if (Answer == IDCANCEL)
            Result = true;
    }
    
    #endif
    
    return Result;
}

SPW_PROC(LP) spwCreateShaderTable_ex()
{
    return CAST(g_RenderSystem->createShaderTable());
}
SPW_PROC(bool) spwShaderTableLink_ex(LP ShaderTable)
{
    return SHADERCLASS(ShaderTable)->link();
}
SPW_PROC(LP) spwLoadShader_ex(
    LP ShaderTable, int Type, int Version, STR Filename, STR EntryPoint)
{
    return CAST(g_RenderSystem->loadShader(
        SHADERCLASS(ShaderTable), (video::EShaderTypes)Type, (video::EShaderVersions)Version, Filename, EntryPoint
    ));
}
SPW_PROC(void) spwSetShader_ex(LP Entity, LP ShaderTable)
{
    MESH(Entity)->setShaderTable(SHADERCLASS(ShaderTable));
}
SPW_PROC(void) spwBindShader_ex(LP ShaderTable)
{
    SHADERCLASS(ShaderTable)->bind();
}
SPW_PROC(void) spwUnbindShader_ex(LP ShaderTable)
{
    SHADERCLASS(ShaderTable)->unbind();
}
SPW_PROC(bool) spwSetShaderInt_ex(LP Shader, STR Name, int Value)
{
    return SHADER(Shader)->setConstant(Name, Value);
}
SPW_PROC(bool) spwSetShaderFloat_ex(LP Shader, STR Name, float Value)
{
    return SHADER(Shader)->setConstant(Name, Value);
}
SPW_PROC(bool) spwSetShaderVec_ex(LP Shader, STR Name, float X, float Y, float Z)
{
    return SHADER(Shader)->setConstant(Name, dim::vector3df(X, Y, Z));
}
SPW_PROC(bool) spwSetShaderColor_ex(LP Shader, STR Name, int Red, int Green, int Blue, int Alpha)
{
    return SHADER(Shader)->setConstant(Name, video::color(Red, Green, Blue, Alpha));
}
SPW_PROC(bool) spwSetShaderMat_ex(LP Shader, STR Name, LP Matrix)
{
    return SHADER(Shader)->setConstant(Name, *MATRIX(Matrix));
}
SPW_PROC(bool) spwSetShaderBuffer_ex(LP Shader, STR Name, void* Buffer)
{
    return SHADER(Shader)->setConstantBuffer(Name, Buffer);
}
SPW_PROC(void) spwFreeShaderTable_ex(LP ShaderTable)
{
    video::ShaderClass* ShaderTableObject = SHADERCLASS(ShaderTable);
    g_RenderSystem->deleteShaderTable(ShaderTableObject);
}
SPW_PROC(void) spwFreeShader_ex(LP Shader)
{
    video::Shader* ShaderObject = SHADER(Shader);
    g_RenderSystem->deleteShader(ShaderObject);
}

SPW_PROC(void) spwClippingPlane_ex(int Index, bool Enable, float NX, float NY, float NZ, float PlaneDistance)
{
    g_RenderSystem->setClipPlane((unsigned int)Index, dim::plane3df(dim::vector3df(NX, NY, NZ), PlaneDistance), Enable);
}
SPW_PROC(void) spwFrontCulling_ex(bool Enable)
{
    g_RenderSystem->setFrontFace(Enable);
}
SPW_PROC(void) spwSetRenderState_ex(int Type, int State)
{
    g_RenderSystem->setRenderState(static_cast<video::ERenderStates>(Type), State);
}
SPW_PROC(int) spwGetRenderState_ex(int Type)
{
    return g_RenderSystem->getRenderState(static_cast<video::ERenderStates>(Type));
}

SPW_PROC(void) spwColor(int Red, int Green, int Blue, int Alpha = 255)
{
    g_Color.Red     = Red;
    g_Color.Green   = Green;
    g_Color.Blue    = Blue;
    g_Color.Alpha   = Alpha;
}
SPW_PROC(void) spwGetColor(int X, int Y)
{
    g_PixelColor = g_RenderSystem->getPixelColor(dim::point2di(X, Y));
}
SPW_PROC(int) spwColorRed()
{
    return g_PixelColor.Red;
}
SPW_PROC(int) spwColorGreen()
{
    return g_PixelColor.Green;
}
SPW_PROC(int) spwColorBlue()
{
    return g_PixelColor.Blue;
}
SPW_PROC(void) spwMultiColor(int Index, int Red, int Green, int Blue, float Alpha = 1.0)
{
    if (Index >= 0 && Index <= 3)
    {
        g_MultiColor[Index].Red     = Red;
        g_MultiColor[Index].Green   = Green;
        g_MultiColor[Index].Blue    = Blue;
        g_MultiColor[Index].Alpha   = (int)(Alpha*255);
    }
}
SPW_PROC(void) spwEnableMultiColor_ex(bool Enable)
{
    g_EnableMultiColor = Enable;
}

SPW_PROC(void) spwWritePixelFast(int X, int Y, int Color, LP Buffer = 0)
{
    video::Texture* LastRenderTarget = g_RenderSystem->getRenderTarget();
    if (Buffer)
        g_RenderSystem->setRenderTarget(TEXTURE(Buffer));
    
    g_RenderSystem->beginDrawing2D();
    g_RenderSystem->draw2DPoint(
        g_Origin2D + dim::point2di(X, Y),
        video::color(video::getRed(Color), video::getGreen(Color), video::getBlue(Color), video::getAlpha(Color))
    );
    g_RenderSystem->endDrawing2D();
    
    if (Buffer)
        g_RenderSystem->setRenderTarget(LastRenderTarget);
}
SPW_PROC(void) spwWritePixel(int X, int Y, int Color, LP Buffer = 0)
{
    spwWritePixelFast(X, Y, Color, Buffer);
}
SPW_PROC(int) spwReadPixelFast(int X, int Y, LP Buffer = 0)
{
    video::Texture* LastRenderTarget = g_RenderSystem->getRenderTarget();
    if (Buffer)
        g_RenderSystem->setRenderTarget(TEXTURE(Buffer));
    
    int Color = g_RenderSystem->getPixelColor(dim::point2di(X, Y)).getSingle();
    
    if (Buffer)
        g_RenderSystem->setRenderTarget(LastRenderTarget);
    
    return Color;
}
SPW_PROC(int) spwReadPixel(int X, int Y, LP Buffer = 0)
{
    return spwReadPixelFast(X, Y, Buffer);
}
SPW_PROC(void) spwCopyPixelFast(int SrcX, int SrcY, LP SrcBuffer, int DestX, int DestY, LP DestBuffer = 0)
{
    video::Texture* LastRenderTarget = g_RenderSystem->getRenderTarget();
    
    if (SrcBuffer)
        g_RenderSystem->setRenderTarget(TEXTURE(SrcBuffer));
    
    const video::color Color = g_RenderSystem->getPixelColor(dim::point2di(SrcX, SrcY));
    
    g_RenderSystem->setRenderTarget(DestBuffer ? TEXTURE(DestBuffer) : LastRenderTarget);
    
    g_RenderSystem->beginDrawing2D();
    g_RenderSystem->draw2DPoint(dim::point2di(DestX, DestY), Color);
    g_RenderSystem->endDrawing2D();
    
    if (DestBuffer)
        g_RenderSystem->setRenderTarget(LastRenderTarget);
}
SPW_PROC(void) spwCopyPixel(int SrcX, int SrcY, LP SrcBuffer, int DestX, int DestY, LP DestBuffer = 0)
{
    spwCopyPixelFast(SrcX, SrcY, SrcBuffer, DestX, DestY, DestBuffer);
}
SPW_PROC(void) spwCopyRect(
    int SrcX, int SrcY, int SrcWidth, int SrcHeight, int DestX, int DestY, LP SrcBuffer = 0, LP DestBuffer = 0)
{
    video::Texture* LastRenderTarget    = g_RenderSystem->getRenderTarget();
    video::Texture* SrcBufferTex        = TEXTURE(SrcBuffer);
    video::Texture* DestBufferTex       = TEXTURE(DestBuffer);
    
    if (SrcBufferTex)
    {
        if (!SrcBufferTex->getRenderTarget())
            SrcBufferTex->setRenderTarget(true);
        g_RenderSystem->setRenderTarget(SrcBufferTex);
    }
    
    g_RenderSystem->setTextureGenFlags(video::TEXGEN_MIPMAPS, false);
    video::Texture* TempTex = g_RenderSystem->createScreenShot(dim::point2di(SrcX, SrcY), dim::size2di(SrcWidth, SrcHeight));
    g_RenderSystem->setTextureGenFlags(video::TEXGEN_MIPMAPS, true);
    
    if (DestBufferTex)
    {
        if (!DestBufferTex->getRenderTarget())
            DestBufferTex->setRenderTarget(true);
        g_RenderSystem->setRenderTarget(DestBufferTex);
    }
    else
        g_RenderSystem->setRenderTarget(LastRenderTarget);
    
    g_RenderSystem->beginDrawing2D();
    g_RenderSystem->draw2DImage(TempTex, dim::point2di(DestX, DestY));
    g_RenderSystem->endDrawing2D();
    
    g_RenderSystem->deleteTexture(TempTex);
    
    if (DestBufferTex)
        g_RenderSystem->setRenderTarget(LastRenderTarget);
}

SPW_PROC(void) spwPlot(int X, int Y)
{
    g_RenderSystem->beginDrawing2D();
    g_RenderSystem->draw2DPoint(g_Origin2D + dim::point2di(X, Y), g_Color);
    g_RenderSystem->endDrawing2D();
}
SPW_PROC(void) spwLine(int X1, int Y1, int X2, int Y2)
{
    g_RenderSystem->beginDrawing2D();
    
    if (g_EnableMultiColor)
    {
        g_RenderSystem->draw2DLine(
            g_Origin2D + dim::point2di(X1, Y1), g_Origin2D + dim::point2di(X2, Y2), g_MultiColor[0], g_MultiColor[1]
        );
    }
    else
        g_RenderSystem->draw2DLine(g_Origin2D + dim::point2di(X1, Y1), g_Origin2D + dim::point2di(X2, Y2), g_Color);
    
    g_RenderSystem->endDrawing2D();
}
SPW_PROC(void) spwRect(int X, int Y, int Width, int Height, bool Solid = true)
{
    X += g_Origin2D.X;
    Y += g_Origin2D.Y;
    g_RenderSystem->beginDrawing2D();
    
    if (g_EnableMultiColor)
    {
        g_RenderSystem->draw2DRectangle(
            dim::rect2di(X, Y, X + Width, Y + Height), g_MultiColor[0], g_MultiColor[1], g_MultiColor[2], g_MultiColor[3], Solid
        );
    }
    else
        g_RenderSystem->draw2DRectangle(dim::rect2di(X, Y, X + Width, Y + Height), g_Color, Solid);
    
    g_RenderSystem->endDrawing2D();
}
SPW_PROC(void) spwOval(int X, int Y, int Width, int Height, bool Solid = true)
{
    //drawing ellipse has been removed
}

SPW_PROC(LP) spwCreateTexture(int Width, int Height, int Flags = 9, int Frames = 0)
{
    video::Texture* Obj = g_RenderSystem->createTexture(dim::size2di(Width, Height));
    SWrapTexture Texture;
    stcSetTextureFalgs(Obj, Flags, &Texture);
    g_TextureList[CAST(Obj)] = Texture;
    return CAST(Obj);
}
SPW_PROC(LP) spwLoadTexture(STR File, int Flags = 9)
{
    video::Texture* Obj = g_RenderSystem->loadTexture(File);
    SWrapTexture Texture;
    stcSetTextureFalgs(Obj, Flags, &Texture);
    g_TextureList[CAST(Obj)] = Texture;
    return CAST(Obj);
}
SPW_PROC(LP) spwLoadAnimTexture(
    STR File, int Flags, int FrameWidth, int FrameHeight, int FirstFrame, int FrameCount)
{
    LP Handle = spwLoadTexture(File, Flags);
    video::Texture* Obj = TEXTURE(Handle);
    Obj->setAnimation(true);
    return Handle;
}
SPW_PROC(LP) spwCopyTexture_ex(LP SourceTexture)
{
    video::Texture* SourceObj = TEXTURE(SourceTexture);
    
    if (SourceObj->getRenderTarget())
        SourceObj->shareImageBuffer();
    
    video::Texture* Obj = g_RenderSystem->copyTexture(SourceObj);
    
    SWrapTexture Texture = g_TextureList[SourceTexture];
    g_TextureList[CAST(Obj)] = Texture;
    
    return CAST(Obj);
}
SPW_PROC(void) spwFreeTexture(LP Texture)
{
    video::Texture* Obj = TEXTURE(Texture);
    g_RenderSystem->deleteTexture(Obj);
}
SPW_PROC(void) spwScaleTexture(LP Texture, float Width, float Height)
{
    g_TextureList[Texture].Scale = dim::size2df(1.0 / Width, 1.0 / Height);
    stcTransformTextureMatrix(g_TextureList[Texture]);
}
SPW_PROC(void) spwPositionTexture(LP Texture, float X, float Y)
{
    g_TextureList[Texture].Position = dim::point2df(X, Y);
    stcTransformTextureMatrix(g_TextureList[Texture]);
}
SPW_PROC(void) spwRotateTexture(LP Texture, float Angle)
{
    g_TextureList[Texture].Angle = Angle;
    stcTransformTextureMatrix(g_TextureList[Texture]);
}
SPW_PROC(void) spwTextureBlend(LP Texture, int Mode)
{
    video::ETextureEnvTypes BlendMode;
    
    g_TextureList[Texture].isEnabled = (Mode != 0);
    
    switch (Mode)
    {
        case 0: // no texture
            return;
        case 1: // one texture
            BlendMode = video::TEXENV_REPLACE; break;
        case 2: // multiply
            BlendMode = video::TEXENV_MODULATE; break;
        case 3: // add
            BlendMode = video::TEXENV_ADD; break;
        case 4: // dot3
            BlendMode = video::TEXENV_DOT3; break;
        case 5: // multiply 2
            BlendMode = video::TEXENV_INTERPOLATE; break;
        case 6: // add signed
            BlendMode = video::TEXENV_ADDSIGNED; break;
        case 7: // subtract
            BlendMode = video::TEXENV_SUBTRACT; break;
        default:
            return;
    }
    
    g_TextureList[Texture].BlendMode = BlendMode;
}
SPW_PROC(void) spwTextureCoords(LP Texture, int Coords)
{
    g_TextureList[Texture].CoordsLayer = Coords;
}

SPW_PROC(void) spwEntityTexture(
    LP Entity, LP Texture, int Frame = 0, int Index = 0)
{
    switch (NODE(Entity)->getType())
    {
        case scene::NODE_MESH:
        {
            stcSetMeshSurfaceTexture(MESH(Entity), TEXTURE(Texture), Frame, Index);
        }
        break;
        
        case scene::NODE_BILLBOARD:
        {
            scene::Billboard* Sprite    = BILLBOARD(Entity);
            video::Texture* Tex         = TEXTURE(Texture);
            
            if (Tex)
                stcSetTextureFrame(Tex, Frame);
            
            Sprite->setTexture(Tex);
        }
        break;
    }
}
SPW_PROC(void) spwPaintEntity(LP Entity, LP Brush)
{
    if (Brush > 0 && Brush <= g_BrushList.size())
        spwEntityTexture(Entity, g_BrushList[Brush - 1].Texture);
}
SPW_PROC(void) spwPaintSurface(LP Surface, LP Brush)
{
    if (Brush <= 0 || Brush > g_BrushList.size())
    {
        WrapperError("PaintSurface", "\"Brush\" is invalid");
        return;
    }
    
    std::map<LP, SWrapSurface>::iterator it = g_SurfaceList.find(Surface);
    
    if (it == g_SurfaceList.end())
    {
        WrapperError("PaintSufrace", "\"Surface\" is invalid");
        return;
    }
    
    it->second.Brush = Brush;
    SWrapSurface SurfaceData = it->second;
    
    stcSetMeshSurfaceTexture(
        MESH(SurfaceData.Mesh), TEXTURE(g_BrushList[Brush - 1].Texture), 0, 0, SurfaceData.SurfaceNr
    );
}

SPW_PROC(LP) spwCreateBrush(int Red = 0, int Green = 0, int Blue = 0)
{
    SWrapBrush BrushData;
    {
        BrushData.Texture = 0;
    }
    g_BrushList.push_back(BrushData);
    
    return g_BrushList.size();
}
SPW_PROC(LP) spwLoadBrush(STR File, int Flags = 9, float Width = 1.0, float Height = 1.0)
{
    SWrapBrush BrushData;
    {
        BrushData.Texture = spwLoadTexture(File, Flags);
    }
    g_BrushList.push_back(BrushData);
    
    return g_BrushList.size();
}
SPW_PROC(void) spwFreeBrush(LP Brush)
{
    // !TODO
}
SPW_PROC(void) spwBrushTexture(LP Brush, LP Texture, int Frame = 0, int Index = 0)
{
    if (Brush > 0 && Brush <= g_BrushList.size())
        g_BrushList[Brush - 1].Texture = Texture;
}
SPW_PROC(void) spwPaintMesh(LP Mesh, LP Brush)
{
    if (Brush > 0 && Brush <= g_BrushList.size())
        spwEntityTexture(Mesh, g_BrushList[Brush - 1].Texture);
}
SPW_PROC(void) spwBrushShininess(LP Brush, float Shininess)
{
    // !TODO
}

SPW_PROC(void) spwMidHandle(LP Handle, bool Enable = true)
{
    g_ImageList[Handle].isMidHandle = Enable;
}
SPW_PROC(LP) spwLoadImage(STR File)
{
    LP Handle = spwLoadTexture(File);
    if (g_AutoMidHandle)
        spwMidHandle(Handle);
    TEXTURE(Handle)->setColorKey(video::color(0, 0, 0, 0));
    TEXTURE(Handle)->setFilter(video::FILTER_LINEAR);
    return Handle;
}
SPW_PROC(LP) spwLoadAnimImage(
    STR File, int FrameWidth, int FrameHeight, int FirstFrame, int FrameCount, int Flags = 9)
{
    LP Handle = spwLoadAnimTexture(File, Flags, FrameWidth, FrameHeight, FirstFrame, FrameCount);
    if (g_AutoMidHandle)
        spwMidHandle(Handle);
    return Handle;
}
SPW_PROC(LP) spwCreateImage(int Width, int Height, int Frames = 0)
{
    return spwCreateTexture(Width, Height, 9, Frames);
}
SPW_PROC(LP) spwCopyImage(LP Handle)
{
    return spwCopyTexture_ex(Handle);
}
SPW_PROC(int) spwImageWidth(LP Handle)
{
    return TEXTURE(Handle)->getSize().Width;
}
SPW_PROC(int) spwImageHeight(LP Handle)
{
    return TEXTURE(Handle)->getSize().Height;
}
SPW_PROC(void) spwMaskImage(LP Handle, int Red, int Green, int Blue, int Alpha = 0)
{
    TEXTURE(Handle)->setColorKey(video::color(Red, Green, Blue, Alpha));
}
SPW_PROC(void) spwScaleImage(LP Handle, float Width, float Height)
{
    const dim::size2di Size(TEXTURE(Handle)->getSize());
    TEXTURE(Handle)->setSize(
        dim::size2di(s32(Width * Size.Width), s32(Height * Size.Height))
    );
}
SPW_PROC(void) spwResizeImage(LP Handle, int Width, int Height)
{
    TEXTURE(Handle)->setSize(dim::size2di(Width, Height));
}
SPW_PROC(void) spwRotateImage(LP Handle, float Angle)
{
    g_ImageList[Handle].Rotation += Angle;
}
SPW_PROC(void) spwAutoMidHandle(bool Enable)
{
    g_AutoMidHandle = Enable;
}
SPW_PROC(void) spwGrabImage(LP Handle, int X, int Y)
{
    spwCopyRect(
        X, Y, spwImageWidth(Handle), spwImageHeight(Handle), 0, 0, CAST(g_RenderSystem->getRenderTarget()), Handle
    );
}

SPW_PROC(int) spwTextureWidth(LP Handle)
{
    return TEXTURE(Handle)->getSize().Width;
}
SPW_PROC(int) spwTextureHeight(LP Handle)
{
    return TEXTURE(Handle)->getSize().Height;
}

SPW_PROC(void) spwDrawImage(LP Handle, int X, int Y, int Frame = 0)
{
    video::Texture* TexObject = TEXTURE(Handle);
    SWrapImage ImageObject = g_ImageList[Handle];
    
    stcSetTextureFrame(TexObject, Frame);
    
    g_RenderSystem->beginDrawing2D();
    
    dim::point2di Pos(X, Y);
    if (ImageObject.isMidHandle)
    {
        Pos.X -= TexObject->getSize().Width/2;
        Pos.Y -= TexObject->getSize().Height/2;
    }
    
    if (math::Equal(ImageObject.Rotation, 0.0f))
        g_RenderSystem->draw2DImage(TexObject, g_Origin2D + Pos, g_Color);
    else
    {
        g_RenderSystem->draw2DImage(
            TexObject, dim::rect2di(Pos.X, Pos.Y, TexObject->getSize().Width, TexObject->getSize().Height), dim::rect2df(0, 0, 1, 1),
            true, ImageObject.Rotation, (ImageObject.isMidHandle ? 0.5f : 0.0f)
        );
    }
    
    g_RenderSystem->endDrawing2D();
}
SPW_PROC(void) spwDrawImageRect(
    LP Handle, int X, int Y, int RectX, int RectY, int RectWidth, int RectHeight, int Frame = 0)
{
    video::Texture* Tex = TEXTURE(Handle);
    stcSetTextureFrame(Tex, Frame);
    const dim::size2di Size(Tex->getSize());
    
    g_RenderSystem->beginDrawing2D();
    g_RenderSystem->draw2DImage(
        Tex,
        dim::rect2di(g_Origin2D.X + X, g_Origin2D.Y + Y, Size.Width, Size.Height),
        dim::rect2df(
            (f32)RectX / Size.Width, (f32)RectY / Size.Height,
            (f32)(RectX + RectWidth) / Size.Width, (f32)(RectY + RectHeight) / Size.Height
        ),
        g_Color
    );
    g_RenderSystem->endDrawing2D();
}
SPW_PROC(void) spwDrawImageStretch(
    LP Handle, int X, int Y, int Width, int Height,
    float MapX1 = 0.0, float MapY1 = 0.0, float MapX2 = 1.0, float MapY2 = 1.0, int Frame = 0)
{
    stcSetTextureFrame(TEXTURE(Handle), Frame);
    
    X += g_Origin2D.X;
    Y += g_Origin2D.Y;
    
    g_RenderSystem->beginDrawing2D();
    
    if (g_EnableMultiColor)
    {
        g_RenderSystem->draw2DImage(
            TEXTURE(Handle),
            dim::point2di(X, Y), dim::point2di(X + Width, Y), dim::point2di(X + Width, Y + Height), dim::point2di(X, Y + Height),
            dim::point2df(MapX1, MapY1), dim::point2df(MapX2, MapY1), dim::point2df(MapX2, MapY2), dim::point2df(MapX1, MapY2),
            g_MultiColor[0], g_MultiColor[1], g_MultiColor[2], g_MultiColor[3]
        );
    }
    else
    {
        g_RenderSystem->draw2DImage(
            TEXTURE(Handle), dim::rect2di(X, Y, Width, Height),
            dim::rect2df(MapX1, MapY1, MapX2, MapY2), g_Color
        );
    }
    
    g_RenderSystem->endDrawing2D();
}
SPW_PROC(void) spwDrawImageRotate(LP Handle, int X, int Y, float Angle, float Radius, int Frame = 0)
{
    stcSetTextureFrame(TEXTURE(Handle), Frame);
    
    X += g_Origin2D.X;
    Y += g_Origin2D.Y;
    
    g_RenderSystem->beginDrawing2D();
    
    if (g_EnableMultiColor)
    {
        g_RenderSystem->draw2DImage(
            TEXTURE(Handle), dim::rect2di(X - Radius, Y - Radius, Radius*2, Radius*2), dim::rect2df(0.0f, 1.0f, 1.0f, 0.0f),
            Angle, dim::point2df(0.5f), g_MultiColor[0], g_MultiColor[1], g_MultiColor[2], g_MultiColor[3]
        );
    }
    else
        g_RenderSystem->draw2DImage(TEXTURE(Handle), dim::point2di(X, Y), Angle, Radius, g_Color);
    
    g_RenderSystem->endDrawing2D();
}
SPW_PROC(void) spwTileImage(LP Handle, int X, int Y, int Frame = 0)
{
    if (!Handle)
    {
        WrapperError("TileImage", "\"Handle\" is invalid");
        return;
    }
    
    dim::rect2df Clipping;
    
    Clipping.Left   = (f32)X / g_ScreenSize.Width;
    Clipping.Top    = (f32)Y / g_ScreenSize.Height;
    Clipping.Right  = Clipping.Left + (f32)(TEXTURE(Handle)->getSize().Width) / g_ScreenSize.Width;
    Clipping.Bottom = Clipping.Top + (f32)(TEXTURE(Handle)->getSize().Height) / g_ScreenSize.Height;
    
    g_RenderSystem->beginDrawing2D();
    
    g_RenderSystem->draw2DImage(
        TEXTURE(Handle), dim::rect2di(0, 0, g_ScreenSize.Width, g_ScreenSize.Height), Clipping
    );
    
    g_RenderSystem->endDrawing2D();
}

SPW_PROC(void) spwDrawBlock(LP Handle, int X, int Y, int Frame = 0)
{
    g_RenderSystem->beginDrawing2D();
    g_RenderSystem->setRenderState(video::RENDER_BLEND, false);
    g_RenderSystem->draw2DImage(TEXTURE(Handle), g_Origin2D + dim::point2di(X, Y), g_Color);
    g_RenderSystem->setRenderState(video::RENDER_BLEND, true);
    g_RenderSystem->endDrawing2D();
}

SPW_PROC(void) spwCls()
{
    g_RenderSystem->clearBuffers();
}
SPW_PROC(void) spwClsColor(int Red, int Green, int Blue, int Alpha = 255)
{
    g_RenderSystem->setClearColor(video::color(Red, Green, Blue, Alpha));
}
SPW_PROC(void) spwMaskColor(bool Red, bool Green, bool Blue, bool Alpha = true)
{
    g_RenderSystem->setColorMask(Red, Green, Blue, Alpha);
}
SPW_PROC(void) spwCameraClsColor(LP Camera, int Red, int Green, int Blue)
{
    g_CameraList[Camera].ClearColor = video::color(Red, Green, Blue);
}
SPW_PROC(void) spwCameraClsMode(LP Camera, bool ClsColor, bool ClsZBuffer)
{
    int Flags = 0;
    
    if (ClsColor)
        Flags |= video::BUFFER_COLOR;
    if (ClsZBuffer)
        Flags |= video::BUFFER_DEPTH;
    
    g_CameraList[Camera].ClearFlags = Flags;
}
SPW_PROC(void) spwCameraFogColor(LP Camera, int Red, int Green, int Blue)
{
    g_CameraList[Camera].FogColor = video::color(Red, Green, Blue);
}
SPW_PROC(void) spwCameraFogMode(LP Camera, int Mode)
{
    g_CameraList[Camera].FogMode = (video::EFogTypes)Mode;
}
SPW_PROC(void) spwCameraFogRange(LP Camera, float Near, float Far, float FallOff = 0.1)
{
    g_CameraList[Camera].FogFallOff     = FallOff;
    g_CameraList[Camera].FogRangeNear   = Near;
    g_CameraList[Camera].FogRangeFar    = Far;
}

SPW_PROC(void) spwHWMultiTex(bool Enable)
{
    // do nothing
}

SPW_PROC(LP) spwTextureBuffer(LP Texture, int Frame = 0)
{
    if (Texture && TEXTURE(Texture)->getAnimation())
        TEXTURE(Texture)->setAnimFrame(Frame);
    return Texture;
}
SPW_PROC(LP) spwImageBuffer(LP Image, int Frame = 0)
{
    if (Image && TEXTURE(Image)->getAnimation())
        TEXTURE(Image)->setAnimFrame(Frame);
    return Image;
}
SPW_PROC(LP) spwFrontBuffer()
{
    return 0;
}
SPW_PROC(LP) spwBackBuffer()
{
    return 0;
}
SPW_PROC(void) spwSetBuffer(LP Buffer)
{
    video::Texture* Obj = TEXTURE(Buffer);
    
    if (Obj && !Obj->getRenderTarget())
        Obj->setRenderTarget(true);
    
    g_RenderSystem->setRenderTarget(Obj);
    
    if (Buffer)
        g_RenderSystem->clearBuffers();
}

SPW_PROC(LP) spwLoadFont(
    STR FontName, int Height = 0, bool Bold = false, bool Italic = false, bool Underlined = false, bool StrikeOut = false)
{
    int Flags = 0;
    
    if (Bold)
        Flags |= video::FONT_BOLD;
    if (Italic)
        Flags |= video::FONT_ITALIC;
    if (Underlined)
        Flags |= video::FONT_UNDERLINED;
    if (StrikeOut)
        Flags |= video::FONT_STRIKEOUT;
    
    return CAST(g_RenderSystem->createFont(FontName, Height, Flags));
}
SPW_PROC(void) spwFreeFont(LP Font)
{
    g_RenderSystem->deleteFont(FONT(Font));
}
SPW_PROC(void) spwSetFont(LP Font)
{
    if (Font)
        g_Font = FONT(Font);
    else
        g_Font = g_StdFont;
}
SPW_PROC(int) spwStringWidth(STR String)
{
    return g_Font->getStringSize(String).Width;
}
SPW_PROC(int) spwStringHeight(STR String)
{
    return g_Font->getStringSize(String).Height;
}
SPW_PROC(int) spwFontWidth()
{
    return g_Font->getSize().Width;//g_Font->getStringSize("A").Width;
}
SPW_PROC(int) spwFontHeight()
{
    return g_Font->getSize().Height;//g_Font->getStringSize("A").Height;
}

SPW_PROC(void) spwText(int X, int Y, STR String, bool CenterX = false, bool CenterY = false)
{
    if (CenterX)
        X -= g_Font->getStringSize(String).Width/2;
    if (CenterY)
        Y -= g_Font->getStringSize(String).Height/2;
    
    g_RenderSystem->beginDrawing2D();
    g_RenderSystem->draw2DText(g_Font, g_Origin2D + dim::point2di(X, Y), String, g_Color);
    g_RenderSystem->endDrawing2D();
}

SPW_PROC(LP) spwOpenMovie(STR File, int Resolution = 512, bool Play = true)
{
    SWrapMovie MovieData;
    {
        MovieData.Movie     = g_RenderSystem->loadMovie(File, Resolution);
        MovieData.Sound     = g_SoundDevice->loadSound(File);
        MovieData.Texture   = g_RenderSystem->createTexture(dim::size2di(Resolution));
    }
    g_MovieList[CAST(MovieData.Movie)] = MovieData;
    
    if (Play && MovieData.Movie && MovieData.Sound)
    {
        MovieData.Movie->play();
        MovieData.Sound->play();
    }
    
    return CAST(MovieData.Movie);
}
SPW_PROC(void) spwCloseMovie(LP Movie)
{
    if (Movie)
    {
        SWrapMovie MovieData = g_MovieList[Movie];
        
        g_RenderSystem->deleteTexture(MovieData.Texture);
        g_RenderSystem->deleteMovie(MovieData.Movie);
        g_SoundDevice->deleteSound(MovieData.Sound);
    }
}
SPW_PROC(void) spwDrawMovie(LP Movie, int X, int Y, int Width = -1, int Height = -1)
{
    SWrapMovie MovieData = g_MovieList[Movie];
    
    if (Movie && MovieData.Movie && MovieData.Texture)
    {
        MovieData.Movie->renderTexture(MovieData.Texture);
        
        if (Width == -1)
            Width = MovieData.Movie->getSize().Width;
        if (Height == -1)
            Height = MovieData.Movie->getSize().Height;
        
        g_RenderSystem->beginDrawing2D();
        g_RenderSystem->draw2DImage(MovieData.Texture, dim::rect2di(X, Y, Width, Height), dim::rect2df(0, 1, 1, 0));
        g_RenderSystem->endDrawing2D();
    }
}
SPW_PROC(int) spwMovieWidth(LP Movie)
{
    SWrapMovie MovieData = g_MovieList[Movie];
    return (MovieData.Movie ? MovieData.Movie->getSize().Width : 0);
}
SPW_PROC(int) spwMovieHeight(LP Movie)
{
    SWrapMovie MovieData = g_MovieList[Movie];
    return (MovieData.Movie ? MovieData.Movie->getSize().Height : 0);
}
SPW_PROC(bool) spwMoviePlaying(LP Movie)
{
    SWrapMovie MovieData = g_MovieList[Movie];
    return (MovieData.Movie ? !MovieData.Movie->finish() : 0);
}
SPW_PROC(LP) spwMovieSound(LP Movie)
{
    SWrapMovie MovieData = g_MovieList[Movie];
    return CAST(MovieData.Sound);
}

SPW_PROC(void) spwMipMapFilter(int Mode, int Anisotropy = 2)
{
    switch (Mode)
    {
        case 0: // Bi-linear filter
            g_RenderSystem->setTextureGenFlags(video::TEXGEN_FILTER, video::FILTER_BILINEAR); break;
        case 1: // Tri-linear filter
            g_RenderSystem->setTextureGenFlags(video::TEXGEN_FILTER, video::FILTER_TRILINEAR); break;
        case 2: // Anisotropic filter
            g_RenderSystem->setTextureGenFlags(video::TEXGEN_FILTER, video::FILTER_ANISOTROPIC);
            g_RenderSystem->setTextureGenFlags(video::TEXGEN_ANISOTROPY, Anisotropy);
            break;
    }
}



// ================================================================================
