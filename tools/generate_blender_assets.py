# Blender 3.x/4.x headless: blender --background --python tools/generate_blender_assets.py
import math
import os
import sys

try:
    import bpy
    from mathutils import Vector, Euler
except ImportError:
    print("Run inside Blender: blender --background --python tools/generate_blender_assets.py")
    sys.exit(0)

OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "assets", "models")
os.makedirs(OUT, exist_ok=True)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for block in bpy.data.meshes:
        if block.users == 0:
            bpy.data.meshes.remove(block)
    for block in bpy.data.materials:
        if block.users == 0:
            bpy.data.materials.remove(block)
    for block in bpy.data.armatures:
        if block.users == 0:
            bpy.data.armatures.remove(block)
    for block in bpy.data.actions:
        if block.users == 0:
            bpy.data.actions.remove(block)


def mat(name, color, metallic=0.0, roughness=0.5):
    m = bpy.data.materials.new(name=name)
    m.use_nodes = True
    nt = m.node_tree
    bsdf = nt.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = (*color, 1.0)
        if "Metallic" in bsdf.inputs:
            bsdf.inputs["Metallic"].default_value = metallic
        if "Roughness" in bsdf.inputs:
            bsdf.inputs["Roughness"].default_value = roughness
    return m


def add_box(name, loc, scale, material):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(scale=True)
    if obj.data.materials:
        obj.data.materials[0] = material
    else:
        obj.data.materials.append(material)
    return obj


def add_uvsphere(name, loc, radius, material):
    bpy.ops.mesh.primitive_uv_sphere_add(radius=radius, location=loc, segments=16, ring_count=8)
    obj = bpy.context.active_object
    obj.name = name
    if obj.data.materials:
        obj.data.materials[0] = material
    else:
        obj.data.materials.append(material)
    bpy.ops.object.shade_smooth()
    return obj


def add_cylinder(name, loc, radius, depth, material):
    bpy.ops.mesh.primitive_cylinder_add(radius=radius, depth=depth, location=loc, vertices=12)
    obj = bpy.context.active_object
    obj.name = name
    if obj.data.materials:
        obj.data.materials[0] = material
    else:
        obj.data.materials.append(material)
    return obj


def join_selected(name):
    bpy.ops.object.join()
    obj = bpy.context.active_object
    obj.name = name
    return obj


def export_glb(path):
    bpy.ops.export_scene.gltf(
        filepath=path,
        export_format="GLB",
        use_selection=False,
        export_animations=True,
        export_skins=True,
        export_apply=False,
    )
    print("Wrote", path)


