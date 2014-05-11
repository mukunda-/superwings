Attribute VB_Name = "modmain"
Option Explicit

Private lscript() As levelcmd
Private ncmd As Long

Private scripts() As aiscript
Private nscripts As Long

Dim line_number As String

Private Const LVLINSTR_NULL = 0
Private Const LVLINSTR_MESSAGE = 1
Private Const LVLINSTR_SPAWN = 2
Private Const LVLINSTR_END = 3
Private Const LVLINSTR_SPEED = 4

Private Const OBJINSTR_MOVEXY = &H1&
Private Const OBJINSTR_MOVE = &H2&
Private Const OBJINSTR_IDLE = &H3&
Private Const OBJINSTR_FIREP = &H4&
Private Const OBJINSTR_FIREA = &H5&
Private Const OBJINSTR_FIREM = &H6&
Private Const OBJINSTR_AIMP = &H7&
Private Const OBJINSTR_AIMA = &H8&
Private Const OBJINSTR_AIMM = &H9&
Private Const OBJINSTR_MOVEXA = &HA&
Private Const OBJINSTR_MOVEYA = &HB&
Private Const OBJINSTR_MOVEXYA = &HC&
Private Const OBJINSTR_HALT = &HD&
Private Const OBJINSTR_REMOVE = &HE&
Private Const OBJINSTR_ITEM = &HF&

Private Type levelcmd
    delta As Byte
    cmd As Byte
    pa As Integer
    pb As Integer
    pc As Integer
    pd As Integer
    pe As Integer
    pf As Integer
    ps As String
End Type

Private Type aicmd
    delta As Byte
    cmd As Byte
    pa  As Integer
    pb As Integer
    pc As Integer
    pd As Integer
    pe As Integer
    pf As Integer
    pg As Integer
End Type

Private Type aiscript
    name As String
    commands() As aicmd
    ncommands As Long
End Type

Sub Main()
    Dim input_file As String
    Dim output_file As String
    Dim str As String
    input_file = Split(Command$, " ")(0)
    output_file = Split(Command$, " ")(1) & "b"
    If Dir$(output_file) <> "" Then Kill output_file
  '  input_file = App.Path & "\script2.txt"
'    output_file = App.Path & "\scriptout.a"
    
    Dim cscript As Long
    cscript = -1
    
    Dim nextdelta As Long
    Dim deltacounter As Long
    ' pass1, find scripts
    Open input_file For Input As #1
        line_number = 0
        Do While Not EOF(1)
            Line Input #1, str
            line_number = line_number + 1
            If Not str = "" Then
                If Mid$(str, 1, 1) = "#" Then
                    ' comment
                Else
                    
                    If Mid$(str, 1, 1) = "@" Then
                        'we found a script
                        cscript = addscript(Mid$(str, 2))
                        deltacounter = 0
                        nextdelta = 0
                    Else
                        If Not cscript = -1 Then
                            If Mid$(str, 1, 1) = "~" Then
                                ' delta time
                                nextdelta = Mid$(str, 2)
                            Else
                                ' command!
                                addcmd scripts(cscript)
                                With scripts(cscript).commands(scripts(cscript).ncommands - 1)
                                    .delta = nextdelta - deltacounter
                                    deltacounter = nextdelta
                                    .cmd = objinstruction(parseinstr(str))
                                    ' get parameters
                                    readparameters scripts(cscript).commands(scripts(cscript).ncommands - 1), str
                                End With
                            End If
                        Else
                            ' level command
                        End If
                    End If
                End If
            End If
        Loop
    Close #1
    nextdelta = 0
    deltacounter = 0
    Dim hascmd As Boolean
    
    Open input_file For Input As #1
    
        line_number = 0
        Do While Not EOF(1)
            
            Line Input #1, str
            line_number = line_number + 1
            If Not str = "" Then
                If Mid$(str, 1, 1) = "#" Then
                    
                Else
                    If Mid$(str, 1, 1) = "@" Then
                        Exit Do
                    Else
                        If Left$(str, 1) = "~" Then
                            ' delta command
                            nextdelta = Mid$(str, 2)
                        Else
                            'level command
                                
                            addlevelcmd
                            lscript(ncmd - 1).delta = nextdelta - deltacounter
                            deltacounter = nextdelta
                            lscript(ncmd - 1).cmd = lvlinstruction(parseinstr(str))
                            readlvlparameters lscript(ncmd - 1), str
                        End If
                    End If
                End If
            End If
        Loop
    Close #1
    writeoutput output_file
    
