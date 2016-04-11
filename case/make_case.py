import bpy
from mathutils import Vector

from case_utils import draw_wall, delete_all

def draw_case_bottom():
    # Let's call one unit in Blender 1cm
    floor_vertices = [
        (0, 0,   0),
        (6, 0,   0),
        (6, 8.4, 0),
        (0, 8.4, 0),
    ]
    # There is one face that connects all four vertices. It's specified by a
    # tuple containing the vertex indices
    faces = [
        (0, 1, 2, 3),
    ] 
    draw_wall(floor_vertices, faces, Vector((0, 0, 0.1)), "Floor")
    bottom_vertices = [
        (-0.1, 0, 0),
        (6,    0, 0),
        (6,    0, 1.1),
        (5,    0, 1.1),
        (5,    0, 1.2),
        (-0.1, 0, 1.2),
    ]
    # On the sides, we'll have more vertices to allow for a small indent for
    # the lid to sit in
    side_faces = [
        (0, 1, 2, 3, 4, 5),
    ]
    draw_wall(bottom_vertices, side_faces, Vector((0, -0.1, 0)), "Bottom")
    top_vertices = [
        (0,   8.4, 0),
        (6.1, 8.4, 0),
        (6.1, 8.4, 1.2),
        (1,   8.4, 1.2),
        (1,   8.4, 1.1),
        (0,   8.4, 1.1),
    ]
    draw_wall(top_vertices, side_faces, Vector((0, 0.1, 0)), "Top")
    left_vertices = [
        (0, 0,   0),
        (0, 8.5, 0),
        (0, 8.5, 1.2),
        (0, 1,   1.2),
        (0, 1,   1.1),
        (0, 0,   1.1),
    ]
    draw_wall(left_vertices, side_faces, Vector((-0.1, 0, 0)), "Left")
    right_vertices = [
        (6, -0.1, 0),
        (6, 8.4,  0),
        (6, 8.4,  1.1),
        (6, 7.4,  1.1),
        (6, 7.4,  1.2),
        (6, 6.6,  1.2),
        (6, 6.6,  0.5),
        (6, 4.6,  0.5),
        (6, 4.6,  1.2),
        (6, -0.1, 1.2),
    ]
    # In order to make the space for the programmer/power, we made a bunch
    # more vertices, so we need to specify a special face for this side
    right_faces = [
        (0, 1, 2, 3, 4, 5, 6, 7, 8, 9),
    ]
    draw_wall(right_vertices, right_faces, Vector((0.1, 0, 0)), "Right")

if __name__ == "__main__":
    delete_all()
    draw_case_bottom()
