bit:                      100
bool:                     110
int:                      120
uint:                     130
float:                    140
double:                   150
long double:              160
const:                    170
angle:                    180
complex:                  190

-------------------------------

bit[n]:                   200
int[n]:                   210
uint[n]:                  220
float[n]:                 230
angle[n]:                 240
complex[n]:               250

-------------------------------

sin:                      300
cos:                      300
tan:                      300
asin:                     300
acos:                     300
atan:                     300
log:                      300
exp:                      300
sqrt:                     300
rotl:                     310
rotr:                     310
popcount:                 320
sizeof:                   330
logical not:              340

-------------------------------

binary ops:
-----------

'+':                      400
'-':                      410
'*':                      420
'/':                      430
'%':                      440
'=':                      450
'==':                     460
'!=':                     470
'&&':                     480
'||':                     490
'<':                      500
'>':                      510
'^':                      520
'&':                      530
'|':                      540
'~':                      550
'<<':                     560
'>>':                     570
'+=':                     580
'-=':                     590
'*=':                     600
'/=':                     610
'%=':                     620
'<=':                     630
'>=':                     640
'^=':                     650
'&=':                     660
'|=':                     670
'<<=':                    680
'>>=':                    690

-------------------------------

unary (logical) ops:
--------------------

'!':                       340

unary (arithmetic) ops:
-----------------------

'-':                       350
'+':                       350

-------------------------------

int/uint literal:          800
float literal:             805
int[n]/uint[n] literal:    810
float[n] literal:          815
string literal:            820

-------------------------------

named type decls:          850

-------------------------------

angle:                    1000
angle[n]:                 1010
qubit:                    1100
qubit[n]:                 1110
duration:                 1200
durationof:               1250
delay:                    1300
stretch:                  1350
box:                      1400
boxas:                    1410
boxto:                    1420
gate:                     1430
defcal:                   1440
defcal grammar:           1445
barrier:                  1450
measure:                  1460
reset:                    1480

-------------------------------

identifier:               1500
indexed-supscript-expr:   1520
indexed-subscript-list:   1530
indexed-identifier:       1550
bound-qubit:              1610
indexed-bound-qubit:      1615
unbound-qubit:            1620
indexed-unbound-qubit:    1625
complex-creal:            1630
complex-cimag:            1631
openpulse-frame-phase:    1640
openpulse-frame-freq:     1641
openpulse-frame-time:     1642

-------------------------------

openpulse frame:          1700
openpulse port:           1710
openpulse waveform:       1720
openpulse play:           1730
openpulse calibration:    1740
openpulse declaration:    1750
openpulse statement:      1760

-------------------------------

return:                   2000
result:                   2100
function:                 2300
kernel (extern):          2500
pragma:                   2600
annotation:               2610

-------------------------------

function call expr:       2700

-------------------------------

if/else-if/else:          3000
switch:                   3100
switch/case:              3110
switch/default:           3120
break/continue:           3150
for loop:                 3200
while loop:               3300
do/while loop:            3400

-------------------------------

gate ops:                 3500

-------------------------------

cast expr:                3600
implicit conversion:      3650

-------------------------------

scalar types:             3700

-------------------------------

param type decls:         3800

-------------------------------

for loop range expression: 860

-------------------------------

gphase expression:         870

-------------------------------

complex initializer expr:  880

-------------------------------

cal block:                4000
frame:                    4010
play:                     4020
port:                     4030
waveform:                 4040

-------------------------------

array index subscript:    5000
array[bit, n]:            5050
array[bool, n]:           5060
array[int, n]:            5070
array[uint, n]:           5080
array[float, n]:          5090
array[complex, n]:        5100
array[int[x], n]:         5110
array[uint[x], n]:        5120
array[float[x], n]:       5130
array[complex[x], n]:     5140
array[angle, n]:          5150
array[angle[x], n]:       5160
array[duration, n]:       5170
array[port, n]:           5180
array[frame, n]:          5190
array[qubit, n]:          5200
array[qubit[x], n]:       5210

-------------------------------

line directive:           5300
file directive:           5300

-------------------------------

misc. warnings:           6000
misc. errors:             7000

-------------------------------
