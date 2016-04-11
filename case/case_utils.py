import bpy

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
