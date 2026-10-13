// DAWN data shard — two-part shell for a naked USB board.
// Cyberpunk shard / holotape. Angular, not rounded.
//
// AMS / multi-material:
//   bottom + top  = matte black or gunmetal
//   accents       = neon #39FF14 or acid yellow #FFF200
// Load "top" and "accents" together. They share an origin, and the
// letters sit in the recesses instead of intersecting the lid.
//
//   part = "print"    bottom, then the lid with its accents beside it
//   part = "bottom"
//   part = "top"
//   part = "accents"
//   part = "assembled"

/* [PCB] */
pcb_length = 30;
pcb_width = 14;
pcb_height = 4.5;

/* [Shell] */
wall = 1.8;
floor_thickness = 1.8;
lid_thickness = 2.6;
cavity_clearance = 0.35;
fit_clearance = 0.18;
chamfer = 0.8;
tail_length = 16;
retain_lip = 0.8;
lanyard_diameter = 4.2;
text_depth = 0.5;
accent_height = 0.4;
snap = 0.45;

/* [Render] */
part = "print";

$fn = 48;

width = pcb_width + 2 * wall + 4;
length = tail_length + pcb_length + wall + 5;
bottom_inner = floor_thickness + pcb_height + cavity_clearance;
pocket_depth = lid_thickness * 0.72;
bottom_height = bottom_inner + pocket_depth;
cavity_length = pcb_length + 2 * cavity_clearance;
cavity_width = pcb_width + 2 * cavity_clearance;
cavity_x = tail_length + 0.6;
cavity_y = (width - cavity_width) / 2;

module shard_2d() {
    polygon(points = [
        [1.2, width * 0.30],
        [0.0, width * 0.40],
        [0.0, width * 0.60],
        [1.2, width * 0.70],
        [tail_length, width],
        [length - 5, width],
        [length, width - 1.6],
        [length, 1.6],
        [length - 5, 0],
        [tail_length, 0]
    ]);
}

module chamfered(h) {
    hull() {
        translate([0, 0, chamfer])
            linear_extrude(h - 2 * chamfer)
                shard_2d();
        linear_extrude(0.02)
            offset(delta = -chamfer)
                shard_2d();
        translate([0, 0, h - 0.02])
            linear_extrude(0.02)
                offset(delta = -chamfer)
                    shard_2d();
    }
}

module lanyard_hole() {
    translate([5.6, width / 2, -2])
        cylinder(h = bottom_height + lid_thickness + 8, d = lanyard_diameter);
}

module greeble_cuts(z, depth) {
    for (i = [0 : 6]) {
        translate([tail_length + 3 + i * 4.2, -0.2, z])
            cube([1.4, 1.3, depth + 0.2]);
        translate([tail_length + 3 + i * 4.2, width - 1.1, z])
            cube([1.4, 1.3, depth + 0.2]);
    }
    translate([length - 3.2, width * 0.5 - 2.2, z])
        cube([1.1, 4.4, depth + 0.2]);
}

module word_2d() {
    translate([length * 0.58, width / 2])
        text("DAWN", size = 5.2, font = "DejaVu Sans:style=Bold",
             halign = "center", valign = "center", spacing = 1.05);
}

module chevron_2d() {
    for (side = [-1, 1]) {
        translate([length * 0.58, width / 2 + side * 5.3])
            rotate([0, 0, side * 18])
                square([16, 0.7], center = true);
    }
}

module dawn_letters() {
    linear_extrude(accent_height) word_2d();
}

module chevrons() {
    linear_extrude(accent_height) chevron_2d();
}

module bottom() {
    difference() {
        chamfered(bottom_height);
        translate([cavity_x, cavity_y, floor_thickness])
            cube([cavity_length, cavity_width, bottom_height]);
        translate([length - wall - 2,
                   (width - (pcb_width - 2 * retain_lip)) / 2,
                   floor_thickness])
            cube([wall + 6, pcb_width - 2 * retain_lip, pcb_height]);
        translate([0, 0, bottom_inner])
            linear_extrude(pocket_depth + 1)
                offset(delta = -(wall - 0.2))
                    shard_2d();
        lanyard_hole();
        greeble_cuts(bottom_height - 0.45, 0.45);
        // Detents in the pocket walls. The lid tongues catch here.
        translate([cavity_x + cavity_length * 0.5, cavity_y - 0.2, bottom_inner + pocket_depth * 0.45])
            cube([3.2, wall, snap + 0.2]);
        translate([cavity_x + cavity_length * 0.5, cavity_y + cavity_width - wall + 0.4, bottom_inner + pocket_depth * 0.45])
            cube([3.2, wall, snap + 0.2]);
    }
}

module top() {
    difference() {
        linear_extrude(lid_thickness)
            offset(delta = -(wall - 0.2) - fit_clearance)
                shard_2d();
        translate([0, 0, lid_thickness - text_depth]) {
            linear_extrude(text_depth + 0.04) word_2d();
            linear_extrude(text_depth + 0.04) chevron_2d();
        }
    }
    translate([cavity_x + cavity_length * 0.5 + 0.4, cavity_y + wall * 0.2, 0])
        cube([2.4, snap, snap]);
    translate([cavity_x + cavity_length * 0.5 + 0.4, cavity_y + cavity_width - wall * 0.2 - snap, 0])
        cube([2.4, snap, snap]);
}

module accents() {
    translate([0, 0, lid_thickness - text_depth + 0.04]) {
        dawn_letters();
        chevrons();
    }
}

module laid_out() {
    bottom();
    translate([0, width + 8, 0]) {
        top();
        accents();
    }
}

if (part == "bottom") bottom();
else if (part == "top") top();
else if (part == "accents") accents();
else if (part == "assembled") {
    bottom();
    translate([0, 0, bottom_inner]) {
        top();
        accents();
    }
} else {
    laid_out();
}
