$fn=20;
pcbx=100;
pcby=100;
bh=2;

translate([bh,bh,0])
translate([2.54,2.54,0]) {
cylinder(h=3.3,r=2); 
 cylinder(h=7,r=1.5);

translate([94.5,94.5,0]){ 
cylinder(h=3.3,r=2); 
 cylinder(h=7,r=1.5);
}
}

difference(){
cube([pcbx+bh+bh,pcby+bh+bh,3]);
translate([bh,bh,0]) {

translate([2.54,2.54,0]) {
translate([95,0,0]) cylinder(h=7,r=2);
translate([0,95,0]) cylinder(h=7,r=2);
}

translate([9,26.5,0]) cube([78,21.3,4]); //B3
translate([9,67.5,0]) cube([78,21.3,4]); //B4
translate([9,0,0]) cube([78,21.3,4]);  //b5
translate([9,50,0]) cube([18-4,16,4]);   //label !001
translate([46,50+0.5,0]) cube([7,7,4*2],true);  // reset switch  y=50 !001
translate([55,51.5,0]) cube([32-2,15,4]);  //usbcom !100

translate([90.2,10.5+0.5,0]) cylinder(h=4,r=2);//led3 !001
translate([90.2,37+1,0]) cylinder(h=4,r=2);//led4  !001
translate([90.2,77.5+0.5,0]) cylinder(h=4,r=2);//led5 !001

translate([66+1.5,94.5,0]) cylinder(h=4,r=6.3);//bat !001
}
translate([bh,bh,0.3]){
translate([41.5,88,0]) cube([12,12,4]);//U9
}
translate([bh,bh,1]){
translate([1,70,0]) cube([8,23,4]); //s2 s3

translate([90.2,10.5+0.5,0]) cylinder(h=4,r=4);//led3 !001
translate([90.2,37+1,0]) cylinder(h=4,r=4);//led4 !001
translate([90.2,77.5+0.5,0]) cylinder(h=4,r=4);//led5 !001

translate([11.9,100-76,0])  {
cylinder(h=4,r=2);//B2
translate([73,0,0]) cylinder(h=4,r=2);//B2
}

translate([11.9,100-10.6,0])  {
cylinder(h=4,r=2);//B1
translate([73,0,0]) cylinder(h=4,r=2);//B1
}

translate([90.4,100-76,0]) cylinder(h=4,r=3.5);//led2
translate([90.4,100-10.6,0]) cylinder(h=4,r=3.5);//led1

translate([30,100-9.5,0]) cylinder(h=4,r=4);//r24,r3

translate([58,100-5.8,0]) cylinder(h=4,r=3.5);//d1

translate([86,100-48,0]) cylinder(h=4,r=2.5);//power
translate([93,100-48,0]) cylinder(h=4,r=2.5);//power
translate([90.2,100-42,0]) cylinder(h=4,r=2.5);//power
}
}