End Sub

Private Sub writeoutput(file As String)
    Open file For Binary As #1
        Dim scriptOffsets() As Long
        ReDim scriptOffsets(nscripts - 1)
        Dim int_out As Integer
        Dim byteOut As Byte
        int_out = nscripts
        ' write number of scripts
        Put #1, , int_out
        Dim x As Long
        
        ' add some space for pointers
        For x = 0 To nscripts - 1
            Put #1, , int_out
        Next
        
        ' write level script
        For x = 0 To ncmd - 1
            byteOut = lscript(x).delta
            Put #1, , byteOut
            byteOut = lscript(x).cmd
            Put #1, , byteOut
            writelvlcmdparams lscript(x)
        Next
        
        ' write scripts
        For x = 0 To nscripts - 1
            scriptOffsets(x) = Seek(1) - 1
            writeobjscript scripts(x)
        Next
        
        Seek #1, 3
        For x = 0 To nscripts - 1
            byteOut = scriptOffsets(x) And 255
            Put #1, , byteOut
            byteOut = scriptOffsets(x) \ 256
            Put #1, , byteOut
        Next
    Close #1
    writesource file, changeextension(file, "s")
End Sub

Private Function changeextension(file As String, ext As String)
    If InStr(file, ".") = 0 Then
        changeextension = file & ext
    Else
        changeextension = Left$(file, InStr(file, ".")) & ext
    End If
End Function

Private Sub writesource(inp As String, outp As String)
    Open inp For Binary As #1
        Open outp For Output As #2
            Dim scriptname As String
            scriptname = Mid$(outp, InStrRev(outp, "/") + 1)
            scriptname = Left$(scriptname, InStr(scriptname, ".") - 1)
            Print #2, ".section .rodata"
            Print #2, vbTab & ".global " & scriptname 'levelscript"
            Print #2, scriptname & ":" ' "levelscript:"
            
            Dim strout As String
            strout = vbTab & ".byte" & vbTab
            Dim b As Byte
            Dim c As Boolean
            Do While Not EOF(1)
                Get #1, , b
                strout = strout & "0x" & String(2 - Len(Hex$(b)), "0") & Hex$(b) & ","
                c = True
                If Len(strout) > 85 Then
                    Print #2, Left$(strout, Len(strout) - 1)
                    strout = vbTab & ".byte" & vbTab
                    c = False
                End If
            Loop
            If c Then
                Print #2, Left$(strout, Len(strout) - 1)
            End If
        Close #2
    Close #1
End Sub

Private Sub writeobjscript(s As aiscript)
    Dim x As Long
    For x = 0 To s.ncommands - 1
        writeobjcmd s.commands(x)
    Next
End Sub

Private Sub writeobjcmd(cmd As aicmd)
    'On Error GoTo 1
    Dim byteOut As Byte
    byteOut = cmd.delta
    Put #1, , byteOut
    byteOut = cmd.cmd
    Put #1, , byteOut
    Select Case cmd.cmd
    Case OBJINSTR_MOVEXYA, OBJINSTR_MOVE, OBJINSTR_FIREP, OBJINSTR_AIMP '  2 parameters
        byteOut = cmd.pa
        Put #1, , byteOut
        byteOut = cmd.pb
        Put #1, , byteOut
    Case OBJINSTR_MOVEXA, OBJINSTR_MOVEYA, OBJINSTR_IDLE, OBJINSTR_FIREA, OBJINSTR_AIMA, OBJINSTR_ITEM ' 1 parameter

        byteOut = cmd.pa
        Put #1, , byteOut
    Case OBJINSTR_MOVEXY
        byteOut = unsign16(cmd.pa) And 255
        Put #1, , byteOut
        byteOut = unsign16(cmd.pa) \ 256
        Put #1, , byteOut
        byteOut = unsign16(cmd.pb) And 255
        Put #1, , byteOut
        byteOut = unsign16(cmd.pb) \ 256
        Put #1, , byteOut
    End Select
    
