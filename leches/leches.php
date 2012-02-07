<?
function outbits($val){
  global $inibit;
  for($i= 0; $i<$val; $i++)
    outbit($inibit);
  $inibit^= 1;
}
function outbits_double($val){
  outbits($val);
  outbits($val);
}
function outbit($val){
  global $bytes;
  $bytes.= $val ? ' �' : '� ';
}
function pilot($val){
  global $mhigh;
  while( $val-- )
    outbits_double( 6 << $mhigh );
}
function loadconf($b27){
  global $mhigh;
  outbits( 6 << $mhigh );
  outbits( 14 << $mhigh );
  pilot( 3 );
  outbits_double(1 << $mhigh);
  $c27= 27;
  while( $c27-- ){
    if (($c27==26 || $c27==10) && $mhigh){
      outbits( $b27&0x4000000 ? 4 : 8 );
      outbits( $b27&0x4000000 ? 5 : 9 );
    }
    else
      outbits_double( ($b27&0x4000000 ? 2 : 4) << $mhigh );
    $b27<<= 1;
  }
  outbits(1 << $mhigh);
  outbits((1 << $mhigh)+1);
}
$tabla1= array( array(1,2,2,3), // 0
                array(2,2,3,3), // 1
                array(2,3,3,4), // 2
                array(3,3,4,4), // 3
                array(1,2,3,4), // 4
                array(2,3,4,5), // 5
                array(2,3,4,5), // 6
                array(3,4,5,6), // 7
                array(1,1,2,2));// 8
$tabla2= array( array(1,1,2,2), // 0
                array(1,2,2,3), // 1
                array(2,2,3,3), // 2
                array(2,3,3,4), // 3
                array(1,2,3,4), // 4
                array(1,2,3,4), // 5
                array(2,3,4,5), // 6
                array(2,3,4,5), // 7
                array(1,2,2,3));// 8
$termin= array( array( 21, 22, 23, 24, 23, 24, 25, 26, 13),  // 0 1 2 3 4 5 6 7
                array( 13, 14, 15, 16, 15, 16, 17, 18, 9)); // 0 1 2 3 4 5 6 7
$velo= isset($_SERVER['argv'][2]) ? $_SERVER['argv'][2] : 3;
$mlow= $_SERVER['argv'][3]==24 || $_SERVER['argv'][3]==48 ? 1 : 0;
$mhigh= $_SERVER['argv'][3]==22 || $_SERVER['argv'][3]==24 ? 0 : 1;
if(!$mhigh)
  $velo= 8;
