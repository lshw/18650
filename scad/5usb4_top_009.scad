$fn=32;
pcbx=100;
pcby=100;
pcbz=1.5;
hi=3;
ha=hi+pcbz;
bh=2;
fx=0.2;
cube([1.2,60,ha]);

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
cube([pcbx+bh+bh,pcby+bh+bh,ha]);
translate([bh-fx,bh-fx,hi]) cube([pcbx+fx+fx,pcby+fx+fx,pcbz]);
translate([bh,bh,0]) {

translate([2.54,2.54,0]) {
translate([95,0,0]) cylinder(h=ha,r=2);
translate([0,95,0]) cylinder(h=ha,r=2);
}

translate([13,13,0]) cube([78,21.3,ha]); //B2
translate([13,78.5,0]) cube([78,21.3,ha]); //B1
translate([-bh,47,0]) cube([15+bh,10,ha]); //CONN1

translate([88.5,41,0]) cylinder(h=ha,r=3.3);//lcd
translate([14,41,0]) cylinder(h=ha,r=3.3);//lcd
translate([14,71.5,0]) cylinder(h=ha,r=3.3);//lcd

translate([10,24,0]) cylinder(h=ha,r=2);//led2
translate([10,90,0]) cylinder(h=ha,r=2);//led1
translate([43,71,0]) cube([43,4,ha]);//LCD1
translate([92.5,71.5,0]) cube([7.5,8,ha]);  // sw1
translate([92.5,83.5,0]) cube([7.5,8,ha]);  // sw2
}
translate([bh,bh,0.2]){
translate([55.5,66,0]) cube([11.5,6,ha]);//x1
translate([33,75.5,0]) cube([8,4,ha]);  // D3  //*
translate([21.5,55.5,0]) cube([8,4,ha]);  // D2  //!
translate([46.5,8,0]) cube([8,4,ha]);  // D4   //!
translate([-bh,63,0]) cube([6+bh,8,ha]); //CONN7
}

translate([bh,bh,1]){

translate([10,24,0]) cylinder(h=ha,r=3.5);//led2
translate([10,90,0]) cylinder(h=ha,r=3.5);//led1
translate([1.5,7,0]) cube([10,9,ha]); //r11,r13,u6
translate([1.5,20,0]) cube([10,9,ha]); //u3,r9,r8,led2
translate([1.5,34,0]) cube([10,9,ha]); //r12,r15,u4
translate([1.5,74,0]) cube([10,9,ha]); //r17,r22,u5
translate([1.5,85.5,0]) cube([10,9,ha]); //r1,r4,u2
translate([31.5,34,0]) cube([31.5,3,ha]); //r16,c5,D6
translate([17,54,0]) cube([3,10,ha]); //usbcom-usb
translate([40,45,0]) cube([32,20,ha]); //usbcom-com,u8,u1,c2,c6,reset,r10,r7,r21

translate([26.5,76.5,0]) cube([48,5,ha]); //r23,c7,d7,r6,c1,r26,r27

translate([34.5,66,0]) cube([26,10,ha]); //r28,Q4,Q1,Q2,R29,R25
translate([82.5,47,0]) cube([18+bh,17,ha]); //conn8 sdcard

translate([15.5,37,0]) cylinder(h=ha,r=3);//B4+
translate([88,37,0]) cylinder(h=ha,r=3);//B4-

translate([15.5,10.7,0]) cylinder(h=ha,r=3);//B3+
translate([88,10.7,0]) cylinder(h=ha,r=3);//B3-

translate([15.5,77.5,0]) cylinder(h=ha,r=2.5);//B5+
translate([88,77.5,0]) cylinder(h=ha,r=2.5);//B5-

translate([35.8,51.3,0]) cylinder(h=ha,r=3);//C10
translate([20,51.1,0]) cylinder(h=ha,r=3);//C11
translate([31,63,0]) cylinder(h=ha,r=4);//R5
translate([57,29.5,0]) cylinder(h=ha,r=3);//ldo
translate([35.8,51.3,0]) cylinder(h=ha,r=3);//C10
translate([58,65.3,0]) cylinder(h=ha,r=3);//C9
translate([66,65.3,0]) cylinder(h=ha,r=3);//C8
translate([79,58.5,0]) cylinder(h=ha,r=3);//C12
translate([23,63,0]) cube([9,9,ha]);//Rf
translate([30,57,0]) cylinder(h=ha,r=5);//LDO
translate([27,51.5,0]) cylinder(h=ha,r=5);//LDO 1117


translate([94,80,0]) cube([6,3,ha]);//R20
translate([92,67.5,0]) cylinder(h=ha,r=3);//Q3
translate([87,69,0]) cylinder(h=ha,r=3);//LED6
translate([81,71.5,0]) cylinder(h=ha,r=3);//C? 0.1uf
translate([78.5,65.3,0]) cylinder(h=ha,r=5);//R19,R30
translate([40,9,0]) cube([5,3,ha]); //C4
translate([54.5,10,0]) cylinder(h=ha,r=3);//c3
translate([60.5,10,0]) cylinder(h=ha,r=4);//R10
translate([71.5,10,0]) cylinder(h=ha,r=4);//R14

}

}
