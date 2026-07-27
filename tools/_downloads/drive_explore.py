import re
import urllib.request

UA = "Mozilla/5.0"

def fetch(url):
    req = urllib.request.Request(url, headers={"User-Agent": UA})
    with urllib.request.urlopen(req, timeout=30) as r:
        return r.read().decode("utf-8", errors="ignore")

def list_folder(folder_id):
    url = f"https://drive.google.com/embeddedfolderview?id={folder_id}"
    html = fetch(url)
    files = re.findall(
        r'<a[^>]*href="([^"]*file/d/[^"]*)"[^>]*>.*?flip-entry-title">([^<]+)</div>',
        html, re.DOTALL)
    subfolders = re.findall(
        r'<a[^>]*href="([^"]*folders/[^"]*)"[^>]*>.*?flip-entry-title">([^<]+)</div>',
        html, re.DOTALL)
    result_files = []
    for href, fn in files:
        fid = re.search(r"file/d/([a-zA-Z0-9_-]+)", href)
        result_files.append((fn, fid.group(1) if fid else None))
    result_folders = []
    for href, fn in subfolders:
        fid = re.search(r"folders/([a-zA-Z0-9_-]+)", href)
        result_folders.append((fn, fid.group(1) if fid else None))
    return result_files, result_folders

# カテゴリフォルダ
categories = {
    "Big": "1fOL6ES-e73dPPLzc7_vvmTJ4uZ_j8QG2",
    "Blob": "1lW9Bw_QaD8TvTFCGcuUevRFb--xTtb_1",
    "Flying": "1tBcwb_hEg1NuLjNp8k5neuomn7catr5K",
}

print("=" * 60)
print("Exploring Ultimate Monsters Pack Drive folder")
print("=" * 60)

for cat_name, cat_id in categories.items():
    print(f"\n### Category: {cat_name}")
    files, subfolders = list_folder(cat_id)
    print(f"  Files: {files}")
    print(f"  Subfolders: {[(n) for n,_ in subfolders]}")

    # glTF サブフォルダを探す
    for sf_name, sf_id in subfolders:
        if "gltf" in sf_name.lower() or "glb" in sf_name.lower():
            print(f"\n  >> {cat_name}/{sf_name} ({sf_id})")
            glb_files, _ = list_folder(sf_id)
            for fn, fid in glb_files:
                print(f"      GLB: {fn}  ->  file_id={fid}")
