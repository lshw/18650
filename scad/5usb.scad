// 第一版线路板的电池座
$fn=40;
x=100;
y=75;
ra=19;
z=ra/2;
xa=92;
xb=xa/5;
x1=(x-xa)/2+xa/5/2;
x2=x1+xa/5;
x3=x2+xa/5;
x4=x3+xa/5;
x5=x4+xa/5;

difference() {
cube([x,y,ra-4]);
translate([x1-z,2,z]) cube([xa+0.5,y-6,z]);
translate([x1,2,z-0.5])    rotate(90, [-1, 0, 0]) cylinder(h=69,r=z);
translate([x2,2,z-0.5])    rotate(90, [-1, 0, 0]) cylinder(h=69,r=z);
translate([x3,2,z-0.5])    rotate(90, [-1, 0, 0]) cylinder(h=69,r=z);
translate([x4,2,z-0.5])    rotate(90, [-1, 0, 0]) cylinder(h=69,r=z);
translate([x5,2,z-0.5])    rotate(90, [-1, 0, 0]) cylinder(h=69,r=z);

translate([x1,y-2-1.5,0])    cube([10,3,z*2],true);
translate([x2,y-2-1.5,0])    cube([10,3,z*2],true);
translate([x3,y-2-1.5,0])    cube([10,3,z*2],true);
translate([x4,y-2-1.5,0])    cube([10,3,z*2],true);
translate([x5,y-2-1.5,0])    cube([10,3,z*2],true);

translate([(x-95)/2,5,0]) {
cylinder(h=z-4,r=1.5);
translate([95,0,0]) cylinder(h=z-4,r=1.5);
}
translate([0,71,0]) {
cube([6.5,y-71,4]);
translate([3,3,0]) cylinder(h=z*2,r=2);
}
}
