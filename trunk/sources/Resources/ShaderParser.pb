;
; ========== ShaderParser (for the "SoftPixel Engine") - (02/08/2010) ==========
;

; ========== Macros ==========

#FILE_WRITE_ID  = 0
#INCLUDE_STR    = "#include "
#INCLUDE_BASE   = "softpixelengine"

#DEST_PATH  = "./"
#RES_PATH   = "../RenderSystem/"
#DR_PATH    = "../RenderSystem/DeferredRenderer/"
#PP_PATH    = "../RenderSystem/PostProcessing/"
#DX11_PATH  = "../RenderSystem/Direct3D11/"
#LMG_PATH   = "../Framework/Tools/LightmapGenerator/"


; ========== Functions ==========

Procedure.s GetCoreExtension(Extension.s)
  
  If Extension = "glvert" Or Extension = "glfrag"
    ProcedureReturn "glsl"
  ElseIf Extension = "hlsl"
    ProcedureReturn "hlsl5"
  EndIf
  
  ProcedureReturn Extension
  
EndProcedure


Procedure.s GetIncludeFilename(Line.s)
  
  ; Find C string tokens
  First.l = FindString(Line, Chr(34), 1)
  
  If First = 0
    First = FindString(Line, "<", 1)
    If (First = 0)
      ProcedureReturn ""
    EndIf
  EndIf
  
  Last.l = FindString(Line, Chr(34), First + 1)
  
  If Last = 0
    Last.l = FindString(Line, ">", First + 1)
    If Last = 0
      ProcedureReturn ""
    EndIf
  EndIf
  
  ; Get include filename
  Filename.s = Mid(Line, First + 1, Last - First - 1)
  
  ProcedureReturn Filename
  
EndProcedure


Procedure ParseOpenFile(FileNr.l, FilenameSrc.s, ShaderExtension.s)
  
  FileNr + 1
  
  If Not ReadFile(FileNr, FilenameSrc)
    MessageRequester("File Error", "Could not open file: " + Chr(34) + FilenameSrc + Chr(34) + "!", 16)
    ProcedureReturn
  EndIf
  
  IncludeStrLen.l = Len(#INCLUDE_STR)
  
  DeadBlock.b = #False
  
  While Not Eof(FileNr)
    
    ; Read from current source file
    Line.s = ReadString(FileNr)
    TrimedLine.s = Trim(Line)
    TrimedLine.s = Trim(Line, Chr(9))
    
    ; Search for dead block
    If FindString(Line, "#if 0", 1) <> 0
      DeadBlock = #True
    EndIf
    
    If DeadBlock And FindString(Line, "#endif", 1) <> 0
      DeadBlock = #False
      Continue
    EndIf
    
    ; Skip comment or blank line
    If Len(TrimedLine) = 0 Or Left(TrimedLine, 2) = "//" Or ( Left(TrimedLine, 2) = "/*" And Right(TrimedLine, 2) = "*/" ) Or DeadBlock
      Continue
    EndIf
    
    ; Check for include file
    If FindString(Line, #INCLUDE_STR, 1) <> 0
      
      ; Get include filename
      SubFilenameSrc.s = GetIncludeFilename(Line)
      
      If SubFilenameSrc = #INCLUDE_BASE
        
        ; Append base include file
        CoreExtension.s = GetCoreExtension(ShaderExtension)
        
        If CoreExtension = "cg" Or CoreExtension = "glsl" Or CoreExtension = "hlsl3" Or CoreExtension = "hlsl5"
          ParseOpenFile(FileNr, #RES_PATH + "spShaderCore" + "." + CoreExtension, ShaderExtension)
        Else
          MessageRequester("Format Error", "Unknown shader extension (" + CoreExtension + ") For base include file: " + Chr(34) + SubFilenameSrc + Chr(34), 16)
        EndIf
        
      Else
          
        ; Append include file
        SubFilenameSrc = GetPathPart(FilenameSrc) + SubFilenameSrc
        
        ParseOpenFile(FileNr, SubFilenameSrc, ShaderExtension)
        
      EndIf
      
      Continue  
      
    EndIf
    
    ; Append C string tokens
    Line = ReplaceString(Line, Chr(9), " ")
    Line = ReplaceString(Line, "\", "\\")
    Line = ReplaceString(Line, Chr(34), "\" + Chr(34))
    Line = Chr(34) + Line + "\n" + Chr(34)
    
    ; Write to destination file
    WriteStringN(#FILE_WRITE_ID, Line)
    
  Wend
  
  CloseFile(FileNr)
  
EndProcedure


Procedure ParseFile(PathSrc.s, FilenameSrc.s, ShaderExtension.s)
  
  FilenameDest.s = #DEST_PATH + FilenameSrc + "Str." + ShaderExtension
  
  If Not CreateFile(0, FilenameDest)
    MessageRequester("File Error", "Could not create file: " + Chr(34) + FilenameDest + Chr(34) + "!", 16)
    ProcedureReturn
  EndIf
  
  ParseOpenFile(0, PathSrc + FilenameSrc + "." + ShaderExtension, ShaderExtension)
  
  CloseFile(0)
  
EndProcedure


; ========== Parse files ==========

ParseFile(#RES_PATH, "spShaderCore",                "cg")
ParseFile(#RES_PATH, "spShaderCore",                "hlsl3")
ParseFile(#RES_PATH, "spShaderCore",                "hlsl5")
ParseFile(#RES_PATH, "spShaderCore",                "glsl")


ParseFile(#DR_PATH, "spGBufferShader",              "cg")
ParseFile(#DR_PATH, "spDeferredShader",             "cg")
ParseFile(#DR_PATH, "spShadowShader",               "cg")
ParseFile(#DR_PATH, "spShadowShader",               "hlsl")

ParseFile(#DR_PATH, "spGBufferShader",              "hlsl")
ParseFile(#DR_PATH, "spGBufferShader",              "glvert")
ParseFile(#DR_PATH, "spGBufferShader",              "glfrag")

ParseFile(#DR_PATH, "spDeferredShader",             "hlsl")
ParseFile(#DR_PATH, "spDeferredShader",             "glvert")
ParseFile(#DR_PATH, "spDeferredShader",             "glfrag")
ParseFile(#DR_PATH, "spDeferredShaderLowResVPL",    "glfrag")

ParseFile(#DR_PATH, "spLightGridShader",            "hlsl")

ParseFile(#DR_PATH, "spDebugVPL",                   "glvert")
ParseFile(#DR_PATH, "spDebugVPL",                   "glfrag")


ParseFile(#PP_PATH, "spBloomFilter",                "cg")
ParseFile(#PP_PATH, "spBloomFilter",                "glvert")
ParseFile(#PP_PATH, "spBloomFilter",                "glfrag")


ParseFile(#DX11_PATH, "spDefaultShader",            "hlsl")
ParseFile(#DX11_PATH, "spDefaultDrawingShader",     "hlsl")


ParseFile(#LMG_PATH, "spLightmapGenerationShader",  "hlsl")


MessageRequester("ShaderParser", "Parsing the shader has been completed successful", 64)

; IDE Options = PureBasic 5.11 (Windows - x86)
; CursorPosition = 182
; FirstLine = 154
; Folding = -