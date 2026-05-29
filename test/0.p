
Program Main @L'libc'
        Require 'Std' @Version='1.0.0'
        Require 'LibC' @Version='1.0.0'
        Author 'Joshua F.'
        License 'MIT'
        Version '1.0.0'

        Procedure Entry(c; v) Nothing
                Integer c
                // array of optional strings
                Optional(Char[*])[c] v
        Begin
                For i In (0)..c
                        // if none, exit
                        Std.Io.Write(Except(v[i]), Exit(1))
                LibC.printf(C.str("Hello, World!"));
        End
End
