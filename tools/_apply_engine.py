import sys


def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]


def parse_ftm(path):
    with open(path, "r", encoding="utf-8") as f:
        text = f.read()
    marker = "FrameTransformMatrix {"
    idx = text.find(marker)
    brace_open = text.find("{", idx)
    block_end = text.find(";;", brace_open)
    raw = text[brace_open + 1:block_end]
    nums = []
    for tok in raw.replace("\n", " ").replace("\t", " ").split(","):
        tok = tok.strip().rstrip(";").strip()
        try:
            nums.append(float(tok))
        except ValueError:
            pass
    return nums[:16]


def parse_vert_count_and_verts(path):
    verts = []
    with open(path, "r", encoding="utf-8") as f:
        lines = f.readlines()
    in_mesh = False
    in_verts = False
    expected = 0
    count = 0
    for ln in lines:
        s = ln.strip()
        if "Mesh " in s and "{" in s:
            in_mesh = True
            in_verts = False
            continue
        if in_mesh and not in_verts:
            token = s.rstrip(";").strip()
            if token.isdigit():
                expected = int(token)
                in_verts = True
                continue
        elif in_mesh and in_verts:
            core = s.rstrip(",;").replace(";;", "")
            parts = core.split(";")
            try:
                vals = [float(p) for p in parts if p.strip() != ""]
            except ValueError:
                if count >= expected:
                    break
                continue
            for v in chunks(vals, 3):
                verts.append(v)
                count += 1
                if count >= expected:
                    break
            if count >= expected:
                break
    return expected, verts


def matmul_row_major(a, b):
    r = [0.0] * 16
    for row in range(4):
        for col in range(4):
            s = 0.0
            for k in range(4):
                s += a[row * 4 + k] * b[k * 4 + col]
            r[row * 4 + col] = s
    return r


def apply_row_vector(m, v):
    rx = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3]
    ry = m[4] * v[0] + m[5] * v[1] + m[6] * v[2] + m[7]
    rz = m[8] * v[0] + m[9] * v[1] + m[10] * v[2] + m[11]
    return (rx, ry, rz)


def correct_blender_axis(ftm):
    axis = [1.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 1.0]
    corrected = matmul_row_major(axis, ftm)
    corrected[14] = -corrected[14]
    return corrected


def main(path):
    ftm = parse_ftm(path)
    corrected = correct_blender_axis(ftm)
    expected, verts = parse_vert_count_and_verts(path)
    print("Expected verts:", expected, "parsed:", len(verts))
    transformed = [apply_row_vector(corrected, v) for v in verts]
    xs = [t[0] for t in transformed]
    ys = [t[1] for t in transformed]
    zs = [t[2] for t in transformed]
    print("Y (height) range:", round(min(ys), 3), round(max(ys), 3))
    print("X range:", round(min(xs), 3), round(max(xs), 3))
    print("Z range:", round(min(zs), 3), round(max(zs), 3))


if __name__ == "__main__":
    main(sys.argv[1])
