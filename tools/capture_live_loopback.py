"""Record speaker-output loopback for observations of the already-open synth.

This never opens a physical microphone, launches a synth or changes audio-device
settings. Loopback can include other system playback; this is not an isolated
sample-accurate instrument fixture.
"""
import argparse
import json
import os
from pathlib import Path
import time
import wave


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--seconds", type=float, default=20)
    args = parser.parse_args()
    output = args.output.resolve()
    if os.name != "nt" or output.drive.upper() != "D:":
        parser.error("local capture must remain on D:")
    if not 0 < args.seconds <= 30:
        parser.error("capture duration must be in (0, 30] seconds")
    output.mkdir(parents=True, exist_ok=False)
    import numpy as np
    import soundcard as sc

    speaker = sc.default_speaker()
    loopback = sc.get_microphone(id=speaker.id, include_loopback=True)
    if not loopback.isloopback:
        raise RuntimeError("refusing to record a non-loopback device")
    sample_rate = 48000
    with loopback.recorder(samplerate=sample_rate, channels=[0, 1], blocksize=2048) as recorder:
        start = time.time_ns()
        (output / "ready.json").write_text(json.dumps({"unix_time_ns": start}), encoding="utf-8")
        print("Speaker loopback ready", flush=True)
        audio = recorder.record(numframes=round(args.seconds * sample_rate))
    np.save(output / "loopback.npy", audio.astype(np.float32), allow_pickle=False)
    with wave.open(str(output / "listening-copy.wav"), "wb") as stream:
        stream.setnchannels(2)
        stream.setsampwidth(2)
        stream.setframerate(sample_rate)
        stream.writeframes((np.clip(audio, -1, 1) * 32767).astype("<i2").tobytes())
    report = {
        "source": "default speaker WASAPI loopback; already-open application",
        "device_name": speaker.name, "sample_rate": sample_rate,
        "requested_seconds": args.seconds, "shape_frames_channels": list(audio.shape),
        "start_unix_time_ns": start, "finish_unix_time_ns": time.time_ns(),
        "peak": float(np.max(np.abs(audio))),
        "rms": float(np.sqrt(np.mean(audio.astype(np.float64) ** 2))),
        "finite": bool(np.isfinite(audio).all()),
        "limitations": ["system-output capture can include other playback",
                        "GUI note timing is not sample synchronized",
                        "not a bit-exact parity fixture or latency measurement"],
    }
    (output / "capture.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2), flush=True)


if __name__ == "__main__":
    main()