def build_character(char_id, female=False, hair_col=(0.1, 0.08, 0.06), shirt_col=(0.15, 0.15, 0.25)):
    clear_scene()
    skin = mat("skin", (0.82, 0.66, 0.55), 0.0, 0.55)
    shirt = mat("shirt", shirt_col, 0.05, 0.6)
    pants = mat("pants", (0.1, 0.1, 0.14), 0.0, 0.7)
    shoes = mat("shoes", (0.05, 0.05, 0.06), 0.1, 0.5)
    hair_m = mat("hair", hair_col, 0.0, 0.45)

    # armature
    bpy.ops.object.armature_add(enter_editmode=True, location=(0, 0, 0))
    arm = bpy.context.active_object
    arm.name = "Armature"
    eb = arm.data.edit_bones
    root = eb[0]
    root.name = "Root"
    root.head = (0, 0, 0)
    root.tail = (0, 0, 0.2)

    def bone(name, parent, head, tail):
        b = eb.new(name)
        b.head = head
        b.tail = tail
        b.parent = parent
        return b

    spine = bone("Spine", root, (0, 0, 0.9), (0, 0, 1.2))
    head_b = bone("Head", spine, (0, 0, 1.35), (0, 0, 1.55))
    thigh_l = bone("Thigh_L", root, (-0.12, 0, 0.9), (-0.12, 0, 0.45))
    shin_l = bone("Shin_L", thigh_l, (-0.12, 0, 0.45), (-0.12, 0, 0.1))
    thigh_r = bone("Thigh_R", root, (0.12, 0, 0.9), (0.12, 0, 0.45))
    shin_r = bone("Shin_R", thigh_r, (0.12, 0, 0.45), (0.12, 0, 0.1))
    arm_l = bone("Arm_L", spine, (-0.25, 0, 1.25), (-0.25, 0, 0.9))
    arm_r = bone("Arm_R", spine, (0.25, 0, 1.25), (0.25, 0, 0.9))

    bpy.ops.object.mode_set(mode="OBJECT")

    parts = []
    parts.append(add_box("Torso", (0, 0, 1.05), (0.22, 0.12, 0.28), shirt))
    parts.append(add_uvsphere("HeadMesh", (0, 0, 1.5), 0.14, skin))
    parts.append(add_uvsphere("Hair", (0, -0.02, 1.56), 0.15, hair_m))
    parts.append(add_box("LegL", (-0.12, 0, 0.5), (0.07, 0.07, 0.35), pants))
    parts.append(add_box("LegR", (0.12, 0, 0.5), (0.07, 0.07, 0.35), pants))
    parts.append(add_box("ShoeL", (-0.12, 0.04, 0.08), (0.08, 0.12, 0.05), shoes))
    parts.append(add_box("ShoeR", (0.12, 0.04, 0.08), (0.08, 0.12, 0.05), shoes))
    parts.append(add_box("ArmL", (-0.28, 0, 1.05), (0.06, 0.06, 0.28), skin))
    parts.append(add_box("ArmR", (0.28, 0, 1.05), (0.06, 0.06, 0.28), skin))
    if female:
        parts.append(add_box("HairLong", (0, -0.08, 1.35), (0.12, 0.05, 0.2), hair_m))

    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    body = join_selected("Body")

    # parent with automatic weights
    body.select_set(True)
    arm.select_set(True)
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.parent_set(type="ARMATURE_AUTO")

    # animations
    anims = {
        "Idle": [(1, 0.0), (20, 0.03), (40, 0.0)],
        "Walk": [(1, 0.0), (10, 0.35), (20, 0.0), (30, -0.35), (40, 0.0)],
        "Run": [(1, 0.0), (8, 0.55), (16, 0.0), (24, -0.55), (32, 0.0)],
        "Drive": [(1, 0.15)],
        "Fight": [(1, 0.0), (8, 0.8), (16, 0.0)],
        "Shoot": [(1, 0.5)],
        "Dance": [(1, 0.0), (12, 0.4), (24, -0.4), (36, 0.0)],
        "UsePhone": [(1, 0.25)],
        "Arrested": [(1, 0.0), (15, 0.5), (30, 0.5)],
    }

    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="POSE")
    for aname, keys in anims.items():
        action = bpy.data.actions.new(name=aname)
        arm.animation_data_create()
        arm.animation_data.action = action
        pb = arm.pose.bones.get("Thigh_L")
        if not pb:
            continue
        for frame, val in keys:
            pb.rotation_mode = "XYZ"
            pb.rotation_euler = (val, 0.0, 0.0)
            pb.keyframe_insert(data_path="rotation_euler", frame=frame)
            pb2 = arm.pose.bones.get("Thigh_R")
            if pb2:
                pb2.rotation_mode = "XYZ"
                pb2.rotation_euler = (-val, 0.0, 0.0)
                pb2.keyframe_insert(data_path="rotation_euler", frame=frame)
            al = arm.pose.bones.get("Arm_L")
            if al:
                al.rotation_mode = "XYZ"
                al.rotation_euler = (-val * 0.7, 0.0, 0.2)
                al.keyframe_insert(data_path="rotation_euler", frame=frame)
    bpy.ops.object.mode_set(mode="OBJECT")

    path = os.path.join(OUT, f"char_{char_id}.glb")
    export_glb(path)


