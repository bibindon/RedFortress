# -*- coding: utf-8 -*-
"""Blenderなしで _build_stage_grounds.py の検証ロジックだけを実行するハーネス。
bpy インポートと main() 呼び出しを除去してから exec し、validate_stage を対象ステージに適用する。"""
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "res" / "model" / "ground" / "_build_stage_grounds.py"
sys.path.insert(0, str(SCRIPT.parent))

src = SCRIPT.read_text(encoding="utf-8-sig")
src = src.replace("import bpy\n", "")
src = src.replace("main()\n", "")

ns = {"__file__": str(SCRIPT), "__name__": "stage_ground_validation"}
exec(compile(src, str(SCRIPT), "exec"), ns)
validate_stage = ns["validate_stage"]
STAGES = ns["STAGES"]

targets = sys.argv[1:] if len(sys.argv) > 1 else ["1-3"]
ok = True
for stage in STAGES:
    if stage["display"] not in targets:
        continue
    conflicts, warnings = validate_stage(stage)
    print("=== %s ===" % stage["display"])
    for w in warnings:
        print("WARNING:", w)
    if conflicts:
        ok = False
        for c in conflicts:
            print("CONFLICT:", c)
    else:
        print("OK: no conflicts")
print("RESULT:", "PASS" if ok else "FAIL")
sys.exit(0 if ok else 1)
