batr=9.75;
bh=1;
$fn=32;
module bat() {
cylinder(h=bh,r=batr+bh);
difference(){
cylinder(h=20,r=batr+bh);
cylinder(h=20,r=batr);
}
}
//bat(); //6

translate([batr+batr+bh,0,0]) bat();
translate([batr*4+bh*2,0,0]) bat();
lx=(batr*2+bh)*cos(60);
ly=(batr*2+bh)*sin(60);
translate([lx,ly,0]) {
bat();
translate([batr*2+bh,0,0]) bat();
translate([(batr*2+bh)*2,0,0]) bat();    //7
translate([lx,ly,0]) {
bat();
translate([batr*2+bh,0,0]) bat();  //7
}
}
