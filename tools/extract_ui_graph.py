"""Capture installed UI declarations for a later faithful renderer.

This preserves declarations and expressions; it does not evaluate the vendor UI
language or assert which conditional widgets are visible at runtime.
"""
import argparse
import collections
import json
import os
from pathlib import Path

from inventory_installation import parse_resource_xml, sha256


def extract(resources):
    documents = []
    failures = []
    tags = collections.Counter()
    for source in sorted(resources.rglob("*.xml")):
        relative = source.relative_to(resources).as_posix()
        try:
            root, normalization = parse_resource_xml(source)
            nodes = []

            def visit(element, parent=None):
                index = len(nodes)
                tags[element.tag] += 1
                nodes.append({"index": index, "parent": parent, "tag": element.tag,
                              "attributes": dict(element.attrib),
                              "text": (element.text or "").strip()})
                for child in element:
                    visit(child, index)

            visit(root)
            documents.append({"path": relative, "sha256": sha256(source),
                              "normalization": normalization, "nodes": nodes})
        except Exception as error:
            failures.append({"path": relative, "error": str(error)})
    return {
        "schema_version": 1, "resources": str(resources),
        "scope": "all resource XML declarations, including non-UI metadata",
        "semantics": "unevaluated declarations; runtime visibility and behavior unknown",
        "document_count": len(documents), "node_count": sum(len(d["nodes"]) for d in documents),
        "tag_counts": dict(tags.most_common()), "failures": failures,
        "documents": documents,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("resources", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    resources = args.resources.resolve(strict=True)
    output = args.output.resolve()
    if os.name == "nt" and output.drive.upper() != "D:":
        parser.error("local evidence must stay on D:")
    if output.is_relative_to(resources):
        parser.error("evidence must not be written into the installed resources")
    if output.exists():
        parser.error("choose a new evidence path; existing evidence is never overwritten")
    result = extract(resources)
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
    print(json.dumps({key: result[key] for key in
                      ("document_count", "node_count", "tag_counts", "failures")}, indent=2))


if __name__ == "__main__":
    main()
