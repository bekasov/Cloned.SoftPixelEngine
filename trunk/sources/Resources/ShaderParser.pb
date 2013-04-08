;
; ========== ShaderParser (for the "SoftPixel Engine") - (02/08/2010) ==========
;

Procedure ParseFile(FilenameSrc.s, FilenameDest.s)
  
  If Not ReadFile(0, FilenameSrc)
    MessageRequester("Error", "Could not open file: " + Chr(34) + FilenameSrc + Chr(34) + "!", 16)
    End
  EndIf
  
  If Not CreateFile(1, FilenameDest)
    MessageRequester("Error", "Could not create file: " + Chr(34) + FilenameDest + Chr(34) + "!", 16)
    End
  EndIf
  
  While Not Eof(0)
    
    Line.s = ReadString(0)
    
    Line = ReplaceString(Line, Chr(9), "    ")
    Line = ReplaceString(Line, "\", "\\")
    Line = ReplaceString(Line, Chr(34), "\" + Chr(34))
    Line = Chr(34) + Line + "\n" + Chr(34)
    
    WriteStringN(1, Line)
    
  Wend
  
  CloseFile(0)
  CloseFile(1)
  
EndProcedure

;ParseFile("D3D11DefaultBasicShader3D(SM4)_RAW.h", "D3D11DefaultBasicShader3D(SM4).h")
;ParseFile("D3D11DefaultBasicShader2D(SM4)_RAW.h", "D3D11DefaultBasicShader2D(SM4).h")


#DR_PATH = "../RenderSystem/DeferredRenderer/"
#PP_PATH = "../RenderSystem/PostProcessing/"

ParseFile(#DR_PATH + "spGBufferShaderMain.shader", #DR_PATH + "spGBufferShaderMainStr.shader")
ParseFile(#DR_PATH + "spDeferredShaderProcs.shader", #DR_PATH + "spDeferredShaderProcsStr.shader")

ParseFile(#DR_PATH + "spGBufferShader.cg", #DR_PATH + "spGBufferShaderStr.cg")
ParseFile(#DR_PATH + "spDeferredShader.cg", #DR_PATH + "spDeferredShaderStr.cg")
ParseFile(#PP_PATH + "spBloomFilter.cg", #PP_PATH + "spBloomFilterStr.cg")
ParseFile(#DR_PATH + "spShadowShader.cg", #DR_PATH + "spShadowShaderStr.cg")

ParseFile(#DR_PATH + "spGBufferShader.glvert", #DR_PATH + "spGBufferShaderStr.glvert")
ParseFile(#DR_PATH + "spGBufferShaderHeader.glfrag", #DR_PATH + "spGBufferShaderHeaderStr.glfrag")
ParseFile(#DR_PATH + "spGBufferShaderBody.glfrag", #DR_PATH + "spGBufferShaderBodyStr.glfrag")

ParseFile(#DR_PATH + "spDeferredShader.glvert", #DR_PATH + "spDeferredShaderStr.glvert")
ParseFile(#DR_PATH + "spDeferredShaderHeader.glfrag", #DR_PATH + "spDeferredShaderHeaderStr.glfrag")
ParseFile(#DR_PATH + "spDeferredShaderBody.glfrag", #DR_PATH + "spDeferredShaderBodyStr.glfrag")

ParseFile(#DR_PATH + "spDebugVPL.glvert", #DR_PATH + "spDebugVPLStr.glvert")
ParseFile(#DR_PATH + "spDebugVPL.glfrag", #DR_PATH + "spDebugVPLStr.glfrag")

ParseFile(#PP_PATH + "spBloomFilter.glvert", #PP_PATH + "spBloomFilterStr.glvert")
ParseFile(#PP_PATH + "spBloomFilter.glfrag", #PP_PATH + "spBloomFilterStr.glfrag")


#DX11_PATH = "../RenderSystem/Direct3D11/"

ParseFile(#DX11_PATH + "spDefaultShader.hlsl", #DX11_PATH + "spDefaultShaderStr.hlsl")
ParseFile(#DX11_PATH + "spDefaultDrawingShader.hlsl", #DX11_PATH + "spDefaultDrawingShaderStr.hlsl")


#RES_PATH = "../RenderSystem/"

ParseFile(#RES_PATH + "spShaderCore.cg", #RES_PATH + "spShaderCoreStr.cg")
ParseFile(#RES_PATH + "spShaderCore.hlsl", #RES_PATH + "spShaderCoreStr.hlsl")
ParseFile(#RES_PATH + "spShaderCore.glsl", #RES_PATH + "spShaderCoreStr.glsl")


MessageRequester("ShaderParser", "Parsing the shader has been completed successful", 64)

; IDE Options = PureBasic 4.50 (Windows - x64)
; CursorPosition = 59
; FirstLine = 18
; Folding = -