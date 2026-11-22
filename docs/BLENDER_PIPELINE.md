# Blender → BOT BANDIT (Raylib)

Raylib loads **.obj**, **.iqm**, **.gltf/.glb**. Recommended path:

## 1. Model in Blender
- Character: low-poly 800–3000 tris, origin at feet
- Apply scale (Ctrl+A)
- UV unwrap, texture max 512–1024 PNG

## 2. Export
**glTF 2.0 (.glb)** preferred:
- File → Export → glTF 2.0 → GLB
- Apply modifiers, UVs, Normals

**OBJ** fallback: triangulate + materials + UVs

## 3. Place files
```
assets/models/char_male.glb
assets/models/char_female.glb
assets/models/car_lambo.glb
```

## 4. Load
```cpp
Model m = LoadModel("assets/models/char_male.glb");
DrawModelEx(m, pos, {0,1,0}, yaw, {1,1,1}, WHITE);
UnloadModel(m);
```

## 5. Shaders
`Shaders3D` — soft light + rim. Full 3D: `BeginMode3D` + `DrawModel`.

## 50/50 roster
m_street, m_cyber, m_suit, m_blond / f_model, f_rock, f_blond, f_street  
→ export as `char_<id>.glb`
