import bpy
from mathutils import Vector

def delete_all():
    """
    From https://github.com/kabuku/blender-python
    """
    for item in bpy.context.scene.objects:
        bpy.context.scene.objects.unlink(item)
    for item in bpy.data.objects:
        bpy.data.objects.remove(item)
    for item in bpy.data.meshes:
        bpy.data.meshes.remove(item)
    for item in bpy.data.materials:
        bpy.data.materials.remove(item)

def draw_wall(vertices, faces, height, name):
    # For some reason we need both a "mesh" and an "object"
    mesh = bpy.data.meshes.new("%sMesh" % name)
    obj = bpy.data.objects.new("%sObject" % name, mesh)
    bpy.context.scene.objects.link(obj)
    bpy.context.scene.objects.active = obj
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    # This is necessary to set the "context" for the operator
    bpy.ops.object.mode_set(mode='OBJECT')
    obj.select = True
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.extrude_region_move(MESH_OT_extrude_region=None,
                                     TRANSFORM_OT_translate={"value": height})
    mesh.update()
    # Deselect it so the next call will not have any leftover context
    bpy.ops.object.mode_set(mode='OBJECT')
    obj.select = False

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
        (6.1,  0, 0),
        (6.1,  0, 1.2),
        (-0.1, 0, 1.2),
    ]
    draw_wall(bottom_vertices, faces, Vector((0, -0.1, 0)), "Bottom")
    top_vertices = [
        (-0.1, 8.4, 0),
        (6.1,  8.4, 0),
        (6.1,  8.4, 1.2),
        (-0.1, 8.4, 1.2),
    ]
    draw_wall(top_vertices, faces, Vector((0, 0.1, 0)), "Top")
    left_vertices = [
        (0, 0,   0),
        (0, 8.4, 0),
        (0, 8.4, 1.2),
        (0, 0,   1.2),
    ]
    draw_wall(left_vertices, faces, Vector((-0.1, 0, 0)), "Left")
    right_vertices = [
        (6, 0,   0),
        (6, 8.4, 0),
        (6, 8.4, 1.2),
        (6, 6.6, 1.2),
        (6, 6.6, 0.5),
        (6, 4.6, 0.5),
        (6, 4.6, 1.2),
        (6, 0,   1.2),
    ]
    # In order to make the space for the programmer/power, we made a bunch
    # more vertices, so we need to specify a special face for this side
    right_faces = [
        (0, 1, 2, 3, 4, 5, 6, 7),
    ]
    draw_wall(right_vertices, right_faces, Vector((0.1, 0, 0)), "Right")

def make_boolean_union():
    """
    Based on https://github.com/kabuku/blender-python
    """
    base_obj = bpy.data.objects[0]
    bpy.context.scene.objects.active = base_obj
    for i, obj in enumerate(bpy.data.objects):
        if i == 0:
            continue
        boolean = base_obj.modifiers.new('BooleanUnion', 'BOOLEAN')
        boolean.object = obj
        boolean.operation = 'UNION'
        bpy.ops.object.modifier_apply(apply_as='DATA', modifier='BooleanUnion')
        bpy.context.scene.objects.unlink(obj)

if __name__ == "__main__":
    delete_all()
    draw_case_bottom()
    #make_boolean_union()