def build_car(car_id, body_col=(0.8, 0.1, 0.15), kind="sedan"):
    clear_scene()
    body_m = mat("body", body_col, 0.6, 0.25)
    glass_m = mat("glass", (0.1, 0.15, 0.2), 0.0, 0.05)
    wheel_m = mat("wheel", (0.05, 0.05, 0.05), 0.2, 0.4)
    rim_m = mat("rim", (0.5, 0.5, 0.55), 0.8, 0.2)

    scales = {
        "sedan": (1.9, 0.9, 0.45),
        "coupe": (1.7, 0.85, 0.4),
        "suv": (1.95, 1.0, 0.65),
        "hyper": (1.8, 0.95, 0.35),
        "muscle": (2.0, 0.95, 0.42),
    }
    s = scales.get(kind, scales["sedan"])
    add_box("Chassis", (0, 0, 0.35), (s[0] * 0.5, s[1] * 0.5, s[2] * 0.5), body_m)
    add_box("Cabin", (-0.1, 0, 0.7), (0.45, 0.4, 0.22), body_m)
    add_box("Windshield", (0.15, 0, 0.72), (0.25, 0.38, 0.18), glass_m)
    for pos in [(-0.55, 0.42, 0.22), (0.55, 0.42, 0.22), (-0.55, -0.42, 0.22), (0.55, -0.42, 0.22)]:
        w = add_cylinder("Wheel", pos, 0.22, 0.14, wheel_m)
        w.rotation_euler = (math.pi / 2, 0, 0)
        r = add_cylinder("Rim", pos, 0.12, 0.08, rim_m)
        r.rotation_euler = (math.pi / 2, 0, 0)
    add_box("HeadL", (0.9, 0.28, 0.38), (0.04, 0.08, 0.06), mat("hl", (1, 0.95, 0.7), 0, 0.2))
    add_box("HeadR", (0.9, -0.28, 0.38), (0.04, 0.08, 0.06), mat("hr", (1, 0.95, 0.7), 0, 0.2))
    add_box("Steer", (0.25, 0.2, 0.65), (0.02, 0.12, 0.12), mat("st", (0.1, 0.1, 0.1), 0.3, 0.4))

    path = os.path.join(OUT, f"car_{car_id}.glb")
    export_glb(path)


def build_building(bld_id, floors=4):
    clear_scene()
    wall = mat("wall", (0.25, 0.25, 0.3), 0.0, 0.8)
    win = mat("win", (0.9, 0.85, 0.5), 0.0, 0.2)
    h = floors * 0.6
    add_box("Structure", (0, 0, h * 0.5), (1.2, 1.0, h * 0.5), wall)
    for f in range(floors):
        z = 0.35 + f * 0.6
        for x in (-0.5, 0.0, 0.5):
            for y in (-0.55, 0.55):
                add_box(f"W_{f}_{x}_{y}", (x, y, z), (0.15, 0.05, 0.15), win)
    add_box("Roof", (0, 0, h + 0.05), (1.3, 1.1, 0.08), mat("roof", (0.15, 0.15, 0.18), 0.1, 0.6))
    path = os.path.join(OUT, f"building_{bld_id}.glb")
    export_glb(path)


def main():
    print("Generating assets to", OUT)
    chars = [
        ("male_street", False, (0.05, 0.05, 0.06), (0.1, 0.12, 0.35)),
        ("male_cyber", False, (0.4, 0.1, 0.6), (0.05, 0.05, 0.08)),
        ("male_suit", False, (0.08, 0.06, 0.04), (0.08, 0.08, 0.1)),
        ("male_blond", False, (0.75, 0.65, 0.35), (0.2, 0.2, 0.25)),
        ("male_sport", False, (0.1, 0.1, 0.1), (0.6, 0.1, 0.1)),
        ("female_model", True, (0.08, 0.06, 0.04), (0.15, 0.12, 0.2)),
        ("female_rock", True, (0.5, 0.08, 0.08), (0.1, 0.1, 0.12)),
        ("female_blond", True, (0.85, 0.75, 0.4), (0.3, 0.3, 0.4)),
        ("female_street", True, (0.12, 0.1, 0.08), (0.15, 0.2, 0.35)),
        ("female_vip", True, (0.05, 0.05, 0.06), (0.05, 0.05, 0.08)),
    ]
    for cid, fem, hair, shirt in chars:
        try:
            build_character(cid, fem, hair, shirt)
        except Exception as e:
            print("char fail", cid, e)

    cars = [
        ("sedan", "sedan", (0.1, 0.1, 0.12)),
        ("coupe", "coupe", (0.7, 0.05, 0.1)),
        ("suv", "suv", (0.15, 0.2, 0.15)),
        ("hyper", "hyper", (0.9, 0.85, 0.1)),
        ("muscle", "muscle", (0.05, 0.15, 0.5)),
        ("mercedes", "sedan", (0.7, 0.7, 0.72)),
        ("bmw", "coupe", (0.05, 0.15, 0.45)),
        ("lambo", "hyper", (0.9, 0.75, 0.05)),
        ("lada", "sedan", (0.55, 0.55, 0.5)),
        ("tesla", "sedan", (0.85, 0.85, 0.88)),
    ]
    for cid, kind, col in cars:
        try:
            build_car(cid, col, kind)
        except Exception as e:
            print("car fail", cid, e)

    for i in range(20):
        try:
            build_building(f"{i:02d}", floors=2 + (i % 5))
        except Exception as e:
            print("bld fail", i, e)

    print("Done. Files in", OUT)


if __name__ == "__main__":
    main()
