// A simple "doghouse" picture, used for the synthetic target
// watchdog device host-side.
//
// This file is normally built using the following command line:
//
// x-povray +Idoghouse.pov +W128 +H128 +D +Q9 +AM2 +A0.9 +FP

#include "consts.inc"
#include "colors.inc"
#include "shapes.inc"
#include "textures.inc"
#include "skies.inc"
#include "woods.inc"

camera {
    location <4.5, 2.2, -5>
    look_at <2, 2, 2>
}

light_source {
    <7, 4, -1.5>
    color White
}

sky_sphere { S_Cloud2 scale 2 }
background { colour LightBlue }

plane {
    y 0
    texture {
        pigment { color red 0.5 green 0.98 blue 0 turbulence 0.5}
        normal  { bumps 0.4 scale 0.1 }
    }
}

fog {
  distance   40
  fog_type   Ground_Fog
  fog_offset 1
  fog_alt    1
  colour     rgbf<0, 0.2, 0.2, 0.3>
  turbulence 0.2
}

// A plank is a 1x1x0.1 block with a corner at the origin. The edges are
// slightly rounded, to make sure that the plank boundaries are just
// about visible.
#declare Plank =
  superellipsoid { <0.03,0.03> scale <0.5,0.5,0.05> translate <0.5,0.5,0.05> texture { T_Wood10 } }

// A variant, for the roof
#declare RoofPlank =
  superellipsoid { <0.05,0.05> scale <0.5,0.5,0.05> translate <0.5,0.5,0.05> texture { T_Wood14 } }

// The front and back, a rectangle with a triangle on top.
// Going clockwise from the bottom left, the coordinates are:
//   <0,0> <0,3.2>, <2.4.5>, <4,3.2>, <4.0>
// Each is made from five horizontal planks.
//
// Note: this doghouse is not intended to be an example of good
// woodworking. For example, butt joints for the walls are a bad idea.
// Most importantly the roof should involve lapped joints with a
// sensible ridge, the current construction is not going to keep the
// rain out.
#declare RoofAngle = degrees(atan2(1.3,2));
#declare RoofAngleR = atan2(1.3,2);

#declare FrontBack =
  difference {
      union {
          object { Plank scale <4,1,1> translate <0,0,0> }
          object { Plank scale <4,1,1> translate <0,1,0> }
          object { Plank scale <4,1,1> translate <0,2,0> }
          object { Plank scale <4,1,1> translate <0,3,0> }
          object { Plank scale <4,1,1> translate <0,4,0> }
      }
      union {
          box { <0,0,0> <4,4,1> rotate <0,0,RoofAngle> translate <0,3.2,-0.5> }
          box { <0,0,0> <4,4,1> rotate <0,0, -1 * RoofAngle> translate <2,4.5,-0.5> }
          pigment { Black }
      }
  }

// The front also has some text to name the dog, and a
// cutout for the opening.
object {
    difference {
        object { FrontBack }
        union {
            text {
                ttf "cyrvetic.ttf" "FIFI" 0.1 0
                translate <1.2, 2.8, -0.05>
            }
            box { <1,0,-0.5> <3,2.2,0.5> }
            object {
                cylinder { <0,0,0> <0,0,1> 1 }
                scale <1,0.33,1>
                translate <2,2.2,-0.5>
            }
            pigment { Black }
        }
    }
}

// The back, nothing fancy needed here. The doghouse is twice
// as deep as it is wide.
object { FrontBack translate <0,0,8> }

// A floor, to prevent any bright grass showing inside
box { <0.05,0,0.05> <3.95,0.05,7.95> texture { T_Wood3 } }

// LHS
object { Plank scale <8,1,1> rotate <0,-90,0> translate <0,0,0.1> }
object { Plank scale <8,1,1> rotate <0,-90,0> translate <0,1,0.1> }
object { Plank scale <8,1,1> rotate <0,-90,0> translate <0,2,0.1> }

// RHS
object { Plank scale <8,1,1> rotate <0,-90,0> translate <4,0,0.1> }
object { Plank scale <8,1,1> rotate <0,-90,0> translate <4,1,0.1> }
object { Plank scale <8,1,1> rotate <0,-90,0> translate <4,2,0.1> }

// Now for the roof. The top of the roof is at <2,4.5,0>, and the
// corners are at <0,3.2,0> and <4,3.2,0>. The planks are 0.1 units
// thick.

#declare RoofPlank = 
    superellipsoid { <0.05,0.05> scale <0.5,0.5,0.05> translate <0.5,0.5,0.05> texture { T_Wood14 } }

#declare RoofPlank_L = object {
    RoofPlank
    rotate <0, -90, RoofAngle - 90>
    scale<1,1,8.5>
}

#declare RoofPlank_R = object {
    RoofPlank
    rotate <0, -90, -90 - RoofAngle>
    scale<1,1,8.5>
}

object { RoofPlank_L translate<2 - (1 * cos(RoofAngleR)), 4.5 - (1 * sin(RoofAngleR)), -0.25> }
object { RoofPlank_L translate<2 - (2 * cos(RoofAngleR)), 4.5 - (2 * sin(RoofAngleR)), -0.25> }
object { RoofPlank_L translate<2 - (3 * cos(RoofAngleR)), 4.5 - (3 * sin(RoofAngleR)), -0.25> }

object { RoofPlank_R translate<2 + (-0.1 * cos(RoofAngleR)), 4.5 - (-0.1 * sin(RoofAngleR)), -0.25> }
object { RoofPlank_R translate<2 + (0.9 * cos(RoofAngleR)), 4.5 - (0.9 * sin(RoofAngleR)), -0.25> }
object { RoofPlank_R translate<2 + (1.9 * cos(RoofAngleR)), 4.5 - (1.9 * sin(RoofAngleR)), -0.25> }

// And just for fun, a dog bowl.
object {
    merge { 
        difference {
            torus { 1.0 0.5 }
            box { <-1,-1,-1> <1,0,1> pigment { Black } }
        }
	cylinder { <0,0,0> <0,0.2,0> 0.9 }
    }
    scale <0.5,0.5,0.5>
    translate <-0.5,0,-0.7>
    pigment { Yellow }
}