$states= array(array(22050,24000),array(44100,48000)); // 22 24 44 48
$inibit= $_SERVER['argv'][4]==1 ? 1 : 0;
$st= array();
for($i= 0; $i<256; $i++){
  $val= $i >> 6;
  outbits($tabla1[$velo][$val]);
  outbits($tabla2[$velo][$val]);
  $val= $i >> 4 & 3;
  outbits($tabla1[$velo][$val]);
  outbits($tabla2[$velo][$val]);
  $val= $i  >> 2 & 3;
  outbits($tabla1[$velo][$val]);
  outbits($tabla2[$velo][$val]);
  $val= $i  & 3;
  outbits($tabla1[$velo][$val]);
  outbits($tabla2[$velo][$val]);
  $st[$i]= $bytes;
  $bytes= '';
}
echo "empieza\n";
$sna= file_get_contents($_SERVER['argv'][1]);
$r= ord($sna[20]);
$r= (($r&127)-5)&127 | $r&128;
if( strtolower(substr($_SERVER['argv'][1],-3))=='tap' ){
  $nocheck= $_SERVER['argv'][5]=='nocheck' ? 0 : 1;
  $long= strlen($sna);
  $lastbl= $pos= 0;
  while($pos<$long){
    $len= ord($sna[$pos])|ord($sna[$pos+1])<<8;
    pilot( $lastbl ? 1000+700 : 200+700 );
    loadconf( $velo                           // velocidad
            | $mlow<<3                        // frecuencia muestreo
            | 0x1f<<4                         // 5 bits a 1
            | 1<<9                            // bit snapshot
            | $nocheck<<10                    // eludo checksum
            | ord($sna[$pos+2])<<11           // byte flag
            | ord($sna[$pos+$len+1])<<19);    // checksum
    for($i= 2; $i<$len; $i++)
      $bytes.= $st[ord($sna[$pos+1+$i])];
    outbits($termin[$mlow][$velo]>>1);
    outbits($termin[$mlow][$velo]-($termin[$mlow][$velo]>>1));
    outbits_double(1 << $mhigh);
    $lastbl= ord($sna[$pos+2]);
    $pos+= $len+2;
  }
}
else{
  $parche= isset($_SERVER['argv'][5]) ? hexdec($_SERVER['argv'][5]) : 0x5780;
  if( strlen($sna)==49179 ){
    $pos= 25;
    $long= 49152+27;
    $sp= ord($sna[23]) | ord($sna[24])<<8;
    $regs=  substr($sna, 0xbffe-0x3fe5, 4).         // stack padding
            substr($sna, 5, 2).                     // BC'
            substr($sna, 3, 2).                     // DE'
            substr($sna, 1, 2).                     // HL'
            substr($sna, 7, 2).                     // AF'
            substr($sna, 13, 2).                    // BC
            substr($sna, 11, 2).                    // DE
            $sna[0].chr($r).                        // IR
            substr($sna, 17, 2).                    // IX
            substr($sna, 15, 2).                    // IY
            chr(ord($sna[25])>>1                    // IM
              | ord($sna[19])<<7                    // IFF1
              | ord($sna[26])<<1).                  // Border
            substr($sna, 21, 2).                    // AF
            chr(0x21) . substr($sna, 9, 2).         // HL
            chr(0x31) . pack('v', $sp+2).           // SP
            chr(0xc3) . substr($sna, $sp-0x3fe5, 2);// PC
    $sna=   substr($sna, 0, 0xbffe-0x3fe5).
            pack('vv', 0x3b88, $parche).
            substr($sna, 0xc002-0x3fe5);
    $sna=   substr($sna, 0, 25).
            pack('v', 0x3c42).                      // posiciones 3ffe-3fff de la ROM
            substr($sna, 27, $parche-0x4000).
            $regs.
            substr($sna, $parche+strlen($regs)-0x3fe5);
    pilot( 200 );
    loadconf( $velo                           // velocidad
            | $mlow<<3                        // frecuencia muestreo
            | 0x1f<<4                         // 5 bits a 1
            | 0<<9                            // bit snapshot activado
            | 1<<10                           // bit checksum desactivado
            | 0<<11                           // byte flag
            | 0x3f<<19);                      // start high byte
    while($pos<$long)
      $bytes.= $st[ord($sna[$pos++])];
    outbits($termin[$mlow][$velo]>>1);
    outbits($termin[$mlow][$velo]-($termin[$mlow][$velo]>>1));
    outbits_double(1 << $mhigh);
  }
  else{
    $page[5]= substr($sna, 27, 0x4000);
    $page[2]= substr($sna, 0x401b, 0x4000);
    $last= ord($sna[0xc01d])&7;
    $page[$last]= substr($sna, 0x801b, 0x4000);
    for($i= 0; $i<8; $i++)
      if(($last!=$i)&&($i!=2)&&($i!=5))
        $page[$i]= substr($sna, 0xc01f+$next++*0x4000, 0x4000);
    $regs=  substr($page[2], 0x3ffe).                 // stack padding
            substr($page[7], 0, 2).                   // stack padding
            chr(ord($sna[0xc01d])|0x10).              // last byte 7FFD
            substr($sna, 5, 2).                       // BC'
            substr($sna, 3, 2).                       // DE'
            substr($sna, 1, 2).                       // HL'
            substr($sna, 7, 2).                       // AF'
            substr($sna, 13, 2).                      // BC
            substr($sna, 11, 2).                      // DE
            $sna[0].chr($r).                          // IR
            substr($sna, 17, 2).                      // IX
            substr($sna, 15, 2).                      // IY
            chr(ord($sna[25])>>1                      // IM
              | ord($sna[19])<<7                      // IFF1
              | ord($sna[26])<<1).                    // Border
            substr($sna, 21, 2).                      // AF
            chr(0x21) . substr($sna, 9, 2).           // HL
            chr(0x31) . substr($sna, 23, 2).          // SP
            ( ord($sna[0xc01d])&0x10
                ? ''
                : chr(0x01) . chr(0xfd) . chr(0x7f).  // LD BC,7FFD
                  chr(0x3e) . $sna[0xc01d].           // LD A,last byte 7FFD
                  chr(0xed) . chr(0x79).              // OUT (C),A
                  chr(0x01) . substr($sna, 13, 2).    // restore BC
                  chr(0x3e) . $sna[0xc01d]).          // restore A
            chr(0xc3) . substr($sna, 0xc01b, 2);      // PC
    if($parche<0x8000)
      $page[5]= substr($page[5], 0, $parche-0x4000).
                $regs.
                substr($page[5], $parche+strlen($regs)-0x4000);
    else
      $page[2]= substr($page[2], 0, $parche-0x8000).
                $regs.
                substr($page[2], $parche+strlen($regs)-0x8000);
    $page[2]= substr($page[2], 0, 0x3ffe).
              pack('v', 0x3ae6);
    $page[7]= pack('v', $parche).
              substr($page[7], 2);
    pilot( 200 );
    loadconf( $velo                           // velocidad
            | $mlow<<3                        // frecuencia muestreo
            | 0x1f<<4                         // 5 bits a 1
            | 0<<9                            // bit snapshot activado
            | 1<<10                           // bit checksum desactivado
            | 0<<11                           // byte flag
            | 0xbf<<19);                      // start high byte
    $page[0]= pack('v', 0x3ae6).$page[0];
    for($j= 0; $j<0x4002; $j++)
      $bytes.= $st[ord($page[0][$j])];
    outbits($termin[$mlow][$velo]>>1);
    outbits($termin[$mlow][$velo]-($termin[$mlow][$velo]>>1));
    outbits_double(1 << $mhigh);
    for($i= 1; $i<8; $i++){
      for($j= 0; $j<0x4000; $j++)
        $bytes.= $st[ord($page[$i][$j])];
      outbits($termin[$mlow][$velo]>>1);
      outbits($termin[$mlow][$velo]-($termin[$mlow][$velo]>>1));
      outbits_double(1 << $mhigh);
    }
  }
}
pilot(8);
$longi= strlen($bytes);
echo 'Hecho.';
file_put_contents(substr($_SERVER['argv'][1],0,-4).'.wav',
                  'RIFF'.pack('L', $longi+36).'WAVEfmt '.pack('L', 16).pack('v', 1).pack('v', 2).
                  pack('L', $states[$mhigh][$mlow]).pack('L', $states[$mhigh][$mlow]<<1).
                  pack('v', 2).pack('v', 8).'data'.pack('L', $longi).$bytes);