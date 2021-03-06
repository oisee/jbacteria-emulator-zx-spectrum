rmask= [0xff, 0x0f, 0xff, 0x0f,
        0xff, 0x0f, 0x1f, 0xff,
        0x1f, 0x1f, 0x1f, 0xff,
        0xff, 0x0f, 0xff, 0xff];
amp= [0,      0.0137, 0.0205, 0.0291,
      0.0423, 0.0618, 0.0847, 0.1369,
      0.1691, 0.2647, 0.3527, 0.4499,
      0.5704, 0.6873, 0.8482, 1];
function aystep() {
  if( ++envc >= (ayr[11]<<1 | ayr[12]<<9) )
    envc= 0,
    envv= estep();
  if( ++noic >= ayr[6]<<1 )
    noic= 0,
    noiv= noir & 1,
    noir= (noir^noiv*0x24000)>>1;
  return (cstep(0) + cstep(1) + cstep(2)) / 3;
}
function aymute() {
  if( ++envc >= (ayr[11]<<1 | ayr[12]<<9) ){
    envc= 0;
    if( envx >> 4 && ~ay13 & 1 )
      envx= 0,
      ay13^= ay13<<1 & 4;
  }
  if( ++noic >= ayr[6]<<1 )
    noic= 0,
    noiv= noir & 1,
    noir= (noir^noiv*0x24000)>>1;
  if( ++ayr[16] >= (ayr[0] | ayr[1]<<8) )
    ayr[16]= 0,
    tons^= 1;
  if( ++ayr[17] >= (ayr[2] | ayr[3]<<8) )
    ayr[17]= 0,
    tons^= 2;
  if( ++ayr[18] >= (ayr[4] | ayr[5]<<8) )
    ayr[18]= 0,
    tons^= 4;
}
function cstep(ch) {
  if( ++ayr[ch+16] >= (ayr[ch<<1] | ayr[1|ch<<1]<<8) )
    ayr[ch+16]= 0,
    tons^= 1 << ch;
  return  ( ( ayr[7] >> ch   | tons >> ch )
          & ( ayr[7] >> ch+3 | noiv       )
          & 1 )  * amp[ ayr[8+ch] & 0x10
                   ? envv
                   : ayr[8+ch] & 0x0f ];
}
function estep() {
  if( envx >> 4 ){
    if( ay13 & 1 )
      return 7.5*((ay13>>1 ^ ay13) & 2);
    envx= 0;
    ay13^= ay13<<1 & 4;
  }
  return  ay13 & 4
          ? envx++
          : 15 - envx++;
}
function ayw(val) {
  ayr[ay]= val & rmask[ay];
  if( ay==13 )
    envx= 0,
    ay13= val & 8
          ? 1 | val>>1 & 2 | val & 4
          : val;
}
