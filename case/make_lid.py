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
    midsection_vertices = [
        (0, 1.6, 0),
        (6, 1.6, 0),
        (6, 3.2, 0),
        (5, 3.2, 0),
        (5, 4.2, 0),
        (6, 4.2, 0),
        (6, 4.5, 0),
        (0, 4.5, 0),
    ]
    midsection_faces = [
        (0, 1, 2, 3, 4, 5, 6, 7),
    ]
    draw_wall(midsection_vertices, midsection_faces, vector, "Midsection")
    screen_left_vertices = [
        (0, 4.5, 0),
        (1, 4.5, 0),
        (1, 7.2, 0),
        (0, 7.2, 0),
    ]
    draw_wall(screen_left_vertices, button_faces, vector, "ScreenLeft")
    screen_right_vertices = [
        (4.8, 4.5, 0),
        (6,   4.5, 0),
        (6,   7.2, 0),
        (4.8, 7.2, 0),
    ]
    draw_wall(screen_right_vertices, button_faces, vector, "ScreenRight")
    top_vertices = [
        (0,   7.2, 0),
        (6,   7.2, 0),
        (6,   7.4, 0),
        (6.1, 7.4, 0),
        (6.1, 8.4, 0),
        (1,   8.4, 0),
        (1,   8.5, 0),
        (0,   8.5, 0),
    ]
    draw_wall(top_vertices, midsection_faces, vector, "Top")
 
if __name__ == "__main__":
    delete_all()
    draw_lid()
