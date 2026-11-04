"""Render a reproducible MIDI scenario through the installed, licensed VST3.

Outputs are local evidence. Audio is saved as float32 NumPy arrays; the WAV is
only a listening copy. Repeatability is measured, never assumed.
Requires the D:-local reference Python with pedalboard 0.9.26 and NumPy.
Historical offline-host workflow: subsequent sound checks in this project use
the user's already-open synth, as directed after the initial host experiment.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
import time
import wave


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("plugin", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--warmup-seconds", type=float, default=0)
    parser.add_argument("--restore-state", action="store_true")
    args = parser.parse_args()
    plugin_path = args.plugin.resolve(strict=True)
    output = args.output.resolve()
    if os.name != "nt" or output.drive.upper() != "D:":
        parser.error("reference capture must store its outputs on D:")
    output.mkdir(parents=True, exist_ok=False)
    for key, child in (("APPDATA", "appdata"), ("LOCALAPPDATA", "localappdata"),
                       ("TEMP", "tmp"), ("TMP", "tmp")):
        directory = output / child
        directory.mkdir(exist_ok=True)
        os.environ[key] = str(directory)

    import numpy as np
    import pedalboard
    from pedalboard import load_plugin

    print("Loading selected VST3", flush=True)
    plugin = load_plugin(str(plugin_path))
    if not plugin.is_instrument:
        raise RuntimeError("selected plugin does not identify itself as an instrument")
    if args.warmup_seconds > 0:
        print("Allowing plugin initialization and processing a silent warmup", flush=True)
        time.sleep(min(args.warmup_seconds, 30))
        plugin([], duration=args.warmup_seconds, sample_rate=48000,
               num_channels=2, buffer_size=128, reset=True)
    state = bytes(plugin.raw_state)
    (output / "initial-state.bin").write_bytes(state)
    parameters = {key: float(value.raw_value) for key, value in plugin.parameters.items()}
    events = [(bytes([0x90, 60, 100]), 0.0), (bytes([0x80, 60, 0]), 0.5)]
    arrays = []
    for index in range(2):
        print(f"Rendering reference {index + 1}", flush=True)
        if args.restore_state:
            plugin.raw_state = state
        audio = plugin(events, duration=1.5, sample_rate=48000,
                       num_channels=2, buffer_size=128, reset=True)
        np.save(output / f"render-{index + 1}.npy", audio, allow_pickle=False)
        arrays.append(audio)
    audio = arrays[0]
    with wave.open(str(output / "listening-copy.wav"), "wb") as stream:
        stream.setnchannels(audio.shape[0])
        stream.setsampwidth(2)
        stream.setframerate(48000)
        stream.writeframes((np.clip(audio.T, -1, 1) * 32767).astype("<i2").tobytes())
    difference = arrays[0].astype(np.float64) - arrays[1].astype(np.float64)
    finite = bool(np.isfinite(audio).all() and np.isfinite(arrays[1]).all())
    peak = float(np.max(np.abs(audio)))
    result = {
        "plugin_path": str(plugin_path),
        "plugin_sha256": hashlib.sha256(plugin_path.read_bytes()).hexdigest(),
        "plugin_name": plugin.name, "plugin_version": plugin.version,
        "pedalboard_version": pedalboard.__version__, "numpy_version": np.__version__,
        "python_version": sys.version, "state_sha256": hashlib.sha256(state).hexdigest(),
        "initial_state_description": "state returned immediately after this host instantiated the plugin",
        "sample_rate": 48000, "buffer_size": 128, "channels": 2, "duration_seconds": 1.5,
        "warmup_seconds": args.warmup_seconds, "restore_state_before_render": args.restore_state,
        "events": [{"bytes": list(message), "seconds": seconds} for message, seconds in events],
        "parameters_raw_normalized": parameters,
        "reported_latency_samples": plugin.reported_latency_samples,
        "samples_shape": list(audio.shape), "finite": finite, "peak": peak,
        "rms": float(np.sqrt(np.mean(audio.astype(np.float64) ** 2))),
        "repeated_render_bit_identical": bool(np.array_equal(arrays[0], arrays[1])),
        "repeat_max_abs_difference": float(np.max(np.abs(difference))),
        "repeat_rms_difference": float(np.sqrt(np.mean(difference ** 2))),
        "limitations": ["one initial state and MIDI scenario only",
                        "offline host render; not an Android latency measurement",
                        "host reset and state restoration do not establish RNG determinism"],
    }
    (output / "capture.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({key: result[key] for key in
                      ("plugin_name", "samples_shape", "finite", "peak", "rms",
                       "repeated_render_bit_identical", "repeat_rms_difference")}, indent=2), flush=True)
    if not finite or peak == 0:
        raise RuntimeError("reference render is silent or contains non-finite samples")


if __name__ == "__main__":
    main()
