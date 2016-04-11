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

def draw_bottom():
    mesh = bpy.data.meshes.new('BottomPlane')
    obj = bpy.data.objects.new('BottomMesh', mesh)
    bpy.context.scene.objects.link(obj)
    bpy.context.scene.objects.active = obj
    # Let's call one unit in Blender 1cm
    vertices = [
        (0, 0, 0),
        (6, 0, 0),
        (6, 8.4, 0),
        (0, 8.4, 0),
    ]

    # There is one face that connects all four vertices. It's specified by a tuple containing the vertex indices
    faces = [
        (0, 1, 2, 3),
    ]

    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    # This is necessary to set the "context" for the operator
    bpy.ops.object.mode_set(mode='OBJECT')
    obj.select = True
    bpy.ops.object.mode_set(mode='EDIT')
    # Make it a thickness of 1mm
    height = Vector((0, 0, 0.1))
    bpy.ops.mesh.extrude_region_move(MESH_OT_extrude_region=None,
                                     TRANSFORM_OT_translate={"value": height})
    mesh.update()

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
    draw_bottom()
    #make_boolean_union()
