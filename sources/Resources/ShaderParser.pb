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

ParseFile(#DR_PATH + "spDeferredShader.cg", #DR_PATH + "spDeferredShaderStr.h")
ParseFile(#DR_PATH + "spGBufferShader.cg", #DR_PATH + "spGBufferShaderStr.h")
ParseFile(#DR_PATH + "spBloomFilter.cg", #DR_PATH + "spBloomFilterStr.h")
ParseFile(#DR_PATH + "spShadowShader.cg", #DR_PATH + "spShadowShaderStr.h")

MessageRequester("ShaderParser", "Parsing the shader has been completed successful", 64)

; IDE Options = PureBasic 4.50 (Windows - x64)
; CursorPosition = 43
; Folding = -