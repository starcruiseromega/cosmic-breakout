import bpy
from mathutils import Vector

from case_utils import draw_wall, delete_all

def draw_lid():
    vector = Vector((0, 0, 0.1))
    # Let's call one unit in Blender 1cm
    bottom_vertices = [
        (-0.1, 0, 0),
        (5,    0, 0),
        (5, -0.1, 0),
        (6, -0.1, 0),
        (6,  1.1, 0),
        (0,  1.1, 0),
        (0,    1, 0),
        (-0.1, 1, 0),
    ]
    bottom_faces = [
        (0, 1, 2, 3, 4, 5, 6, 7),
    ] 
    draw_wall(bottom_vertices, bottom_faces, vector, "Bottom")
    button_left_vertices = [
        (0,   1.1, 0),
        (1.5, 1.1, 0),
        (1.5, 1.6, 0),
        (0,   1.6, 0),
    ]
    button_faces = [
        (0, 1, 2, 3),
    ]
    draw_wall(button_left_vertices, button_faces, vector, "ButtonLeft")
    button_middleL_vertices = [
        (2, 1.1, 0),
        (3, 1.1, 0),
        (3, 1.6, 0),
        (2, 1.6, 0),
    ]
    draw_wall(button_middleL_vertices, button_faces, vector, "ButtonMiddleLeft")
    button_middleR_vertices = [
        (3.5, 1.1, 0),
        (4.5, 1.1, 0),
        (4.5, 1.6, 0),
        (3.5, 1.6, 0),
    ]
    draw_wall(button_middleR_vertices, button_faces, vector,
              "ButtonMiddleRight")
    button_right_vertices = [
        (5, 1.1, 0),
        (6, 1.1, 0),
        (6, 1.6, 0),
        (5, 1.6, 0),
    ]
    draw_wall(button_right_vertices, button_faces, vector, "ButtonRight")
 

if __name__ == "__main__":
    delete_all()
    draw_lid()
