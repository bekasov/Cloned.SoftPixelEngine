;
; ========== ShaderParser (for the "SoftPixel Engine") - (02/08/2010) ==========
;

#FILE_WRITE_ID  = 0
#INCLUDE_STR    = "#include "

Procedure.s GetIncludeFilename(Line.s)
  
  ; Find C string tokens
  First.l = FindString(Line, Chr(34), 1)
  
  If First = 0
    ProcedureReturn ""
  EndIf
  
  Last.l = FindString(Line, Chr(34), First + 1)
  
  If Last = 0
    ProcedureReturn ""
  EndIf
  
  ; Get include filename
  ProcedureReturn Mid(Line, First + 1, Last - First - 1)
  
EndProcedure

Procedure ParseOpenFile(FilenameSrc.s, FileNr.l)
  
  FileNr + 1
  
  If Not ReadFile(FileNr, FilenameSrc)
    MessageRequester("Error", "Could not open file: " + Chr(34) + FilenameSrc + Chr(34) + "!", 16)
    End
  EndIf
  
  IncludeStrLen.l = Len(#INCLUDE_STR)
  
  While Not Eof(FileNr)
    
    ; Read from current source file
    Line.s = ReadString(FileNr)
    
    ; Check for include file
    If FindString(Line, #INCLUDE_STR, 1) <> 0
      
      ; Append include file
      SubFilenameSrc.s = GetPathPart(FilenameSrc) + GetIncludeFilename(Line)
      
      ParseOpenFile(SubFilenameSrc, FileNr)
      
      Continue  
      
    EndIf
    
    ; Append C string tokens
    Line = ReplaceString(Line, Chr(9), "    ")
    Line = ReplaceString(Line, "\", "\\")
    Line = ReplaceString(Line, Chr(34), "\" + Chr(34))
    Line = Chr(34) + Line + "\n" + Chr(34)
    
    ; Write to destination file
    WriteStringN(#FILE_WRITE_ID, Line)
    
  Wend
  
  CloseFile(FileNr)
  
EndProcedure

Procedure ParseFile(FilenameSrc.s, FilenameDest.s)
  
  If Not CreateFile(0, FilenameDest)
    MessageRequester("Error", "Could not create file: " + Chr(34) + FilenameDest + Chr(34) + "!", 16)
    End
  EndIf
  
  ParseOpenFile(FilenameSrc, 0)
  
  CloseFile(0)
  
EndProcedure


#DEST_PATH = "./"

#DR_PATH = "../RenderSystem/DeferredRenderer/"

ParseFile(#DR_PATH + "spGBufferShader.cg",                #DEST_PATH + "spGBufferShaderStr.cg")
ParseFile(#DR_PATH + "spDeferredShader.cg",               #DEST_PATH + "spDeferredShaderStr.cg")
ParseFile(#DR_PATH + "spShadowShader.cg",                 #DEST_PATH + "spShadowShaderStr.cg")

ParseFile(#DR_PATH + "spGBufferShader.hlsl",              #DEST_PATH + "spGBufferShaderStr.hlsl")
ParseFile(#DR_PATH + "spGBufferShader.glvert",            #DEST_PATH + "spGBufferShaderStr.glvert")
ParseFile(#DR_PATH + "spGBufferShader.glfrag",            #DEST_PATH + "spGBufferShaderStr.glfrag")

ParseFile(#DR_PATH + "spDeferredShader.hlsl",             #DEST_PATH + "spDeferredShaderStr.hlsl")
ParseFile(#DR_PATH + "spDeferredShader.glvert",           #DEST_PATH + "spDeferredShaderStr.glvert")
ParseFile(#DR_PATH + "spDeferredShader.glfrag",           #DEST_PATH + "spDeferredShaderStr.glfrag")
ParseFile(#DR_PATH + "spDeferredShaderLowResVPL.glfrag",  #DEST_PATH + "spDeferredShaderLowResVPLStr.glfrag")

ParseFile(#DR_PATH + "spDebugVPL.glvert",                 #DEST_PATH + "spDebugVPLStr.glvert")
ParseFile(#DR_PATH + "spDebugVPL.glfrag",                 #DEST_PATH + "spDebugVPLStr.glfrag")


#PP_PATH = "../RenderSystem/PostProcessing/"

ParseFile(#PP_PATH + "spBloomFilter.cg",                  #DEST_PATH + "spBloomFilterStr.cg")
ParseFile(#PP_PATH + "spBloomFilter.glvert",              #DEST_PATH + "spBloomFilterStr.glvert")
ParseFile(#PP_PATH + "spBloomFilter.glfrag",              #DEST_PATH + "spBloomFilterStr.glfrag")


#DX11_PATH = "../RenderSystem/Direct3D11/"

ParseFile(#DX11_PATH + "spDefaultShader.hlsl",            #DEST_PATH + "spDefaultShaderStr.hlsl")
ParseFile(#DX11_PATH + "spDefaultDrawingShader.hlsl",     #DEST_PATH + "spDefaultDrawingShaderStr.hlsl")


#RES_PATH = "../RenderSystem/"

ParseFile(#RES_PATH + "spShaderCore.cg",                  #DEST_PATH + "spShaderCoreStr.cg")
ParseFile(#RES_PATH + "spShaderCore.hlsl3",               #DEST_PATH + "spShaderCoreStr.hlsl3")
ParseFile(#RES_PATH + "spShaderCore.hlsl5",               #DEST_PATH + "spShaderCoreStr.hlsl5")
ParseFile(#RES_PATH + "spShaderCore.glsl",                #DEST_PATH + "spShaderCoreStr.glsl")


MessageRequester("ShaderParser", "Parsing the shader has been completed successful", 64)

; IDE Options = PureBasic 4.50 (Windows - x64)
; CursorPosition = 100
; FirstLine = 69
; Folding = -