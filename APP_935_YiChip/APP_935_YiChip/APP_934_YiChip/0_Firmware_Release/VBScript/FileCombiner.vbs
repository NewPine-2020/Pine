 
 Set objArgs = WScript. Arguments 
 AppPath = "app.bin"
 ProductionStuffPath = "production_stuff.bin"
 ReleasePath = objArgs(0)
 FcErrorCode = 0
 
 bufferAPP = ReadBinary (AppPath)
 bufferProdStuff = ReadBinary (ProductionStuffPath)
 bufferCom = Combine (bufferProdStuff, 0, bufferAPP)
 WriteBinary ReleasePath,bufferCom

Function Combine(Buffer1, Position, Buffer2)
	Dim I, J, K
	I = UBound(Buffer1) + 1
	K = UBound(Buffer2) + 1
	'Wscript.echo now & ", I:" & I
	'ReDim Preserve Buffer1(I + K - 1)
	For J = 0 To K - 1
		Buffer1(Position + J) = Buffer2(J)
	Next
	Combine = Buffer1
End Function

Function ReadBinary(FileName)
  Dim Buf(), I
  With CreateObject("ADODB.Stream")
    .Mode = 3: .Type = 1: .Open: .LoadFromFile FileName
    ReDim Buf(.Size - 1)
    For I = 0 To .Size - 1: Buf(I) = AscB(.Read(1)): Next
    .Close
  End With
  ReadBinary = Buf
End Function

Sub WriteBinary(FileName, Buf)
  Dim I, aBuf, Size, bStream
  Size = UBound(Buf): ReDim aBuf(Size \ 2)
  For I = 0 To Size - 1 Step 2
	  aBuf(I \ 2) = ChrW(Buf(I + 1) * 256 + Buf(I))
	  'Wscript.echo now & ", Buf(I):" & Buf(I)
  Next
  If I = Size Then aBuf(I \ 2) = ChrW(Buf(I))
  aBuf=Join(aBuf, "")
  Set bStream = CreateObject("ADODB.Stream")
  bStream.Type = 1: bStream.Open
  With CreateObject("ADODB.Stream")
    .Type = 2 : .Open: .WriteText aBuf
    .Position = 2: .CopyTo bStream: .Close
  End With
  bStream.SaveToFile FileName, 2: bStream.Close
  Set bStream = Nothing
End Sub





