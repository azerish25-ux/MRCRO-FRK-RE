"""Check resource constructs that otherwise hide installed UI controls."""
from pathlib import Path
import tempfile
import unittest

from inventory_installation import parse_resource_xml


class ResourceXmlTests(unittest.TestCase):
    def parse(self, content):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "resource.xml"
            path.write_text(content, encoding="utf-8")
            return parse_resource_xml(path)

    def test_standard_document_has_no_normalization(self):
        document, normalization = self.parse('<root><param name="Cutoff"/></root>')
        self.assertEqual(document.find("param").get("name"), "Cutoff")
        self.assertIsNone(normalization)

    def test_template_prefix_and_multiple_roots_preserve_control_bindings(self):
        document, normalization = self.parse(
            '<?xml version="1.0"?><template:knob param="Cutoff"/>,<style name="dial"/>'
        )
        self.assertEqual(document[0].tag, "template:knob")
        self.assertEqual(document[0].get("param"), "Cutoff")
        self.assertEqual(document[1].tag, "style")
        self.assertEqual(normalization["prefixes"], ["template"])

    def test_broken_resource_is_not_silently_accepted(self):
        import xml.etree.ElementTree as ET
        with self.assertRaises(ET.ParseError):
            self.parse('<root><param name="Cutoff"></root>')


if __name__ == "__main__":
    unittest.main()
