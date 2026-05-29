
Program Main
        Require 'Std' @Version='1.0.0'

        Procedure Entry(c; v) Nothing
                Integer c
                // array of optional strings
                Optional(Char[*])[c] v
        Begin
                For i In (0)..c
                        // if none, exit
                        Std.Io.Write(Except(v[i]), Exit(1))
        End
End