1
    If Err Then
    '    MsgBox "error" & Err.Description & " L" & line_number
    End If
End Sub

Private Sub writelvlcmdparams(cmd As levelcmd)
    Dim byteOut As Byte
    Dim x As Long
    Select Case cmd.cmd
    Case LVLINSTR_MESSAGE
        byteOut = Len(cmd.ps) + 1
        Put #1, , byteOut
        For x = 0 To Len(cmd.ps) - 1
            byteOut = AscB(Mid$(cmd.ps, x + 1, 1))
            Put #1, , byteOut
        Next
        byteOut = 0
        Put #1, , byteOut
    Case LVLINSTR_SPAWN
        byteOut = unsign16(cmd.pa) And 255
        Put #1, , byteOut
        byteOut = unsign16(cmd.pa) \ 256
        Put #1, , byteOut
        byteOut = unsign16(cmd.pb) And 255
        Put #1, , byteOut
        byteOut = unsign16(cmd.pb) \ 256
        Put #1, , byteOut
        byteOut = cmd.pc
        Put #1, , byteOut
        byteOut = cmd.pd
        Put #1, , byteOut
    Case LVLINSTR_SPEED
        byteOut = cmd.pa
        Put #1, , byteOut
        
    End Select
End Sub

Private Function unsign16(ByVal value As Long) As Long

    If value < 0 Then
        unsign16 = 65536 + value
    Else
        unsign16 = value
    End If
End Function

Private Sub addlevelcmd()
    If ncmd = 0 Then
        ReDim lscript(0)
    Else
        ReDim Preserve lscript(ncmd)
    End If
    ncmd = ncmd + 1
End Sub

Private Function findscript(name As String) As Long
    Dim x As Long
    For x = 0 To nscripts - 1
        If scripts(x).name = name Then
            findscript = x
            Exit Function
        End If
    Next
    findscript = -1
End Function

