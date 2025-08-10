; Macro as command name
mcro mov
mcroend

; mcro with invalid number of names
mcro a asd
mcroend

; Mcro with invalid name syntax
mcro 3s
mcroend

; Mcro with invalid name instruction
mcro data
mcroend

; mcro name is entry instruction
mcro entry
mcroend

; Invalid chars after mcroend
mcro a
mcroend  asddas

; Two macros with same name
mcro b
mcroend
mcro b
mcroend

; line length
.string "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdasas"