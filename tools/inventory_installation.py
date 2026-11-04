"""Read-only, repeatable inventory of a user-selected MiniFreak V installation.

Detailed reports contain local resource metadata and are not publication inputs.
No installed code is loaded or executed. No third-party Python packages needed.
"""

import argparse
import collections
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import xml.etree.ElementTree as ET


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def parse_resource_xml(path):
    """Preserve vendor XML fragments and literal template: tag semantics.

    Vendor resources use multiple roots and undeclared template prefixes. A
    synthetic wrapper admits those constructs without editing installed files.
    The normalization is explicit evidence, not a claim of standard XML validity.
    """
    try:
        return ET.parse(path).getroot(), None
    except ET.ParseError as original:
        text = path.read_text(encoding="utf-8-sig")
        text = re.sub(r"^\s*<\?xml[^?]*\?>", "", text, count=1)
        prefixes = sorted(set(re.findall(r"</?([A-Za-z_][\w.-]*):[A-Za-z_]", text)))
        declarations = " ".join(f'xmlns:{prefix}="urn:resource-prefix:{prefix}"' for prefix in prefixes)
        wrapped = ET.fromstring(f"<resource-fragment {declarations}>{text}</resource-fragment>")
        for element in wrapped.iter():
            if element.tag.startswith("{urn:resource-prefix:"):
                prefix, name = element.tag[len("{urn:resource-prefix:"):].split("}", 1)
                element.tag = f"{prefix}:{name}"
        return wrapped, {
            "mode": "synthetic-fragment-wrapper", "prefixes": prefixes,
            "original_parse_error": str(original),
        }


def inventory(root, excluded):
    files = []
    issues = []
    xml_documents = []
    parameters = []
    enum_lists = []
    ui_types = collections.Counter()
    for directory, subdirs, filenames in os.walk(root, followlinks=False):
        subdirs[:] = sorted(
            name for name in subdirs
            if not (Path(directory) / name).is_symlink()
            and not (Path(directory) / name).resolve().is_relative_to(excluded)
        )
        for name in sorted(filenames):
            path = Path(directory) / name
            relative = path.relative_to(root).as_posix()
            if path.is_symlink():
                issues.append({"path": relative, "issue": "symlink not followed"})
                continue
            try:
                before = path.stat()
                record = {
                    "path": relative, "bytes": before.st_size,
                    "sha256": sha256(path), "extension": path.suffix.lower(),
                }
                if path.suffix.lower() == ".png":
                    with path.open("rb") as stream:
                        header = stream.read(24)
                    if header[:8] == b"\x89PNG\r\n\x1a\n" and len(header) == 24:
                        record["width"], record["height"] = struct.unpack(">II", header[16:24])
                if path.suffix.lower() == ".xml":
                    try:
                        document, normalization = parse_resource_xml(path)
                        elements = list(document.iter())
                        xml_documents.append({
                            "path": relative, "root": document.tag,
                            "elements": len(elements), "sha256": record["sha256"],
                            "normalization": normalization,
                        })
                        for index, element in enumerate(elements):
                            ui_types[element.tag] += 1
                            if element.tag == "param":
                                parameters.append({
                                    "source": relative, "element_index": index,
                                    "attributes": dict(element.attrib),
                                    "children": [
                                        {"tag": child.tag, "attributes": dict(child.attrib),
                                         "text": (child.text or "").strip()}
                                        for child in element
                                    ],
                                })
                            elif element.tag == "item_list":
                                enum_lists.append({
                                    "source": relative, "name": element.get("name"),
                                    "items": [dict(child.attrib) for child in element],
                                })
                    except (ET.ParseError, UnicodeError) as error:
                        issues.append({"path": relative, "issue": "xml_parse", "detail": str(error)})
                after = path.stat()
                if (before.st_size, before.st_mtime_ns) != (after.st_size, after.st_mtime_ns):
                    raise RuntimeError("file changed while being inventoried")
                files.append(record)
            except (OSError, RuntimeError) as error:
                issues.append({"path": relative, "issue": "file_read", "detail": str(error)})
    return {
        "schema_version": 1,
        "root": str(root),
        "excluded": str(excluded),
        "coverage": "sequential per-file observations; not an atomic directory snapshot",
        "file_count": len(files),
        "total_bytes": sum(item["bytes"] for item in files),
        "extensions": dict(sorted(collections.Counter(item["extension"] for item in files).items())),
        "xml_documents": xml_documents,
        "parameter_declarations": parameters,
        "enum_lists": enum_lists,
        "xml_element_counts": dict(ui_types.most_common()),
        "files": files,
        "issues": issues,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    parser.add_argument("--exclude", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    root = args.root.resolve(strict=True)
    excluded = args.exclude.resolve(strict=True)
    output = args.output.resolve()
    if not root.is_dir() or not excluded.is_relative_to(root) or excluded == root:
        parser.error("exclude must be a descendant directory of the installation")
    if not output.is_relative_to(excluded):
        parser.error("output must be inside the excluded project directory")
    if os.name == "nt" and output.drive.upper() != "D:":
        parser.error("project output must remain on D:")
    if output.exists():
        parser.error("refusing to overwrite existing evidence; choose a new output path")
    result = inventory(root, excluded)
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2, ensure_ascii=False)
        stream.write("\n")
    print(json.dumps({
        "file_count": result["file_count"], "total_bytes": result["total_bytes"],
        "xml_documents": len(result["xml_documents"]),
        "parameter_declarations": len(result["parameter_declarations"]),
        "enum_lists": len(result["enum_lists"]), "issues": result["issues"],
        "output": str(output),
    }, indent=2))


if __name__ == "__main__":
    main()