Private Function findobject(name As String) As Long
    Select Case LCase$(name)
    Case "heli"
        findobject = 1
    Case "tank"
        findobject = 2
    Case "bigplane"
        findobject = 3
    Case "jet"
        findobject = 4
    Case "warehouse"
        findobject = 5
    Case "barrel"
        findobject = 6
    Case "barrels"
        findobject = 7
    Case "crate"
        findobject = 8
    Case "crates"
        findobject = 9
    Case "prototype"
        findobject = 10
    Case "bighouse"
        findobject = 11
    Case "bigtank"
        findobject = 12
    Case "tom"
        findobject = 13
    Case "invader"
        findobject = 14
    Case Else
        MsgBox "unknown object """ & name & """ (line" & line_number & ")", vbExclamation
    End Select
End Function

Private Sub readlvlparameters(cmd As levelcmd, inst As String)
    Dim plist() As String
    If InStr(inst, vbTab) = 0 Then
        Exit Sub
    End If
    plist = Split(Mid$(inst, InStr(inst, vbTab) + 1), ",")
    Select Case LCase$(cmd.cmd)
    Case LVLINSTR_MESSAGE
        ' special string case
        cmd.ps = Replace(Mid$(inst, InStr(inst, vbTab) + 1), "\\", vbCr)
    Case LVLINSTR_SPAWN
        cmd.pa = plist(0)
        cmd.pb = plist(1)
        cmd.pc = findobject(plist(2))
        cmd.pd = findscript(plist(3))
        If cmd.pd = -1 Then
            MsgBox "cannot find script """ & plist(3) & """", vbExclamation
        End If
    Case LVLINSTR_END
        'foo
    Case LVLINSTR_SPEED
        cmd.pa = plist(0)
    End Select
End Sub

Private Sub readparameters(cmd As aicmd, inst As String)
    Dim plist() As String
    If InStr(inst, vbTab) = 0 Then
        Exit Sub
    End If
    plist = Split(Mid$(inst, InStr(inst, vbTab) + 1), ",")
    Select Case LCase$(cmd.cmd)
    Case OBJINSTR_MOVEXY
        cmd.pa = plist(0)
        cmd.pb = plist(1)
    Case OBJINSTR_MOVEXA, OBJINSTR_MOVEYA, OBJINSTR_IDLE, OBJINSTR_FIREA, OBJINSTR_AIMA, OBJINSTR_ITEM
        cmd.pa = plist(0)
    Case OBJINSTR_MOVEXYA, OBJINSTR_FIREP, OBJINSTR_AIMP, OBJINSTR_MOVE
        cmd.pa = plist(0)
        cmd.pb = plist(1)
    End Select
End Sub

Private Function lvlinstruction(instruction As String) As Long
    Select Case LCase$(instruction)
    Case "null"
        lvlinstruction = LVLINSTR_NULL
    Case "message"
        lvlinstruction = LVLINSTR_MESSAGE
    Case "spawn"
        lvlinstruction = LVLINSTR_SPAWN
    Case "end"
        lvlinstruction = LVLINSTR_END
    Case "speed"
        lvlinstruction = LVLINSTR_SPEED
    Case Else
        lvlinstruction = LVLINSTR_NULL
        MsgBox "invalid instruction found, line " & line_number & vbCrLf & instruction, vbExclamation
    End Select
End Function

Private Function objinstruction(instruction As String) As Long
    Select Case LCase$(instruction)
    Case "movexy"
        objinstruction = OBJINSTR_MOVEXY
    Case "movexa"
        objinstruction = OBJINSTR_MOVEXA
    Case "moveya"
        objinstruction = OBJINSTR_MOVEYA
    Case "movexya"
        objinstruction = OBJINSTR_MOVEXYA
    Case "move"
        objinstruction = OBJINSTR_MOVE
    Case "idle"
        objinstruction = OBJINSTR_IDLE
    Case "firep"
        objinstruction = OBJINSTR_FIREP
    Case "firea"
        objinstruction = OBJINSTR_FIREA
    Case "firem"
        objinstruction = OBJINSTR_FIREM
    Case "aimp"
        objinstruction = OBJINSTR_AIMP
    Case "aima"
        objinstruction = OBJINSTR_AIMA
    Case "aimm"
        objinstruction = OBJINSTR_AIMM
    Case "halt"
        objinstruction = OBJINSTR_HALT
    Case "remove"
        objinstruction = OBJINSTR_REMOVE
    Case "item"
        objinstruction = OBJINSTR_ITEM
    End Select
End Function

Private Function parseinstr(instruction As String) As String
    If InStr(instruction, vbTab) = 0 Then
        parseinstr = instruction
    Else
        parseinstr = Left$(instruction, InStr(instruction, vbTab) - 1)
    End If
End Function

Private Function addscript(name As String) As Long
    If nscripts = 0 Then
        ReDim scripts(0)
    Else
        ReDim Preserve scripts(nscripts)
    End If
    scripts(nscripts).name = name
    addscript = nscripts
    nscripts = nscripts + 1
End Function

Private Function addcmd(script As aiscript) As Long
    If script.ncommands = 0 Then
        ReDim script.commands(0)
    Else
        ReDim Preserve script.commands(script.ncommands)
    End If
    addcmd = script.ncommands
    script.ncommands = script.ncommands + 1
End Function
