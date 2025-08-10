.data a ; invalid data type
.data 10000 ; Too big number
.data -10000 ; Too small number
.data 10, ; invalid syntax
.data 123 213 ; Expected comma
.data 234, asd ; Invalid second param

; don't close the string
.string "Sad
; Invalid value
.string 13
.string asd"sad" ; Start with spam letter


.mat[3][] ; Missing row size
.mat[][3] ; Missing col size
.mat[2][2] 1, 2, 3, 4, 5 ; Too much numbers
.mat[2][2] 1, 2  3 ; Missing comma
.mat[2][2] 21313221 ; Huge number
.mat[2][2] -21313221 ; Small number
.mat[2][2] sad ; Invalid number

.entry 32dasdsaas ; Invalid entry name
.extern 32dasdsaas ; Invalid extern name

; Two same entries
a: .data 1
.entry a
.entry a

; Two same externals
.extern a
.extern a

; Missing entry symbol
.entry missing

; Spam letters
Spam: .data 23
.entry Spam asdsad
.extern spanE asdsad
